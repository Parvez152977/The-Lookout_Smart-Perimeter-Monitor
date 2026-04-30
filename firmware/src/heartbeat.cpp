// ============================================================
// Smart Perimeter Monitor — Heartbeat Implementation
// ============================================================

#include "heartbeat.h"
#include "wifi_manager.h"
#include "config.h"
#include <HTTPClient.h>

static unsigned long _lastHeartbeatTime     = 0;
static unsigned long _lastHeartbeatAttempt  = 0;

// ---- initHeartbeat() ---------------------------------------
void initHeartbeat() {
    _lastHeartbeatTime    = 0;
    _lastHeartbeatAttempt = 0;
    DBGLN("Heartbeat module initialised");
}

// ---- tickHeartbeat() ---------------------------------------
void tickHeartbeat() {
    unsigned long now = millis();

    // Only send every HEARTBEAT_INTERVAL_MS milliseconds
    if ((now - _lastHeartbeatAttempt) < HEARTBEAT_INTERVAL_MS) {
        return;
    }
    _lastHeartbeatAttempt = now;

    // Need WiFi to send
    if (!isWiFiConnected()) {
        DBGLN("Heartbeat skipped — no WiFi");
        return;
    }

    // Build the URL with camera ID as a query parameter
    // Backend endpoint: GET /api/v1/heartbeat?cam_id=cam1
    String url = String(HEARTBEAT_ENDPOINT) + "?cam_id=" + CAMERA_ID;

    HTTPClient http;
    http.begin(url);
    http.setTimeout(5000); // short timeout — heartbeats are lightweight

    int httpCode = http.GET();
    http.end();

    if (httpCode == 200) {
        _lastHeartbeatTime = now;
        DBGF("Heartbeat sent OK (every %ds)\n", HEARTBEAT_INTERVAL_MS / 1000);
    } else {
        DBGF("Heartbeat failed — HTTP %d\n", httpCode);
    }
}

// ---- getLastHeartbeatTime() --------------------------------
unsigned long getLastHeartbeatTime() {
    return _lastHeartbeatTime;
}
