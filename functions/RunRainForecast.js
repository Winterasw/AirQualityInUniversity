const admin = require("firebase-admin");
const fetch = require("node-fetch");

// 👇 โหลด service account
const serviceAccount = require("./serviceAccountKey.json");

// 👇 Initialize Firebase
if (!admin.apps.length) {
  admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
  });
}

const db = admin.firestore();

const API_KEY =
  process.env.OPENWEATHER_API_KEY || "e5d06f804986aee2b7fbef62dd81435d";
const LAT = 13.85527;
const LON = 100.58532;

// 🔹 คืนวันที่ถัดไปจากวันนี้ N วัน เช่น getDateDaysAhead(1) = พรุ่งนี้
function getDateDaysAhead(days) {
  const date = new Date();
  date.setDate(date.getDate() + days);
  return date.toISOString().split("T")[0];
}

// 🔹 ดึง rainChance จาก OpenWeather
async function get5DayRainChance() {
  const url = `https://api.openweathermap.org/data/2.5/forecast?lat=${LAT}&lon=${LON}&appid=${API_KEY}&units=metric`;
  const response = await fetch(url);

  if (!response.ok)
    throw new Error(`🌩️ OpenWeather API error: ${response.status}`);
  const data = await response.json();

  const rainMap = {};

  for (const item of data.list) {
    const date = item.dt_txt.substring(0, 10);
    const pop = item.pop ?? 0;

    if (!rainMap[date]) rainMap[date] = [];
    rainMap[date].push(pop);
  }

  const result = {};
  for (const [date, pops] of Object.entries(rainMap)) {
    const avg = pops.reduce((a, b) => a + b, 0) / pops.length;
    result[date] = Math.round(avg * 100);
  }

  return result;
}

// 🔧 Main Function
(async () => {
  try {
    const forecast = await get5DayRainChance();
    const daysToSave = [1, 2, 3, 4, 5].map(getDateDaysAhead);

    for (const date of daysToSave) {
      const chance = forecast[date];
      if (chance === undefined) {
        console.log(`⚠️ No data for ${date}`);
        continue;
      }

      await db.collection("forecast_data").doc(date).set({
        rainChance: chance,
        date: date,
      });

      console.log(`✅ Written to forecast_data/${date}: ${chance}%`);
    }

    console.log("🎉 Manual rainChance write success!");
    process.exit(0);
  } catch (err) {
    console.error("❌ Manual run failed:", err);
    process.exit(1);
  }
})();
