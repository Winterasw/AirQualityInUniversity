const { onSchedule } = require("firebase-functions/v2/scheduler");
const admin = require("firebase-admin");

const serviceAccount = require("./serviceAccountKey.json");

if (!admin.apps.length) {
  admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL:
      "https://air-qulity-in-u-default-rtdb.asia-southeast1.firebasedatabase.app/",
  });
}

const rtdb = admin.database();
const firestore = admin.firestore();

/** ✅ ดึงข้อมูลจาก `/records/{date}/sensors` แล้วคำนวณค่าเฉลี่ย */
async function averageSensorData(date) {
  const snapshot = await rtdb.ref(`/records/${date}/sensors`).once("value");
  const data = snapshot.val();

  if (!data) {
    console.log(`⚠️ No sensor data found at /records/${date}/sensors`);
    return null;
  }

  const timeKeys = Object.keys(data);
  const sum = {};
  let count = 0;

  for (const time of timeKeys) {
    const item = data[time];
    let hasValid = false;

    for (const [k, v] of Object.entries(item)) {
      if (typeof v === "number") {
        sum[k] = (sum[k] || 0) + v;
        hasValid = true;
      }
    }

    if (hasValid) count++;
  }

  if (count === 0) return null;

  const avg = {};
  for (const [k, total] of Object.entries(sum)) {
    avg[k] = +(total / count).toFixed(2);
  }

  return avg;
}

/** ✅ เขียนข้อมูลเฉลี่ยลง Firestore ที่ collection: sensor_data */
async function updateTodayToRTDB(timeLabel) {
  try {
    const dateStr = new Date().toISOString().split("T")[0];
    console.log(`▶ [${timeLabel}] Averaging sensor data for ${dateStr}`);

    const avgData = await averageSensorData(dateStr);
    if (avgData) {
      await firestore
        .collection("sensor_data")
        .doc(dateStr)
        .set(
          {
            ...avgData,
            date: dateStr,
          },
          { merge: true }
        );

      console.log("✅ Averages → Firestore/sensor_data success");
    } else {
      console.log("⚠️ No average data to write");
    }
  } catch (err) {
    console.error("❌ Error in updateTodayToRTDB:", err);
  }
}

// 🔁 รันอัตโนมัติทุกวัน 06:00
exports.syncTodayAvg = onSchedule(
  "every day 06:00",
  { timeZone: "Asia/Bangkok" },
  () => updateTodayToRTDB("06:00")
);

// 🧪 สำหรับ manual
module.exports = { updateTodayToRTDB };
