/**
 * Student Response System - Frontend JavaScript
 *
 * Handles:
 * - WebSocket connections for real-time updates
 * - Question creation and management
 * - Live response display
 * - Student tracking
 */

// Global state
let socket;
let currentQuestion = null;
let responses = [];
let students = new Map();

// Initialize when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    initializeSocketConnection();
    setupEventListeners();
    loadQuestionHistory();
    loadStudents();
});

// ==================== WebSocket Connection ====================

function initializeSocketConnection() {
    socket = io();

    socket.on('connect', () => {
        console.log('Connected to server');
        updateConnectionStatus(true);
        socket.emit('request_active_question');
    });

    socket.on('disconnect', () => {
        console.log('Disconnected from server');
        updateConnectionStatus(false);
    });

    socket.on('connection_status', (data) => {
        console.log('Connection status:', data);
    });

    socket.on('new_question', (question) => {
        console.log('New question received:', question);
        handleNewQuestion(question);
    });

    socket.on('new_response', (response) => {
        console.log('New response received:', response);
        handleNewResponse(response);
    });

    socket.on('question_closed', (data) => {
        console.log('Question closed:', data);
        if (currentQuestion && currentQuestion.id === data.question_id) {
            currentQuestion.is_active = false;
            updateActiveQuestionDisplay();
        }
    });
}

function updateConnectionStatus(connected) {
    const statusIndicator = document.getElementById('connectionStatus');
    const statusText = document.getElementById('connectionText');

    if (connected) {
        statusIndicator.textContent = '🟢';
        statusIndicator.className = 'status-indicator connected';
        statusText.textContent = 'Connected';
    } else {
        statusIndicator.textContent = '🔴';
        statusIndicator.className = 'status-indicator disconnected';
        statusText.textContent = 'Disconnected';
    }
}

// ==================== Event Listeners ====================

function setupEventListeners() {
    // Question form submission
    const questionForm = document.getElementById('questionForm');
    questionForm.addEventListener('submit', handleQuestionSubmit);

    // Question type change
    const questionType = document.getElementById('questionType');
    questionType.addEventListener('change', handleQuestionTypeChange);

    // Close question button
    const closeBtn = document.getElementById('closeQuestionBtn');
    closeBtn.addEventListener('click', handleCloseQuestion);
}

function handleQuestionTypeChange(e) {
    const optionsGroup = document.getElementById('optionsGroup');
    if (e.target.value === 'multiple_choice') {
        optionsGroup.style.display = 'block';
    } else {
        optionsGroup.style.display = 'none';
    }
}

// ==================== Question Management ====================

async function handleQuestionSubmit(e) {
    e.preventDefault();

    const questionText = document.getElementById('questionText').value;
    const questionType = document.getElementById('questionType').value;
    const correctAnswer = document.getElementById('correctAnswer').value;

    let options = null;
    if (questionType === 'multiple_choice') {
        options = {
            A: document.getElementById('optionA').value || 'Option A',
            B: document.getElementById('optionB').value || 'Option B',
            C: document.getElementById('optionC').value || 'Option C',
            D: document.getElementById('optionD').value || 'Option D'
        };
    }

    const questionData = {
        question_text: questionText,
        question_type: questionType,
        options: options,
        correct_answer: correctAnswer || null
    };

    try {
        const response = await fetch('/api/questions', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(questionData)
        });

        if (response.ok) {
            const data = await response.json();
            console.log('Question created:', data);

            // Reset form
            e.target.reset();
            document.getElementById('optionsGroup').style.display = 'none';

            // Clear responses
            responses = [];

            // Reload history
            loadQuestionHistory();
        } else {
            const error = await response.json();
            alert('Error creating question: ' + error.error);
        }
    } catch (error) {
        console.error('Error creating question:', error);
        alert('Failed to create question. Check console for details.');
    }
}

