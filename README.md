
# 🔭 The Lookout — Smart Perimeter Monitor

<div align="center">

![Version](https://img.shields.io/badge/version-2.0.0-blue)
![ESP32](https://img.shields.io/badge/ESP32-CAM-green)
![FastAPI](https://img.shields.io/badge/FastAPI-0.104.0-teal)
![Python](https://img.shields.io/badge/Python-3.8+-yellow)
![License](https://img.shields.io/badge/license-MIT-orange)

**Motion-triggered security camera system that texts you photos via Telegram**

[Features](#features) • [Hardware](#required-hardware) • [Quick Start](#quick-start) • [Setup Guide](#setup-guide) • [Troubleshooting](#troubleshooting)

</div>

---

## 📖 What is The Lookout?

The Lookout is a **DIY motion-activated security system** built around the ESP32-CAM module. When the PIR sensor detects movement, it instantly captures a photo and sends it to your Telegram account plus it has a web dashboard where you can live view and past detections — all without monthly fees or cloud subscriptions.

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🚀 **Instant Alerts** | Telegram notification within seconds of motion |
| 🖼️ **Photo Evidence** | Captures JPEG images (VGA or CIF resolution) |
| 📊 **Web Dashboard** | Beautiful gallery of all events with auto-refresh |
| 💾 **Local Storage** | All images saved to your PC — full privacy |
| 📱 **Mobile Friendly** | View alerts on any device with Telegram |
| 🔧 **No Monthly Fees** | $0 subscription — runs on your hardware |
| 🎯 **Expandable** | Add multiple cameras to the same system |

---

## 🔧 Required Hardware

### Mandatory Components

| Component | Model | Reference | Notes |
|-----------|-------|-----------|-------|
| **ESP32-CAM Module** | AI Thinker | RBD-1407 | OV2640 camera |
| **MB Programmer Board** | ESP32-CAM-MB | RBD-2044 | CH340G chip, auto-boot circuit |
| **PIR Sensor** | HC-SR501 | — | Motion detection |
| **Micro USB Cable** | Data-capable | — | **MUST support data transfer** |
| **MicroSD Card** | ≤32GB, FAT32 | — | Required for default firmware test |

> 

### Optional
- **5V/1A Power Supply** — For standalone deployment (without PC)
- **100μF Capacitor** — To smooth power spikes if board keeps rebooting

---

## 📸 Hardware Verification

### Step-by-Step Assembly
Step 1: Attach Camera Ribbon
├── Lift brown ZIF connector tab
├── Insert ribbon (gold contacts DOWN)
└── Press tab down firmly

Step 2: Insert MicroSD Card
├── Format FAT32 (max 32GB)
├── Insert into ESP32-CAM slot
└── Gold contacts facing board

Step 3: Snap onto MB Board
├── Camera faces AWAY from USB port
├── Press evenly on both sides
└── Wiggle test → should NOT move

Step 4: Connect PIR Sensor
├── VCC (red) → 5V on ESP32-CAM
├── GND (black/brown) → GND
└── OUT (yellow/orange) → GPIO13

Step 5: USB Connection
└── Connect to PC with DATA cable



### Hardware Test (Before ANY Coding)

**Do not upload any code initially** — test with default firmware first:

1. **Attach the camera ribbon** — gold contacts facing DOWN, lock the brown ZIF tab
2. **Insert MicroSD card** — formatted FAT32, max 32GB
3. **Snap ESP32-CAM onto MB board** — camera faces AWAY from USB port
4. **Connect to PC** using a data-capable Micro USB cable
5. **On your phone** — scan WiFi, connect to `ESP32-CAM` network
6. **Open browser** — go to `192.168.4.1` → click "Start Streaming"

✅ **If you see video** → Hardware is 100% good. Proceed to software setup.

❌ **If no video** → Check ribbon cable seating (90% of failures)

---

## 🖥️ Software Requirements

### Development Environment

| Software | Version | Purpose |
|----------|---------|---------|
| Arduino IDE | 2.x | ESP32 firmware development |
| Python | 3.8+ | Backend server |
| CH340G Driver | Latest | USB-to-serial communication |

### Python Dependencies

```bash
pip install fastapi uvicorn sqlalchemy aiofiles httpx python-multipart

###Or use the provided requirements.txt:

pip install -r requirements.txt
```

