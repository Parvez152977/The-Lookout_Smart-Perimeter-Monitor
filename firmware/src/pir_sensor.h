#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

// ============================================================
// Smart Perimeter Monitor — PIR Sensor Header
// Handles HC-SR501 debouncing and capture cooldown
// ============================================================

#include <Arduino.h>

// Initialise the PIR sensor GPIO pin.
// Call once in setup().
void initPIR();

// Returns true if the PIR has detected motion AND the cooldown
// period since the last detection has elapsed.
//
// This is the function to call in loop().
// It handles all debouncing and cooldown internally —
// you don't need any extra delay() after calling this.
bool motionDetected();

// Returns the timestamp (millis()) of the last detection.
// Useful for debugging or logging.
unsigned long getLastDetectionTime();

// Returns how many total detections have occurred since boot.
unsigned long getDetectionCount();

#endif // PIR_SENSOR_H
