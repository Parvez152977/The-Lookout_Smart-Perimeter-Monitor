// ============================================================
// Smart Perimeter Monitor — WiFi Manager Implementation
// ============================================================

#include "wifi_manager.h"
#include "config.h"
#include "../data/secrets.h"
#include <WiFi.h>

static unsigned long _lastRetryTime = 0;
static bool _wasConnected = false;

// ---- connectWiFi() -----------------------------------------
bool connectWiFi() {
    DBGF("Connecting to WiFi: %s\n", WIFI_SSID);

    WiFi.mode(WIFI_STA);        // Station mode — connect to existing network
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Wait up to 20 seconds for connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        DBG(".");
        attempts++;
    }
    DBGLN("");

    if (WiFi.status() == WL_CONNECTED) {
        _wasConnected = true;
        DBGF("WiFi connected. IP: %s\n", WiFi.localIP().toString().c_str());
        DBGF("Signal strength (RSSI): %d dBm\n", WiFi.RSSI());
        return true;
    } else {
        DBGLN("WiFi connection FAILED — check SSID and password in secrets.h");
        return false;
    }
}

// ---- maintainWiFi() ----------------------------------------
void maintainWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        // All good — do nothing
        if (!_wasConnected) {
            // Just reconnected
            DBGF("WiFi reconnected. IP: %s\n", WiFi.localIP().toString().c_str());
            _wasConnected = true;
        }
        return;
    }

    // Connection lost
    if (_wasConnected) {
        DBGLN("WiFi connection lost — will retry...");
        _wasConnected = false;
    }

    // Only retry every WIFI_RETRY_INTERVAL_MS milliseconds
    // to avoid hammering the router
    unsigned long now = millis();
    if (now - _lastRetryTime < WIFI_RETRY_INTERVAL_MS) {
        return;
    }
    _lastRetryTime = now;

    DBGLN("Attempting WiFi reconnect...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // Non-blocking — we check the result next time maintainWiFi() is called
}

// ---- isWiFiConnected() -------------------------------------
bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

// ---- getLocalIP() ------------------------------------------
String getLocalIP() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}
