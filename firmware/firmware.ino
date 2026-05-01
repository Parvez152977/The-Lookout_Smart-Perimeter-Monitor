// ============================================================
// The Lookout — Main Sketch
// ESP32-CAM (AI Thinker RBD-1407) + MB Programmer
//
// UPLOAD PROCEDURE WITH MB BOARD:
//  Open Serial Monitor at 115200 baud for the IP address
//
// BEFORE FIRST USE:
//   - Copy data/secrets.example.h → data/secrets.h
//   - Fill in your WiFi credentials in data/secrets.h
//   - Set your PC's local IP in src/config.h (SERVER_BASE_URL)
// ============================================================

#include "src/config.h"
#include "src/camera.h"
#include "src/wifi_manager.h"
#include "src/pir_sensor.h"
#include "src/uploader.h"
#include "src/heartbeat.h"

// ---- NTP Time Sync -----------------------------------------
// NTP timestamp images with real clock time
// instead of just millis() since boot.
// Uses a public NTP pool — requires internet access.
#define NTP_SERVER "pool.ntp.org"
#define NTP_GMT_OFFSET_SEC 21600  // Bangladesh = UTC+6 = 6*3600
#define NTP_DAYLIGHT_OFFSET 0     // Bangladesh does not observe DST

// ---- State tracking ----------------------------------------
static bool _systemReady = false;

// ============================================================
// setup() — runs once
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(500);  // give Serial time to stabilise before first print

  DBGLN("\n============================================");
  DBGLN("  The Lookout — Booting...");
  DBGLN("============================================");
  DBGF("Camera ID   : %s\n", CAMERA_ID);
  DBGF("Camera Name : %s\n", CAMERA_NAME);
  DBGF("Upload URL  : %s\n", UPLOAD_ENDPOINT);
  DBGLN("--------------------------------------------");

  // ---- Flash LED ----
  // Blink twice on boot so you know the board started
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);
  blinkLED(2, 150);

  // ---- PIR Sensor ----
  initPIR();
  DBGLN("Waiting 3s for PIR to stabilise after init...");
  delay(3000);  // brief settle — full warmup takes 30-60s on first power

  // ---- WiFi ----
  DBGLN("Connecting to WiFi...");
  bool wifiOk = connectWiFi();
  if (!wifiOk) {
    DBGLN("WARNING: Started without WiFi — will retry in background");
    DBGLN("Images will be dropped until WiFi connects");
  }

  // ---- NTP Time Sync ----
  if (wifiOk) {
    DBGLN("Syncing time via NTP...");
    configTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET, NTP_SERVER);
    // Wait up to 5 seconds for NTP
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 5000)) {
      char timebuf[32];
      strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &timeinfo);
      DBGF("Time synced: %s\n", timebuf);
    } else {
      DBGLN("NTP sync failed — timestamps will use uptime fallback");
    }
  }

  // ---- Camera ----
  DBGLN("Initialising camera...");
  bool cameraOk = initCamera();
  if (!cameraOk) {
    // Camera failure is fatal — no point running without it
    DBGLN("FATAL: Camera init failed — halting");
    DBGLN("Check: is the ribbon cable seated? ZIF connector locked?");
    // Blink rapidly to signal hardware fault
    while (true) {
      blinkLED(5, 100);
      delay(500);
    }
  }

  // ---- Heartbeat ----
  initHeartbeat();

  // ---- Ready ----
  _systemReady = true;
  blinkLED(3, 200);  // three slow blinks = ready

  DBGLN("--------------------------------------------");
  DBGLN("System ready. Watching for motion...");
  DBGF("PIR cooldown : %dms\n", CAPTURE_COOLDOWN_MS);
  DBGF("Heartbeat    : every %ds\n", HEARTBEAT_INTERVAL_MS / 1000);
  DBGF("Frame size   : %s\n", psramFound() ? "VGA (640x480)" : "CIF (352x288)");
  DBGLN("============================================\n");
}


void loop() {
  if (!_systemReady) {
    delay(100);
    return;
  }

  // ---- Maintain WiFi connection ----
  // Non-blocking — reconnects automatically if dropped
  maintainWiFi();

  // ---- Send heartbeat if interval has elapsed ----
  // Non-blocking — only fires every HEARTBEAT_INTERVAL_MS
  tickHeartbeat();

  // ---- Check for motion ----
  // motionDetected() handles debouncing and cooldown internally
  if (motionDetected()) {
    handleDetection();
  }

  // Small delay to prevent watchdog timer resets
  // and give the CPU breathing room
  delay(50);
}

// ============================================================
// handleDetection() — called when valid motion is detected
// ============================================================
void handleDetection() {
  DBGLN("\n>>> Motion detected! Capturing...");

  // Flash LED to give a visual indication of capture
  digitalWrite(FLASH_LED_PIN, HIGH);

  // ---- Capture frame ----
  camera_fb_t* fb = captureFrame();

  if (fb == NULL) {
    DBGLN("Capture failed — skipping upload");
    digitalWrite(FLASH_LED_PIN, LOW);
    return;
  }

  // ---- Upload to backend ----
  UploadResult result = uploadFrame(fb);

  // ---- Always release the frame buffer ----
  // Even if upload failed — otherwise camera memory fills up
  releaseFrame(fb);
  fb = NULL;

  // ---- Turn off LED ----
  digitalWrite(FLASH_LED_PIN, LOW);

  // ---- Log result ----
  DBGF("Upload result: %s\n", uploadResultString(result));
  DBGF("Stats — successful: %lu  failed: %lu\n",
       getSuccessfulUploadCount(), getFailedUploadCount());
  DBGLN("");
}

// ============================================================
// blinkLED() — utility to blink the flash LED
// count   = number of blinks
// delayMs = on/off duration in milliseconds
// ============================================================
void blinkLED(int count, int delayMs) {
  for (int i = 0; i < count; i++) {
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(FLASH_LED_PIN, LOW);
    delay(delayMs);
  }
}
