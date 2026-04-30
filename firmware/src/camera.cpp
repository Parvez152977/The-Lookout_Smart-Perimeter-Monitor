// ============================================================
// Smart Perimeter Monitor — Camera Module Implementation
// ============================================================

#include "camera.h"
#include "config.h"

static bool _cameraReady = false;

// ---- initCamera() ------------------------------------------
bool initCamera() {
    camera_config_t config;

    // ---- AI Thinker pin assignments ----
    // These must match the hardware exactly — do not modify
    config.ledc_channel  = LEDC_CHANNEL_0;
    config.ledc_timer    = LEDC_TIMER_0;
    config.pin_d0        = Y2_GPIO_NUM;
    config.pin_d1        = Y3_GPIO_NUM;
    config.pin_d2        = Y4_GPIO_NUM;
    config.pin_d3        = Y5_GPIO_NUM;
    config.pin_d4        = Y6_GPIO_NUM;
    config.pin_d5        = Y7_GPIO_NUM;
    config.pin_d6        = Y8_GPIO_NUM;
    config.pin_d7        = Y9_GPIO_NUM;
    config.pin_xclk      = XCLK_GPIO_NUM;
    config.pin_pclk      = PCLK_GPIO_NUM;
    config.pin_vsync     = VSYNC_GPIO_NUM;
    config.pin_href      = HREF_GPIO_NUM;
    config.pin_sscb_sda  = SIOD_GPIO_NUM;
    config.pin_sscb_scl  = SIOC_GPIO_NUM;
    config.pin_pwdn      = PWDN_GPIO_NUM;
    config.pin_reset     = RESET_GPIO_NUM;
    config.xclk_freq_hz  = 20000000; // 20 MHz — standard for OV2640

    // ---- Format ----
    config.pixel_format  = PIXFORMAT_JPEG;

    // ---- Resolution and quality ----
    // Use higher settings if PSRAM is available (your board has 2MB PSRAM
    // so psramFound() will return true and use the better settings)
    if (psramFound()) {
        DBGLN("PSRAM found — using VGA + dual frame buffers");
        config.frame_size   = FRAME_SIZE;
        config.jpeg_quality = JPEG_QUALITY;
        config.fb_count     = 2; // double buffering for smoother capture
    } else {
        DBGLN("No PSRAM — falling back to CIF resolution");
        config.frame_size   = FRAMESIZE_CIF;
        config.jpeg_quality = 20;
        config.fb_count     = 1;
    }

    // ---- Initialise ----
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        DBGF("Camera init FAILED — error 0x%x\n", err);
        DBGLN("Check: ribbon cable seated? ZIF connector locked?");
        _cameraReady = false;
        return false;
    }

    // ---- Fine-tune sensor settings ----
    // These improve image quality in typical indoor/outdoor conditions
    sensor_t* s = esp_camera_sensor_get();
    if (s != NULL) {
        s->set_brightness(s, 0);     // -2 to 2
        s->set_contrast(s, 0);       // -2 to 2
        s->set_saturation(s, 0);     // -2 to 2
        s->set_sharpness(s, 0);      // -2 to 2
        s->set_denoise(s, 1);        // reduce noise in low light
        s->set_whitebal(s, 1);       // auto white balance on
        s->set_awb_gain(s, 1);       // auto white balance gain on
        s->set_exposure_ctrl(s, 1);  // auto exposure on
        s->set_aec2(s, 1);           // auto exposure correction on
        s->set_gain_ctrl(s, 1);      // auto gain on
        s->set_agc_gain(s, 0);       // start gain at 0
        s->set_gainceiling(s, (gainceiling_t)2); // max gain ceiling
        s->set_bpc(s, 1);            // black pixel correction
        s->set_wpc(s, 1);            // white pixel correction
        s->set_raw_gma(s, 1);        // gamma correction
        s->set_lenc(s, 1);           // lens correction
        s->set_hmirror(s, 0);        // flip horizontal? 0=no, 1=yes
        s->set_vflip(s, 0);          // flip vertical?   0=no, 1=yes
    }

    DBGLN("Camera initialised successfully");
    _cameraReady = true;
    return true;
}

// ---- captureFrame() ----------------------------------------
camera_fb_t* captureFrame() {
    if (!_cameraReady) {
        DBGLN("captureFrame: camera not ready");
        return NULL;
    }

    // Discard the first frame — camera auto-exposure needs
    // a moment to settle after being idle. Without this the
    // first captured image is often too dark or blown out.
    camera_fb_t* discard = esp_camera_fb_get();
    if (discard) {
        esp_camera_fb_return(discard);
    }
    delay(FRAME_SETTLE_DELAY_MS);

    // Capture the actual frame
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        DBGLN("captureFrame: frame buffer capture failed");
        return NULL;
    }

    DBGF("Frame captured: %d bytes (%dx%d)\n",
         fb->len, fb->width, fb->height);

    return fb;
}

// ---- releaseFrame() ----------------------------------------
void releaseFrame(camera_fb_t* fb) {
    if (fb != NULL) {
        esp_camera_fb_return(fb);
    }
}

// ---- isCameraReady() ---------------------------------------
bool isCameraReady() {
    return _cameraReady;
}