async function handleCloseQuestion() {
    if (!currentQuestion) return;

    if (!confirm('Are you sure you want to close this question?')) {
        return;
    }

    try {
        const response = await fetch(`/api/questions/${currentQuestion.id}/close`, {
            method: 'POST'
        });

        if (response.ok) {
            const data = await response.json();
            console.log('Question closed:', data);
            currentQuestion = null;
            updateActiveQuestionDisplay();
            loadQuestionHistory();
        }
    } catch (error) {
        console.error('Error closing question:', error);
    }
}

function handleNewQuestion(question) {
    currentQuestion = question;
    responses = [];
    updateActiveQuestionDisplay();
    updateResponseStats();
    clearResponseFeed();
}

function handleNewResponse(response) {
    responses.push(response);
    addResponseToFeed(response);
    updateResponseStats();

    // Update student last seen
    updateStudentLastSeen(response.student_id);
}

// ==================== UI Updates ====================

function updateActiveQuestionDisplay() {
    const display = document.getElementById('activeQuestionDisplay');
    const closeBtn = document.getElementById('closeQuestionBtn');
    const responseStats = document.getElementById('responseStats');

    if (!currentQuestion || !currentQuestion.is_active) {
        display.innerHTML = '<p class="no-question">No active question. Create one above!</p>';
        closeBtn.style.display = 'none';
        responseStats.style.display = 'none';
        return;
    }

    closeBtn.style.display = 'block';
    responseStats.style.display = 'block';

    let optionsHTML = '';
    if (currentQuestion.options) {
        optionsHTML = '<div class="question-options">';
        for (const [key, value] of Object.entries(currentQuestion.options)) {
            optionsHTML += `<div class="option"><strong>${key}:</strong> ${value}</div>`;
        }
        optionsHTML += '</div>';
    }

    display.innerHTML = `
        <div class="question-text">${currentQuestion.question_text}</div>
        <span class="question-type">${formatQuestionType(currentQuestion.question_type)}</span>
        ${optionsHTML}
    `;
}

function updateResponseStats() {
    const totalSpan = document.getElementById('totalResponses');
    const chartDiv = document.getElementById('responseChart');

    totalSpan.textContent = responses.length;

    // Count responses
    const counts = {};
    responses.forEach(r => {
        counts[r.response] = (counts[r.response] || 0) + 1;
    });

    // Calculate percentages and create chart
    const total = responses.length;
    let chartHTML = '';

    for (const [answer, count] of Object.entries(counts)) {
        const percentage = total > 0 ? (count / total) * 100 : 0;
        chartHTML += `
            <div class="chart-bar">
                <div class="chart-label">${answer}</div>
                <div class="chart-bar-container">
                    <div class="chart-bar-fill" style="width: ${percentage}%">
                        ${count} (${percentage.toFixed(1)}%)
                    </div>
                </div>
            </div>
        `;
    }

    chartDiv.innerHTML = chartHTML || '<p style="text-align: center; color: #6b7280;">No responses yet</p>';
}

function addResponseToFeed(response) {
    const feed = document.getElementById('responseFeed');

    // Remove "no responses" message if present
    const noResponses = feed.querySelector('.no-responses');
    if (noResponses) {
        noResponses.remove();
    }

    const timestamp = new Date(response.timestamp).toLocaleTimeString();

    const item = document.createElement('div');
    item.className = 'response-item';
    item.innerHTML = `
        <div>
            <span class="student">${response.student_id}</span>
        </div>
        <div style="display: flex; gap: 15px; align-items: center;">
            <span class="answer">${response.response}</span>
            <span class="timestamp">${timestamp}</span>
        </div>
    `;

    // Add to top of feed
    feed.insertBefore(item, feed.firstChild);

    // Limit feed to 50 items
    while (feed.children.length > 50) {
        feed.removeChild(feed.lastChild);
    }
}

function clearResponseFeed() {
    const feed = document.getElementById('responseFeed');
    feed.innerHTML = '<p class="no-responses">Waiting for student responses...</p>';
}

