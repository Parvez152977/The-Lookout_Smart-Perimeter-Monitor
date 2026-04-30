#ifndef HEARTBEAT_H
#define HEARTBEAT_H

// ============================================================
// Smart Perimeter Monitor — Heartbeat Header
// Sends periodic pings so the dashboard shows online/offline
// ============================================================

#include <Arduino.h>

// Initialise the heartbeat timer.
// Call once in setup().
void initHeartbeat();

// Call in loop() — sends a heartbeat to the backend if the
// interval has elapsed. Non-blocking: returns immediately
// if it's not time to send yet.
void tickHeartbeat();

// Returns the millis() timestamp of the last successful heartbeat.
unsigned long getLastHeartbeatTime();

#endif // HEARTBEAT_H
