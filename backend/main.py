"""
main.py - FastAPI backend for The Lookout Smart Perimeter Monitor
"""

import os
import asyncio
import logging
from datetime import datetime
from pathlib import Path

# FastAPI imports
from fastapi import FastAPI, Request, HTTPException, Depends
from fastapi.responses import JSONResponse, FileResponse
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware

# Local imports
import aiofiles
from sqlalchemy.orm import Session

from config import (
    UPLOADS_DIR, HOST, PORT, 
    MAX_IMAGE_SIZE_MB, API_KEY_ENABLED, API_KEY,
    ensure_directories
)
from database import get_db, add_event, get_recent_events, get_event_count, get_events_last_hour, get_unique_days
from notifier import send_alert

# ========== LOGGING SETUP ==========
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# ========== FASTAPI APP INIT ==========
app = FastAPI(
    title="The Lookout - Smart Perimeter Monitor",
    description="Motion-triggered security camera system with Telegram alerts",
    version="2.0.0"
)

# Enable CORS (for development — restrict in production)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Change to specific IPs in production
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ========== DIRECTORY SETUP ==========
ensure_directories()

# Mount static files
app.mount("/uploads", StaticFiles(directory=str(UPLOADS_DIR)), name="uploads")

# Dashboard mounting (CHECK YOUR PATH)
DASHBOARD_PATH = Path(__file__).parent.parent / "dashboard"
if DASHBOARD_PATH.exists():
    app.mount("/dashboard", StaticFiles(directory=str(DASHBOARD_PATH), html=True), name="dashboard")
    logger.info(f"✓ Dashboard mounted from {DASHBOARD_PATH}")
else:
    logger.warning(f"⚠️ Dashboard not found at {DASHBOARD_PATH}")

# ========== API KEY VALIDATION (OPTIONAL) ==========
async def verify_api_key(request: Request):
    """Optional API key verification"""
    if not API_KEY_ENABLED:
        return True
    
    api_key = request.headers.get("X-API-Key")
    if not api_key or api_key != API_KEY:
        raise HTTPException(status_code=401, detail="Invalid API Key")
    return True

# ========== ENDPOINTS ==========

@app.get("/")
async def root():
    """Root endpoint with API info"""
    return {
        "name": "The Lookout - Smart Perimeter Monitor",
        "version": "2.0.0",
        "status": "online",
        "endpoints": {
            "/upload": "POST - Receive motion images",
            "/events": "GET - List recent events",
            "/stats": "GET - Get statistics",
            "/dashboard": "GET - Web dashboard"
        }
    }

@app.post("/upload")
async def upload_image(
    request: Request,
    db: Session = Depends(get_db)
):
    """
    Receive JPEG image from ESP32-CAM
    """
    # Check content length
    content_length = request.headers.get("content-length")
    if content_length and int(content_length) > MAX_IMAGE_SIZE_MB * 1024 * 1024:
        logger.warning(f"Image too large: {content_length} bytes")
        raise HTTPException(status_code=413, detail="Image too large")
    
    # Read image bytes
    try:
        body = await request.body()
    except Exception as e:
        logger.error(f"Failed to read request body: {e}")
        raise HTTPException(status_code=400, detail="Failed to read image data")
    
    if len(body) < 1000:  # Less than ~1KB is probably invalid
        logger.warning(f"Suspiciously small image: {len(body)} bytes")
        raise HTTPException(status_code=400, detail="Invalid image data")
    
    # Generate filename with timestamp
    timestamp = datetime.now()
    filename = f"motion_{timestamp.strftime('%Y%m%d_%H%M%S')}.jpg"
    filepath = UPLOADS_DIR / filename
    
    # Save image to disk
    try:
        async with aiofiles.open(filepath, 'wb') as f:
            await f.write(body)
        logger.info(f"✓ Saved image: {filename} ({len(body)} bytes)")
    except Exception as e:
        logger.error(f"Failed to save image: {e}")
        raise HTTPException(status_code=500, detail="Failed to save image")
    
    # Get client IP (for debugging)
    client_ip = request.client.host if request.client else None
    
    # Save to database
    try:
        event = add_event(
            db=db,
            filename=filename,
            camera_id="cam1",
            file_size=len(body),
            ip=client_ip
        )
        logger.info(f"✓ Database record created: event #{event.id}")
    except Exception as e:
        logger.error(f"Database error: {e}")
        # Don't fail the request, just log it
    
    # Send Telegram alert (non-blocking)
    timestamp_str = timestamp.strftime("%Y-%m-%d %H:%M:%S")
    asyncio.create_task(send_alert(str(filepath), timestamp_str, "cam1"))
    
    return {
        "status": "ok",
        "file": filename,
        "event_id": event.id if event else None,
        "size_bytes": len(body)
    }

@app.get("/events")
async def get_events(
    limit: int = 50,
    db: Session = Depends(get_db)
):
    """
    Get recent detection events
    """
    if limit > 200:
        limit = 200  # Cap at 200
    
    events = get_recent_events(db, limit)
    return [event.to_dict() for event in events]

@app.get("/stats")
async def get_stats(db: Session = Depends(get_db)):
    """
    Get system statistics
    """
    total_events = get_event_count(db)
    last_hour = get_events_last_hour(db)
    unique_days = get_unique_days(db)
    
    # Get disk usage
    uploads_size = sum(f.stat().st_size for f in UPLOADS_DIR.glob("*.jpg")) if UPLOADS_DIR.exists() else 0
    
    return {
        "total_events": total_events,
        "events_last_hour": last_hour,
        "unique_days_active": unique_days,
        "storage_used_mb": round(uploads_size / (1024 * 1024), 2),
        "status": "active"
    }

@app.get("/image/{filename}")
async def get_image(filename: str):
    """
    Serve a specific image by filename
    """
    filepath = UPLOADS_DIR / filename
    if not filepath.exists():
        raise HTTPException(status_code=404, detail="Image not found")
    return FileResponse(filepath)

# ========== HEALTH CHECK ==========
@app.get("/health")
async def health_check():
    """Simple health check endpoint"""
    return {
        "status": "healthy",
        "timestamp": datetime.utcnow().isoformat(),
        "uploads_dir_exists": UPLOADS_DIR.exists()
    }

# ========== STARTUP EVENT ==========
@app.on_event("startup")
async def startup_event():
    """Run on server startup"""
    logger.info("=" * 50)
    logger.info("🔭 The Lookout - Smart Perimeter Monitor")
    logger.info("=" * 50)
    logger.info(f"Uploads directory: {UPLOADS_DIR}")
    logger.info(f"Dashboard: http://{HOST}:{PORT}/dashboard")
    logger.info(f"API docs: http://{HOST}:{PORT}/docs")
    logger.info("=" * 50)

# ========== RUN (for development) ==========
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "main:app",
        host=HOST,
        port=PORT,
        reload=True,
        log_level="info"
    )