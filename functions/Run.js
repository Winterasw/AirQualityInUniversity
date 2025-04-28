const { updateTodayToRTDB } = require("./openMeteoTodayRTDB");

updateTodayToRTDB("🔧 Manual run")
  .then(() => {
    console.log("✅ Manual update complete");
    process.exit(0);
  })
  .catch((err) => {
    console.error("❌ Manual run failed:", err);
    process.exit(1);
  });
