"""
Flask Web Server for Student Response System

This server handles:
- Teacher interface for creating questions
- API endpoints for ESP32 devices to submit responses
- Real-time dashboard displaying student responses
- Database management for questions and responses
"""

from flask import Flask, request, jsonify, render_template, send_from_directory
from flask_cors import CORS
from flask_socketio import SocketIO, emit
from datetime import datetime
import os
import json

from database import db, init_db, Question, Response, Student

# Initialize Flask app
app = Flask(__name__, static_folder='static', template_folder='static')
app.config['SECRET_KEY'] = 'your-secret-key-change-this-in-production'
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///student_responses.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

# Initialize extensions
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*")

# Initialize database
init_db(app)

# ==================== Routes ====================

@app.route('/')
def index():
    """Serve the teacher dashboard"""
    return send_from_directory('static', 'index.html')

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.utcnow().isoformat()
    })

# ==================== Question Management ====================

@app.route('/api/questions', methods=['POST'])
def create_question():
    """Create a new question"""
    try:
        data = request.get_json()

        # Validate required fields
        if not data.get('question_text'):
            return jsonify({'error': 'Question text is required'}), 400

        question_type = data.get('question_type', 'multiple_choice')

        # Close any currently active questions
        active_questions = Question.query.filter_by(is_active=True).all()
        for q in active_questions:
            q.is_active = False
            q.closed_at = datetime.utcnow()

        # Create new question
        question = Question(
            question_text=data['question_text'],
            question_type=question_type,
            options=data.get('options'),
            correct_answer=data.get('correct_answer'),
            is_active=True
        )

        db.session.add(question)
        db.session.commit()

        # Broadcast new question to all connected clients
        socketio.emit('new_question', question.to_dict())

        return jsonify({
            'message': 'Question created successfully',
            'question': question.to_dict()
        }), 201

    except Exception as e:
        db.session.rollback()
        return jsonify({'error': str(e)}), 500

