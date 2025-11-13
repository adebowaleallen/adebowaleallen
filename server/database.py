"""
Database models for Student Response System

This module defines the database schema using SQLAlchemy ORM.
Tables: Questions, Responses, Students
"""

from flask_sqlalchemy import SQLAlchemy
from datetime import datetime

db = SQLAlchemy()

class Question(db.Model):
    """Questions created by teachers"""
    __tablename__ = 'questions'

    id = db.Column(db.Integer, primary_key=True)
    question_text = db.Column(db.Text, nullable=False)
    question_type = db.Column(db.String(20), nullable=False)  # 'yes_no', 'multiple_choice', 'true_false'
    options = db.Column(db.JSON)  # For multiple choice: ['A', 'B', 'C', 'D']
    correct_answer = db.Column(db.String(10))  # Optional: for quiz mode
    is_active = db.Column(db.Boolean, default=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    closed_at = db.Column(db.DateTime, nullable=True)

    # Relationship to responses
    responses = db.relationship('Response', back_populates='question', cascade='all, delete-orphan')

    def to_dict(self):
        """Convert question to dictionary"""
        return {
            'id': self.id,
            'question_text': self.question_text,
            'question_type': self.question_type,
            'options': self.options,
            'correct_answer': self.correct_answer,
            'is_active': self.is_active,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'closed_at': self.closed_at.isoformat() if self.closed_at else None,
            'response_count': len(self.responses)
        }

class Response(db.Model):
    """Student responses to questions"""
    __tablename__ = 'responses'

    id = db.Column(db.Integer, primary_key=True)
    question_id = db.Column(db.Integer, db.ForeignKey('questions.id'), nullable=True)
    student_id = db.Column(db.String(50), nullable=False)
    response = db.Column(db.String(10), nullable=False)  # 'YES', 'NO', 'A', 'B', 'C', 'D'
    timestamp = db.Column(db.DateTime, default=datetime.utcnow)
    device_timestamp = db.Column(db.Integer)  # Timestamp from ESP32 (millis)

    # Relationship to question
    question = db.relationship('Question', back_populates='responses')

    def to_dict(self):
        """Convert response to dictionary"""
        return {
            'id': self.id,
            'question_id': self.question_id,
            'student_id': self.student_id,
            'response': self.response,
            'timestamp': self.timestamp.isoformat() if self.timestamp else None,
            'device_timestamp': self.device_timestamp
        }

class Student(db.Model):
    """Student information"""
    __tablename__ = 'students'

    id = db.Column(db.Integer, primary_key=True)
    student_id = db.Column(db.String(50), unique=True, nullable=False)
    name = db.Column(db.String(100))
    last_seen = db.Column(db.DateTime, default=datetime.utcnow)
    is_active = db.Column(db.Boolean, default=True)

    def to_dict(self):
        """Convert student to dictionary"""
        return {
            'id': self.id,
            'student_id': self.student_id,
            'name': self.name,
            'last_seen': self.last_seen.isoformat() if self.last_seen else None,
            'is_active': self.is_active
        }

def init_db(app):
    """Initialize database with app context"""
    db.init_app(app)
    with app.app_context():
        db.create_all()
        print("Database initialized successfully!")
