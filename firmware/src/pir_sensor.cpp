// ============================================================
// Smart Perimeter Monitor — PIR Sensor Implementation
// ============================================================

#include "pir_sensor.h"
#include "config.h"

static unsigned long _lastDetectionTime = 0;
static unsigned long _detectionCount    = 0;

// Software debounce state
static bool          _lastRawState      = false;
static unsigned long _debounceStart     = 0;
#define DEBOUNCE_MS 50

// ---- initPIR() ---------------------------------------------
void initPIR() {
    pinMode(PIR_PIN, INPUT);

    // The HC-SR501 has a 30-60 second warmup period on first power.
    // During warmup it can trigger randomly — this is normal hardware
    // behaviour, not a wiring problem.
    // We do NOT add a blocking delay here — the main sketch handles
    // the boot sequence timing.

    DBGF("PIR sensor initialised on GPIO%d\n", PIR_PIN);
    DBGLN("Note: HC-SR501 has 30-60s warmup — expect false triggers initially");
}

// ---- motionDetected() --------------------------------------
bool motionDetected() {
    bool currentRaw = (digitalRead(PIR_PIN) == HIGH);
    unsigned long now = millis();

    // ---- Software debounce ----
    // The PIR output can briefly bounce when transitioning.
    // We only accept a state change after it's been stable
    // for DEBOUNCE_MS milliseconds.
    if (currentRaw != _lastRawState) {
        _debounceStart = now;
        _lastRawState  = currentRaw;
        return false;
    }

    // State hasn't changed — check if it's been stable long enough
    if ((now - _debounceStart) < DEBOUNCE_MS) {
        return false; // still within debounce window
    }

    // ---- Only act on HIGH (motion present) ----
    if (!currentRaw) {
        return false;
    }

    // ---- Cooldown check ----
    // Enforce minimum time between captures.
    // Without this a person walking slowly could trigger
    // dozens of uploads in quick succession.
    if ((now - _lastDetectionTime) < CAPTURE_COOLDOWN_MS) {
        return false; // still in cooldown
    }

    // ---- Valid detection ----
    _lastDetectionTime = now;
    _detectionCount++;

    DBGF("Motion detected! (total: %lu, PIR HIGH for %lums)\n",
         _detectionCount, now - _debounceStart);

    return true;
}

// ---- getLastDetectionTime() --------------------------------
unsigned long getLastDetectionTime() {
    return _lastDetectionTime;
}

// ---- getDetectionCount() -----------------------------------
unsigned long getDetectionCount() {
    return _detectionCount;
}
