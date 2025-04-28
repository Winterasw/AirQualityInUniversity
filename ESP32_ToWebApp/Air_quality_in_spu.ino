#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "time.h"
#include <HTTPClient.h>

/* WiFi credentials */
#define WIFI_SSID "Five Star Apartment Floor 4"
#define WIFI_PASSWORD "fst1658/29"

/* Firebase config */
#define API_KEY "AIzaSyCSZPuWoGaD6PRtQ1HDPGOAsSZuYLnqE5M"
#define DATABASE_URL "https://air-qulity-in-u-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define DATABASE_SECRET "l6hVz5gWoqfeSFNV6R1SbXX1EOmSPBigsBV4Va0F"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;  // GMT+7 (ไทย)
const int   daylightOffset_sec = 0;

void setup() {
    Serial.begin(115200);

    // เชื่อมต่อ WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println("\nConnected with IP: " + WiFi.localIP().toString());

    // ตั้งค่า Firebase
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    config.signer.tokens.legacy_token = DATABASE_SECRET;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    Serial.println("🔥 Firebase Connected!");
}

String getCurrentDate() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {  
        Serial.println("❌ Failed to obtain time");
        return "0000-00-00";
    }
    
    char dateStr[11];  
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
    Serial.printf("✅ วันที่: %s\n", dateStr);
    return String(dateStr);
}

// ฟังก์ชันดึงเวลาปัจจุบัน
String getCurrentTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {  
        Serial.println("❌ Failed to obtain time");
        return "00:00";
    }

    char timeStr[6];  
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    Serial.printf("✅ เวลา: %s\n", timeStr);
    return String(timeStr);
}

// ฟังก์ชันสร้าง JSON และส่งไป Firebase
void sendJsonToFirebase(float airpressure, float co, float co2, float hcho, float humidity, float pm10, float pm1_0, float pm2_5, float temp, float windspeed) {
    FirebaseJson json;
    String today = getCurrentDate();  
    String currentTime = getCurrentTime();  

    json.set("airpressure", airpressure);
    json.set("co", co);
    json.set("co2", co2);
    json.set("hcho", hcho);
    json.set("humidity", humidity);
    json.set("pm10", pm10);
    json.set("pm1_0", pm1_0);
    json.set("pm2_5", pm2_5);
    json.set("temp", temp);
    json.set("windspeed", windspeed);
    json.set("time", currentTime);

    delay(12000); //For store database
    String path = "/recordsWin/" + today + "/sensors/" + currentTime;
    if (Firebase.setJSON(fbdo, path, json)) {
        Serial.println("✅ JSON uploaded successfully!");
    } else {
        Serial.println("❌ Firebase Error: " + fbdo.errorReason());
    }
    
}



void sendFloatToFirebase(String path, float value) {
    if (Firebase.setFloat(fbdo, path, value)) {
        Serial.println("✅ Float uploaded: " + path + " = " + String(value));
    } else {
        Serial.println("❌ Firebase Error: " + fbdo.errorReason());
    }
}


void loop() {
 
    // random Sensors.
    float airpressure = random(90, 110) + random(0, 99) / 100.0;
    float co = random(10, 50) + random(0, 99) / 100.0;
    float co2 = random(50, 100) + random(0, 99) / 100.0;
    float hcho = random(20, 80) + random(0, 99) / 100.0;
    float humidity = random(10, 100);
    float pm10 = random(20, 150) + random(0, 99) / 100.0;
    float pm1_0 = random(10, 100) + random(0, 99) / 100.0;
    float pm2_5 = random(10, 100) + random(0, 99) / 100.0;
    float temp = random(20, 40);
    float windspeed = random(5, 30) + random(0, 99) / 100.0;


    sendFloatToFirebase("/realtimeWin/airpressure", airpressure);
    sendFloatToFirebase("/realtimeWin/co", co);
    sendFloatToFirebase("/realtimeWin/co2", co2);
    sendFloatToFirebase("/realtimeWin/hcho", hcho);

    
    // sendFloatToFirebase("/realtime/humidity", humidity);
    // sendFloatToFirebase("/realtime/pm10", pm10);
    // sendFloatToFirebase("/realtime/pm1_0", pm1_0);
    // sendFloatToFirebase("/realtime/pm2_5", pm2_5);
    // sendFloatToFirebase("/realtime/temp", temp);
    // sendFloatToFirebase("/realtime/windspeed", windspeed);
    Serial.println("--------------------------------------");
    sendJsonToFirebase(airpressure, co, co2, hcho, humidity, pm10, pm1_0, pm2_5, temp, windspeed);

    Serial.println("--------------------------------------");
    delay(10000);

}
