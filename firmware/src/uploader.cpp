// ============================================================
// Smart Perimeter Monitor — HTTP Uploader Implementation
// ============================================================

#include "uploader.h"
#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

static unsigned long _successCount = 0;
static unsigned long _failCount    = 0;

// ---- getTimestamp() (internal helper) ----------------------
// Returns current time as ISO-8601 string if NTP is configured,
// otherwise returns millis()-based fallback string.
static String getTimestamp() {
    // Try to get real time via NTP
    // NTP sync is attempted once at boot (see main.ino setup)
    time_t now;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo);
        return String(buf);
    }
    // Fallback: uptime in seconds since boot
    return "uptime_" + String(millis() / 1000) + "s";
}

// ---- uploadFrame() -----------------------------------------
UploadResult uploadFrame(camera_fb_t* fb) {

    // Guard: need WiFi
    if (!isWiFiConnected()) {
        DBGLN("Upload skipped — no WiFi");
        _failCount++;
        return UPLOAD_WIFI_DOWN;
    }

    // Guard: need a valid frame
    if (fb == NULL || fb->buf == NULL || fb->len == 0) {
        DBGLN("Upload skipped — null or empty frame");
        _failCount++;
        return UPLOAD_NULL_FRAME;
    }

    DBGF("Uploading %d bytes to %s\n", fb->len, UPLOAD_ENDPOINT);

    HTTPClient http;
    http.begin(UPLOAD_ENDPOINT);
    http.setTimeout(HTTP_TIMEOUT_MS);

    // ---- Headers ----
    // Content-Type tells the server this is raw JPEG binary
    http.addHeader("Content-Type",  "image/jpeg");

    // Camera identity headers — backend uses these to log
    // which camera sent the image and when
    http.addHeader("X-Camera-ID",   CAMERA_ID);
    http.addHeader("X-Camera-Name", CAMERA_NAME);
    http.addHeader("X-Timestamp",   getTimestamp());

    // ---- POST the raw JPEG bytes ----
    int httpCode = http.POST(fb->buf, fb->len);

    http.end(); // always close the connection

    if (httpCode <= 0) {
        // httpCode is negative — a connection-level error, not an HTTP status
        // Common values:
        //   HTTPC_ERROR_CONNECTION_REFUSED = -1
        //   HTTPC_ERROR_CONNECTION_LOST    = -5
        //   HTTPC_ERROR_READ_TIMEOUT       = -11
        DBGF("HTTP POST failed — error code: %d\n", httpCode);
        DBGLN("Is the backend server running? Is the IP correct in config.h?");
        _failCount++;
        return UPLOAD_HTTP_FAILED;
    }

    if (httpCode == 200 || httpCode == 201) {
        _successCount++;
        DBGF("Upload OK (HTTP %d) — total successful: %lu\n",
             httpCode, _successCount);
        return UPLOAD_OK;
    } else {
        DBGF("Server returned HTTP %d\n", httpCode);
        _failCount++;
        return UPLOAD_SERVER_ERROR;
    }
}

// ---- uploadResultString() ----------------------------------
const char* uploadResultString(UploadResult result) {
    switch (result) {
        case UPLOAD_OK:           return "OK";
        case UPLOAD_WIFI_DOWN:    return "No WiFi";
        case UPLOAD_NULL_FRAME:   return "Null frame";
        case UPLOAD_HTTP_FAILED:  return "HTTP connection failed";
        case UPLOAD_SERVER_ERROR: return "Server error";
        default:                  return "Unknown";
    }
}

// ---- getSuccessfulUploadCount() ----------------------------
unsigned long getSuccessfulUploadCount() {
    return _successCount;
}

// ---- getFailedUploadCount() --------------------------------
unsigned long getFailedUploadCount() {
    return _failCount;
}
