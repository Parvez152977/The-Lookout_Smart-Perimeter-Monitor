#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

// ============================================================
// Smart Perimeter Monitor — WiFi Manager Header
// Handles initial connection and automatic reconnection
// ============================================================

#include <Arduino.h>

// Connect to WiFi using credentials from secrets.h
// Blocks until connected or times out after 20 seconds.
// Returns true if connected, false on timeout.
bool connectWiFi();

// Call this in loop() — checks if still connected and
// attempts to reconnect automatically if the connection dropped.
// Non-blocking: only retries after WIFI_RETRY_INTERVAL_MS.
void maintainWiFi();

// Returns true if currently connected to WiFi.
bool isWiFiConnected();

// Returns the current local IP address as a string.
// Returns "0.0.0.0" if not connected.
String getLocalIP();

#endif // WIFI_MANAGER_H