// ==================== Question History ====================

async function loadQuestionHistory() {
    try {
        const response = await fetch('/api/questions');
        const data = await response.json();

        const historyDiv = document.getElementById('questionHistory');

        if (data.questions.length === 0) {
            historyDiv.innerHTML = '<p class="no-history">No questions yet.</p>';
            return;
        }

        historyDiv.innerHTML = '';
        data.questions.forEach(question => {
            const item = document.createElement('div');
            item.className = 'history-item';
            if (question.is_active) {
                item.classList.add('active');
            }

            const createdDate = new Date(question.created_at).toLocaleString();
            const status = question.is_active ? '🟢 Active' : '⚫ Closed';

            item.innerHTML = `
                <div class="question-text">${question.question_text}</div>
                <div class="meta">
                    <span>${status} • ${createdDate}</span>
                    <span class="response-count">${question.response_count} responses</span>
                </div>
            `;

            item.addEventListener('click', () => viewQuestionDetails(question.id));

            historyDiv.appendChild(item);
        });
    } catch (error) {
        console.error('Error loading question history:', error);
    }
}

async function viewQuestionDetails(questionId) {
    try {
        const response = await fetch(`/api/responses/${questionId}/summary`);
        const data = await response.json();

        console.log('Question details:', data);

        // Update display with selected question's responses
        alert(`Question: ${data.question.question_text}\n\nTotal Responses: ${data.total_responses}\n\n${JSON.stringify(data.percentages, null, 2)}`);
    } catch (error) {
        console.error('Error loading question details:', error);
    }
}

// ==================== Student Management ====================

async function loadStudents() {
    try {
        const response = await fetch('/api/students');
        const data = await response.json();

        const studentListDiv = document.getElementById('studentList');

        if (data.students.length === 0) {
            studentListDiv.innerHTML = '<p class="no-students">No students connected yet.</p>';
            return;
        }

        studentListDiv.innerHTML = '';
        data.students.forEach(student => {
            students.set(student.student_id, student);
            addStudentToList(student);
        });
    } catch (error) {
        console.error('Error loading students:', error);
    }
}

function addStudentToList(student) {
    const listDiv = document.getElementById('studentList');

    // Remove "no students" message if present
    const noStudents = listDiv.querySelector('.no-students');
    if (noStudents) {
        noStudents.remove();
    }

    // Check if student already in list
    const existingItem = document.getElementById(`student-${student.student_id}`);
    if (existingItem) {
        return; // Already in list
    }

    const item = document.createElement('div');
    item.className = 'student-item';
    item.id = `student-${student.student_id}`;

    const lastSeen = student.last_seen ? new Date(student.last_seen).toLocaleTimeString() : 'Never';

    item.innerHTML = `
        <span class="student-icon">👤</span>
        <div class="student-info">
            <div class="student-id">${student.name || student.student_id}</div>
            <div class="last-seen">Last seen: ${lastSeen}</div>
        </div>
    `;

    listDiv.appendChild(item);
}

function updateStudentLastSeen(studentId) {
    const item = document.getElementById(`student-${studentId}`);
    if (item) {
        const lastSeenSpan = item.querySelector('.last-seen');
        if (lastSeenSpan) {
            lastSeenSpan.textContent = `Last seen: ${new Date().toLocaleTimeString()}`;
        }
    } else {
        // Student not in list yet, reload
        loadStudents();
    }
}

// ==================== Utility Functions ====================

function formatQuestionType(type) {
    const types = {
        'yes_no': 'Yes/No',
        'multiple_choice': 'Multiple Choice',
        'true_false': 'True/False'
    };
    return types[type] || type;
}

// Auto-refresh students every 30 seconds
setInterval(loadStudents, 30000);

// Auto-refresh history every 60 seconds
setInterval(loadQuestionHistory, 60000);
