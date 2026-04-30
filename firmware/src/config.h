#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// Smart Perimeter Monitor — Central Configuration
// Edit values here to tune behaviour without touching logic
// ============================================================

// ---- Server ------------------------------------------------
// Your PC's local IP address (run ipconfig on Windows)
// Must be on the same WiFi network as the ESP32-CAM
#define SERVER_BASE_URL     "http://192.168.1.100:8000"
#define UPLOAD_ENDPOINT     SERVER_BASE_URL "/api/v1/upload"
#define HEARTBEAT_ENDPOINT  SERVER_BASE_URL "/api/v1/heartbeat"

// ---- Camera Identity ---------------------------------------
// Unique ID for this camera — change for each unit you deploy
#define CAMERA_ID           "cam1"
#define CAMERA_NAME         "Front Door"

// ---- GPIO Pins ---------------------------------------------
// HC-SR501 PIR sensor signal wire
#define PIR_PIN             13

// Onboard flash LED (active HIGH — turns ON when set HIGH)
#define FLASH_LED_PIN       4

// ---- Camera Settings ---------------------------------------
// FRAMESIZE options (comment/uncomment one):
//   FRAMESIZE_QQVGA  — 160x120  (fastest upload, lowest detail)
//   FRAMESIZE_QVGA   — 320x240  (good balance)
//   FRAMESIZE_CIF    — 352x288
//   FRAMESIZE_VGA    — 640x480  (recommended — best balance)
//   FRAMESIZE_SVGA   — 800x600  (slower, larger files)
#define FRAME_SIZE          FRAMESIZE_VGA

// JPEG quality: 0-63, lower number = better quality, larger file
// 10-15 is high quality, 20-25 is medium, 30+ is low
#define JPEG_QUALITY        12

// ---- Timing ------------------------------------------------
// How long to wait between captures after a detection (ms)
// Prevents image flooding. 5000 = 5 seconds minimum between shots
#define CAPTURE_COOLDOWN_MS         5000

// How often to send a heartbeat ping to mark camera as online (ms)
// Backend uses this to show online/offline status on the dashboard
#define HEARTBEAT_INTERVAL_MS       30000

// How long to wait for the HTTP upload to complete (ms)
// If your network is slow, increase this
#define HTTP_TIMEOUT_MS             10000

// How many milliseconds to discard the first frame after capture
// Camera auto-exposure needs a moment to settle
#define FRAME_SETTLE_DELAY_MS       150

// WiFi reconnect retry interval (ms)
#define WIFI_RETRY_INTERVAL_MS      5000

// ---- Serial Debug ------------------------------------------
// Set to 1 to enable Serial.print debug output, 0 to disable
#define DEBUG_ENABLED       1

#if DEBUG_ENABLED
  #define DBG(x)    Serial.print(x)
  #define DBGLN(x)  Serial.println(x)
  #define DBGF(...) Serial.printf(__VA_ARGS__)
#else
  #define DBG(x)
  #define DBGLN(x)
  #define DBGF(...)
#endif

// ---- AI Thinker ESP32-CAM Pin Map --------------------------
// These are FIXED for the AI Thinker board — do NOT change
#define PWDN_GPIO_NUM       32
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM        0
#define SIOD_GPIO_NUM       26
#define SIOC_GPIO_NUM       27
#define Y9_GPIO_NUM         35
#define Y8_GPIO_NUM         34
#define Y7_GPIO_NUM         39
#define Y6_GPIO_NUM         36
#define Y5_GPIO_NUM         21
#define Y4_GPIO_NUM         19
#define Y3_GPIO_NUM         18
#define Y2_GPIO_NUM          5
#define VSYNC_GPIO_NUM      25
#define HREF_GPIO_NUM       23
#define PCLK_GPIO_NUM       22

#endif // CONFIG_H
