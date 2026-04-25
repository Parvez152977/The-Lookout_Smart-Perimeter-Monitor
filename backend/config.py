"""
config.py - Centralized configuration for The Lookout
"""

import os
from pathlib import Path

# ========== BASE PATHS ==========
BASE_DIR = Path(__file__).parent.parent  # Goes up to project root
BACKEND_DIR = Path(__file__).parent
UPLOADS_DIR = BACKEND_DIR / "uploads"
DB_PATH = BASE_DIR / "perimeter.db"

# ========== SERVER CONFIGURATION ==========
HOST = "0.0.0.0"
PORT = 8000

# ========== TELEGRAM CONFIGURATION ==========
# Get these from @BotFather and /getUpdates
BOT_TOKEN = "YOUR_BOT_TOKEN_HERE"  # ← REPLACE THIS
CHAT_ID = "YOUR_CHAT_ID_HERE"      # ← REPLACE THIS

# ========== TELEGRAM BEHAVIOR ==========
TELEGRAM_ENABLED = True  # Set to False to disable alerts
TELEGRAM_RATE_LIMIT = 1.0  # Minimum seconds between messages to same chat
MAX_RETRIES = 3  # Number of retries for failed Telegram sends

# ========== IMAGE SETTINGS ==========
MAX_IMAGE_SIZE_MB = 10  # Max JPEG size to accept
IMAGE_QUALITY = 85  # If you re-encode images (not used for direct save)

# ========== SECURITY (Optional) ==========
# Enable if you want to require an API key from ESP32
API_KEY_ENABLED = False
API_KEY = "your-secret-api-key-here"

# ========== CREATE REQUIRED DIRECTORIES ==========
def ensure_directories():
    """Create uploads directory if it doesn't exist"""
    UPLOADS_DIR.mkdir(exist_ok=True)
    print(f"✓ Uploads directory: {UPLOADS_DIR}")

# ========== VALIDATE CONFIGURATION ==========
def validate_config():
    """Check if critical config is set"""
    warnings = []
    if BOT_TOKEN == "YOUR_BOT_TOKEN_HERE":
        warnings.append("⚠️ BOT_TOKEN not configured — Telegram alerts disabled")
    if CHAT_ID == "YOUR_CHAT_ID_HERE":
        warnings.append("⚠️ CHAT_ID not configured — Telegram alerts disabled")
    
    for w in warnings:
        print(w)
    
    return len(warnings) == 0

if __name__ == "__main__":
    ensure_directories()
    validate_config()