#ifndef UPLOADER_H
#define UPLOADER_H

// ============================================================
// Smart Perimeter Monitor — HTTP Uploader Header
// Sends captured JPEG frames to the FastAPI backend
// ============================================================

#include "esp_camera.h"

// Upload result codes
typedef enum {
    UPLOAD_OK           = 200, // server accepted the image
    UPLOAD_WIFI_DOWN    = -1,  // no WiFi connection
    UPLOAD_NULL_FRAME   = -2,  // frame buffer was NULL
    UPLOAD_HTTP_FAILED  = -3,  // HTTP request failed to send
    UPLOAD_SERVER_ERROR = -4   // server returned non-200 status
} UploadResult;

// Upload a JPEG frame buffer to the backend /upload endpoint.
// Adds camera ID and timestamp headers automatically.
// Returns an UploadResult code.
//
// The caller keeps ownership of the frame buffer —
// this function does NOT call releaseFrame().
UploadResult uploadFrame(camera_fb_t* fb);

// Returns a human-readable string for an UploadResult code.
const char* uploadResultString(UploadResult result);

// Returns the total number of successful uploads since boot.
unsigned long getSuccessfulUploadCount();

// Returns the total number of failed uploads since boot.
unsigned long getFailedUploadCount();

#endif // UPLOADER_H