@app.route('/api/questions', methods=['GET'])
def get_questions():
    """Get all questions"""
    try:
        questions = Question.query.order_by(Question.created_at.desc()).all()
        return jsonify({
            'questions': [q.to_dict() for q in questions]
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/questions/<int:question_id>', methods=['GET'])
def get_question(question_id):
    """Get a specific question"""
    try:
        question = Question.query.get(question_id)
        if not question:
            return jsonify({'error': 'Question not found'}), 404

        return jsonify(question.to_dict())
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/questions/<int:question_id>/close', methods=['POST'])
def close_question(question_id):
    """Close a question (stop accepting responses)"""
    try:
        question = Question.query.get(question_id)
        if not question:
            return jsonify({'error': 'Question not found'}), 404

        question.is_active = False
        question.closed_at = datetime.utcnow()
        db.session.commit()

        # Broadcast question closed event
        socketio.emit('question_closed', {'question_id': question_id})

        return jsonify({
            'message': 'Question closed successfully',
            'question': question.to_dict()
        })
    except Exception as e:
        db.session.rollback()
        return jsonify({'error': str(e)}), 500

@app.route('/api/questions/active', methods=['GET'])
def get_active_question():
    """Get the currently active question"""
    try:
        question = Question.query.filter_by(is_active=True).first()
        if not question:
            return jsonify({'message': 'No active question'}), 404

        return jsonify(question.to_dict())
    except Exception as e:
        return jsonify({'error': str(e)}), 500

# ==================== Response Management ====================

@app.route('/api/responses', methods=['POST'])
def submit_response():
    """Submit a student response (called by ESP32 devices)"""
    try:
        data = request.get_json()

        # Validate required fields
        if not data.get('student_id') or not data.get('response'):
            return jsonify({'error': 'student_id and response are required'}), 400

        student_id = data['student_id']
        response_text = data['response'].upper()

        # Update or create student record
        student = Student.query.filter_by(student_id=student_id).first()
        if not student:
            student = Student(student_id=student_id, name=student_id)
            db.session.add(student)
        student.last_seen = datetime.utcnow()

        # Get active question (if any)
        active_question = Question.query.filter_by(is_active=True).first()

        # Create response
        response = Response(
            question_id=active_question.id if active_question else None,
            student_id=student_id,
            response=response_text,
            device_timestamp=data.get('timestamp')
        )

        db.session.add(response)
        db.session.commit()

        # Broadcast new response to connected clients
        socketio.emit('new_response', response.to_dict())

        return jsonify({
            'message': 'Response recorded successfully',
            'response': response.to_dict()
        }), 201

    except Exception as e:
        db.session.rollback()
        return jsonify({'error': str(e)}), 500

@app.route('/api/responses', methods=['GET'])
def get_responses():
    """Get all responses (optionally filtered by question_id)"""
    try:
        question_id = request.args.get('question_id', type=int)

        if question_id:
            responses = Response.query.filter_by(question_id=question_id).all()
        else:
            responses = Response.query.order_by(Response.timestamp.desc()).limit(100).all()

        return jsonify({
            'responses': [r.to_dict() for r in responses]
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/responses/<int:question_id>/summary', methods=['GET'])
def get_response_summary(question_id):
    """Get summary statistics for a question's responses"""
    try:
        question = Question.query.get(question_id)
        if not question:
            return jsonify({'error': 'Question not found'}), 404

        responses = Response.query.filter_by(question_id=question_id).all()

        # Count responses by type
        summary = {}
        for response in responses:
            resp_text = response.response
            summary[resp_text] = summary.get(resp_text, 0) + 1

        # Calculate percentages
        total = len(responses)
        percentages = {}
        if total > 0:
            for key, count in summary.items():
                percentages[key] = round((count / total) * 100, 1)

        return jsonify({
            'question': question.to_dict(),
            'total_responses': total,
            'summary': summary,
            'percentages': percentages,
            'responses': [r.to_dict() for r in responses]
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500

# ==================== Student Management ====================

@app.route('/api/students', methods=['GET'])
def get_students():
    """Get all registered students"""
    try:
        students = Student.query.all()
        return jsonify({
            'students': [s.to_dict() for s in students]
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/students/<student_id>', methods=['PUT'])
def update_student(student_id):
    """Update student information"""
    try:
        student = Student.query.filter_by(student_id=student_id).first()
        if not student:
            return jsonify({'error': 'Student not found'}), 404

        data = request.get_json()
        if data.get('name'):
            student.name = data['name']

        db.session.commit()

        return jsonify({
            'message': 'Student updated successfully',
            'student': student.to_dict()
        })
    except Exception as e:
        db.session.rollback()
        return jsonify({'error': str(e)}), 500

# ==================== WebSocket Events ====================

@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    print('Client connected')
    emit('connection_status', {'status': 'connected'})

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    print('Client disconnected')

@socketio.on('request_active_question')
def handle_request_active_question():
    """Send active question to newly connected client"""
    question = Question.query.filter_by(is_active=True).first()
    if question:
        emit('new_question', question.to_dict())

# ==================== Error Handlers ====================

@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors"""
    return jsonify({'error': 'Resource not found'}), 404

@app.errorhandler(500)
def internal_error(error):
    """Handle 500 errors"""
    db.session.rollback()
    return jsonify({'error': 'Internal server error'}), 500

# ==================== Main ====================

if __name__ == '__main__':
    print("\n" + "="*50)
    print("  Student Response System - Server Starting")
    print("="*50)
    print("\nServer will be available at:")
    print("  - Local:   http://localhost:5000")
    print("  - Network: http://<your-ip>:5000")
    print("\nTo find your IP address:")
    print("  - Windows: ipconfig")
    print("  - Mac/Linux: ifconfig")
    print("\n" + "="*50 + "\n")

    # Run with SocketIO
    socketio.run(app, host='0.0.0.0', port=5000, debug=True, allow_unsafe_werkzeug=True)
