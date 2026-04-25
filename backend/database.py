"""
database.py - SQLite database models and utilities for The Lookout
"""

from sqlalchemy import create_engine, Column, Integer, String, DateTime, Index
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session
from datetime import datetime, timezone
import os
from pathlib import Path

# Import config
from config import DB_PATH

# ========== DATABASE SETUP ==========
# Ensure the directory for the database exists
DB_PATH.parent.mkdir(parents=True, exist_ok=True)

# Create engine with SQLite optimizations
engine = create_engine(
    f"sqlite:///{DB_PATH}",
    connect_args={
        "check_same_thread": False,
        "timeout": 10  # Prevents database locked errors
    },
    echo=False  # Set to True for SQL debugging
)

SessionLocal = sessionmaker(bind=engine)
Base = declarative_base()

# ========== MODEL DEFINITION ==========
class DetectionEvent(Base):
    __tablename__ = "events"
    
    id = Column(Integer, primary_key=True, index=True, autoincrement=True)
    filename = Column(String, nullable=False)
    camera_id = Column(String, default="cam1", nullable=False)
    timestamp = Column(DateTime, default=datetime.utcnow, nullable=False, index=True)
    file_size_bytes = Column(Integer, default=0)  # NEW: track file size
    ip_address = Column(String, nullable=True)    # NEW: source IP for debugging
    
    # Composite index for faster queries
    __table_args__ = (
        Index('idx_timestamp_camera', 'timestamp', 'camera_id'),
    )
    
    def to_dict(self):
        """Convert event to dictionary for JSON responses"""
        return {
            "id": self.id,
            "filename": self.filename,
            "camera_id": self.camera_id,
            "timestamp": self.timestamp.isoformat(),
            "url": f"/uploads/{self.filename}"
        }

# ========== DATABASE HELPER FUNCTIONS ==========
def init_db():
    """Create all tables if they don't exist"""
    Base.metadata.create_all(bind=engine)
    print(f"✓ Database initialized at: {DB_PATH}")

def get_db():
    """Dependency for FastAPI to get database session"""
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

def add_event(db: Session, filename: str, camera_id: str = "cam1", file_size: int = 0, ip: str = None):
    """Add a new detection event to the database"""
    event = DetectionEvent(
        filename=filename,
        camera_id=camera_id,
        timestamp=datetime.utcnow(),
        file_size_bytes=file_size,
        ip_address=ip
    )
    db.add(event)
    db.commit()
    db.refresh(event)
    return event

def get_recent_events(db: Session, limit: int = 50):
    """Get most recent events"""
    return db.query(DetectionEvent).order_by(
        DetectionEvent.timestamp.desc()
    ).limit(limit).all()

def get_event_count(db: Session):
    """Get total number of events"""
    return db.query(DetectionEvent).count()

def get_events_last_hour(db: Session):
    """Get number of events in the last hour"""
    one_hour_ago = datetime.utcnow().replace(minute=0, second=0, microsecond=0)
    return db.query(DetectionEvent).filter(
        DetectionEvent.timestamp >= one_hour_ago
    ).count()

def get_unique_days(db: Session):
    """Get number of unique days with events"""
    from sqlalchemy import func
    result = db.query(
        func.count(func.distinct(func.date(DetectionEvent.timestamp)))
    ).scalar()
    return result or 0

def delete_old_events(db: Session, days_to_keep: int = 30):
    """Delete events older than specified days (for cleanup)"""
    cutoff_date = datetime.utcnow().replace(hour=0, minute=0, second=0, microsecond=0)
    from datetime import timedelta
    cutoff_date -= timedelta(days=days_to_keep)
    
    deleted = db.query(DetectionEvent).filter(
        DetectionEvent.timestamp < cutoff_date
    ).delete()
    db.commit()
    return deleted

# ========== INITIALIZE ON IMPORT ==========
init_db()

if __name__ == "__main__":
    # Test database functions
    print("Testing database...")
    db = SessionLocal()
    print(f"Total events: {get_event_count(db)}")
    print(f"Unique days: {get_unique_days(db)}")
    db.close()