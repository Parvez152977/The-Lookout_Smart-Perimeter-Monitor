#ifndef CAMERA_H
#define CAMERA_H

// ============================================================
// Smart Perimeter Monitor — Camera Module Header
// Handles OV2640 initialisation and frame capture
// ============================================================

#include "esp_camera.h"
#include <stdbool.h>

// Initialise the OV2640 camera with the AI Thinker pin map.
// Must be called once in setup() before any capture attempt.
// Returns true on success, false on failure.
// On failure: check ribbon cable seating in the ZIF connector.
bool initCamera();

// Capture a single JPEG frame from the camera.
// Returns a pointer to the frame buffer on success, NULL on failure.
//
// IMPORTANT: You MUST call releaseFrame() after you are done
// with the frame buffer, otherwise the camera memory fills up
// and subsequent captures will fail.
camera_fb_t* captureFrame();

// Release a frame buffer back to the camera driver.
// Always call this after captureFrame(), even if you didn't use the data.
void releaseFrame(camera_fb_t* fb);

// Returns true if the camera was successfully initialised.
bool isCameraReady();

#endif // CAMERA_H
