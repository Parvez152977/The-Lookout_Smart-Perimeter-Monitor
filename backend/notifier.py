"""
notifier.py - Telegram alert system with rate limiting and retry logic
"""

import asyncio
import httpx
from datetime import datetime, timedelta
from pathlib import Path
import logging

# Import config
from config import BOT_TOKEN, CHAT_ID, TELEGRAM_ENABLED, TELEGRAM_RATE_LIMIT, MAX_RETRIES

# Set up logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# ========== RATE LIMITING ==========
class RateLimiter:
    """Simple rate limiter for Telegram messages"""
    def __init__(self, min_interval_seconds: float = 1.0):
        self.min_interval = min_interval_seconds
        self.last_sent = None
    
    async def wait_if_needed(self):
        """Wait if we're sending too fast"""
        if self.last_sent:
            elapsed = (datetime.now() - self.last_sent).total_seconds()
            if elapsed < self.min_interval:
                wait_time = self.min_interval - elapsed
                logger.info(f"Rate limiting: waiting {wait_time:.1f}s")
                await asyncio.sleep(wait_time)
        self.last_sent = datetime.now()

# Global rate limiter instance
rate_limiter = RateLimiter(TELEGRAM_RATE_LIMIT)

# ========== TELEGRAM MESSAGE SENDER ==========
async def send_telegram_photo(image_path: str, caption: str, retry_count: int = 0):
    """
    Send a photo to Telegram with retry logic
    
    Args:
        image_path: Path to the image file
        caption: Text caption for the photo
        retry_count: Current retry attempt (internal use)
    
    Returns:
        bool: True if successful, False otherwise
    """
    if not TELEGRAM_ENABLED:
        logger.info("Telegram alerts disabled (TELEGRAM_ENABLED = False)")
        return False
    
    if BOT_TOKEN == "YOUR_BOT_TOKEN_HERE" or CHAT_ID == "YOUR_CHAT_ID_HERE":
        logger.warning("Telegram not configured — set BOT_TOKEN and CHAT_ID in config.py")
        return False
    
    # Check if image exists
    image_file = Path(image_path)
    if not image_file.exists():
        logger.error(f"Image not found: {image_path}")
        return False
    
    # Apply rate limiting
    await rate_limiter.wait_if_needed()
    
    # Build Telegram API URL
    url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendPhoto"
    
    # Prepare caption with timestamp
    full_caption = f"🚨 **The Lookout — Motion Detected!**\n\n📸 {caption}\n\n🔭 Stay vigilant."
    
    try:
        async with httpx.AsyncClient(timeout=15.0) as client:
            with open(image_path, "rb") as photo_file:
                files = {"photo": ("image.jpg", photo_file, "image/jpeg")}
                data = {
                    "chat_id": CHAT_ID,
                    "caption": full_caption,
                    "parse_mode": "Markdown"
                }
                
                response = await client.post(url, data=data, files=files)
                
                if response.status_code == 200:
                    logger.info(f"✓ Telegram alert sent successfully")
                    return True
                elif response.status_code == 429:
                    # Rate limit from Telegram's side
                    retry_after = response.json().get("parameters", {}).get("retry_after", 5)
                    logger.warning(f"Telegram rate limit (429). Retry after {retry_after}s")
                    
                    if retry_count < MAX_RETRIES:
                        await asyncio.sleep(retry_after)
                        return await send_telegram_photo(image_path, caption, retry_count + 1)
                    else:
                        logger.error(f"Max retries reached for Telegram")
                        return False
                else:
                    logger.error(f"Telegram API error: {response.status_code} - {response.text}")
                    return False
                    
    except httpx.TimeoutException:
        logger.error("Telegram request timeout")
        if retry_count < MAX_RETRIES:
            await asyncio.sleep(2)
            return await send_telegram_photo(image_path, caption, retry_count + 1)
        return False
        
    except Exception as e:
        logger.error(f"Telegram send failed: {str(e)}")
        return False

# ========== MAIN ALERT FUNCTION ==========
async def send_alert(image_path: str, timestamp_str: str, camera_id: str = "cam1"):
    """
    Main entry point for sending alerts
    
    Args:
        image_path: Path to the captured image
        timestamp_str: Human-readable timestamp (e.g., "2024-03-15 14:30:22")
        camera_id: Camera identifier (for multi-camera setups)
    """
    logger.info(f"Preparing Telegram alert for {image_path}")
    
    # Format the caption
    caption = f"`{timestamp_str}` | Camera: `{camera_id}`"
    
    # Send the photo
    success = await send_telegram_photo(image_path, caption)
    
    if success:
        logger.info("✅ Alert delivered successfully")
    else:
        logger.warning("❌ Alert delivery failed")
    
    return success

# ========== TEST FUNCTION ==========
async def test_alert():
    """Send a test alert to verify Telegram configuration"""
    print("Testing Telegram alert...")
    
    # Create a simple test message (no photo)
    if not TELEGRAM_ENABLED:
        print("Telegram disabled. Set TELEGRAM_ENABLED = True in config.py")
        return
    
    url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"
    
    async with httpx.AsyncClient() as client:
        response = await client.post(url, json={
            "chat_id": CHAT_ID,
            "text": "🔔 **The Lookout** is online and ready!\n\nTelegram alerts configured successfully.",
            "parse_mode": "Markdown"
        })
        
        if response.status_code == 200:
            print("✅ Test message sent! Check your Telegram.")
        else:
            print(f"❌ Failed: {response.status_code} - {response.text}")

if __name__ == "__main__":
    # Run test
    asyncio.run(test_alert())