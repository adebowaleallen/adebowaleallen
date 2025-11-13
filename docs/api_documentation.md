# API Documentation

Complete reference for the Student Response System REST API and WebSocket events.

## Base URL

```
http://localhost:5000/api
```

For production, replace with your server's URL.

## Authentication

Currently no authentication required. For production deployment, consider adding:
- API keys
- JWT tokens
- OAuth 2.0

## HTTP Endpoints

### Health Check

Check if the server is running.

**Endpoint:** `GET /api/health`

**Response:**
```json
{
  "status": "healthy",
  "timestamp": "2025-11-13T10:30:00.000Z"
}
```

---

## Questions

### Create Question

Create a new question and automatically close any currently active questions.

**Endpoint:** `POST /api/questions`

**Request Body:**
```json
{
  "question_text": "What is 2 + 2?",
  "question_type": "multiple_choice",
  "options": {
    "A": "3",
    "B": "4",
    "C": "5",
    "D": "6"
  },
  "correct_answer": "B"
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| question_text | string | Yes | The question to ask |
| question_type | string | No | Type: "yes_no", "multiple_choice", "true_false" (default: "multiple_choice") |
| options | object | No | Answer options for multiple choice |
| correct_answer | string | No | Correct answer for grading (optional) |

**Response:** `201 Created`
```json
{
  "message": "Question created successfully",
  "question": {
    "id": 1,
    "question_text": "What is 2 + 2?",
    "question_type": "multiple_choice",
    "options": {
      "A": "3",
      "B": "4",
      "C": "5",
      "D": "6"
    },
    "correct_answer": "B",
    "is_active": true,
    "created_at": "2025-11-13T10:30:00.000Z",
    "closed_at": null,
    "response_count": 0
  }
}
```

**Errors:**
- `400 Bad Request`: Missing required fields
- `500 Internal Server Error`: Database error

---

### Get All Questions

Retrieve all questions (most recent first).

**Endpoint:** `GET /api/questions`

**Response:** `200 OK`
```json
{
  "questions": [
    {
      "id": 2,
      "question_text": "Is Python a programming language?",
      "question_type": "yes_no",
      "options": null,
      "correct_answer": "YES",
      "is_active": true,
      "created_at": "2025-11-13T10:35:00.000Z",
      "closed_at": null,
      "response_count": 5
    },
    {
      "id": 1,
      "question_text": "What is 2 + 2?",
      "question_type": "multiple_choice",
      "options": {
        "A": "3",
        "B": "4",
        "C": "5",
        "D": "6"
      },
      "correct_answer": "B",
      "is_active": false,
      "created_at": "2025-11-13T10:30:00.000Z",
      "closed_at": "2025-11-13T10:34:00.000Z",
      "response_count": 12
    }
  ]
}
```

---

### Get Single Question

Retrieve a specific question by ID.

**Endpoint:** `GET /api/questions/:id`

**Parameters:**
- `id` (path parameter): Question ID

**Response:** `200 OK`
```json
{
  "id": 1,
  "question_text": "What is 2 + 2?",
  "question_type": "multiple_choice",
  "options": {
    "A": "3",
    "B": "4",
    "C": "5",
    "D": "6"
  },
  "correct_answer": "B",
  "is_active": false,
  "created_at": "2025-11-13T10:30:00.000Z",
  "closed_at": "2025-11-13T10:34:00.000Z",
  "response_count": 12
}
```

**Errors:**
- `404 Not Found`: Question does not exist

---

### Get Active Question

Retrieve the currently active question.

**Endpoint:** `GET /api/questions/active`

**Response:** `200 OK`
```json
{
  "id": 2,
  "question_text": "Is Python a programming language?",
  "question_type": "yes_no",
  "options": null,
  "correct_answer": "YES",
  "is_active": true,
  "created_at": "2025-11-13T10:35:00.000Z",
  "closed_at": null,
  "response_count": 5
}
```

**Errors:**
- `404 Not Found`: No active question

---

### Close Question

Close a question to stop accepting new responses.

**Endpoint:** `POST /api/questions/:id/close`

**Parameters:**
- `id` (path parameter): Question ID

**Response:** `200 OK`
```json
{
  "message": "Question closed successfully",
  "question": {
    "id": 2,
    "question_text": "Is Python a programming language?",
    "question_type": "yes_no",
    "options": null,
    "correct_answer": "YES",
    "is_active": false,
    "created_at": "2025-11-13T10:35:00.000Z",
    "closed_at": "2025-11-13T10:40:00.000Z",
    "response_count": 5
  }
}
```

**Errors:**
- `404 Not Found`: Question does not exist

---

## Responses

### Submit Response

Submit a student response. Called by ESP32 devices.

**Endpoint:** `POST /api/responses`

**Request Body:**
```json
{
  "student_id": "STUDENT_001",
  "response": "B",
  "timestamp": 123456789
}
```

**Parameters:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| student_id | string | Yes | Unique student identifier |
| response | string | Yes | Answer: "YES", "NO", "A", "B", "C", "D" |
| timestamp | integer | No | ESP32 millis() timestamp |

**Response:** `201 Created`
```json
{
  "message": "Response recorded successfully",
  "response": {
    "id": 15,
    "question_id": 2,
    "student_id": "STUDENT_001",
    "response": "B",
    "timestamp": "2025-11-13T10:36:00.000Z",
    "device_timestamp": 123456789
  }
}
```

**Errors:**
- `400 Bad Request`: Missing required fields
- `500 Internal Server Error`: Database error

**Notes:**
- Automatically creates student record if doesn't exist
- Associates response with currently active question (if any)
- Broadcasts response via WebSocket to connected clients

---

### Get Responses

Retrieve responses, optionally filtered by question.

**Endpoint:** `GET /api/responses`

**Query Parameters:**
- `question_id` (optional): Filter by question ID

**Examples:**
- All responses: `GET /api/responses`
- Question responses: `GET /api/responses?question_id=2`

**Response:** `200 OK`
```json
{
  "responses": [
    {
      "id": 15,
      "question_id": 2,
      "student_id": "STUDENT_001",
      "response": "YES",
      "timestamp": "2025-11-13T10:36:00.000Z",
      "device_timestamp": 123456789
    },
    {
      "id": 14,
      "question_id": 2,
      "student_id": "STUDENT_002",
      "response": "NO",
      "timestamp": "2025-11-13T10:35:30.000Z",
      "device_timestamp": 123450000
    }
  ]
}
```

**Notes:**
- Returns last 100 responses if no question_id specified
- Ordered by most recent first

---

### Get Response Summary

Get statistics and summary for a question's responses.

**Endpoint:** `GET /api/responses/:question_id/summary`

**Parameters:**
- `question_id` (path parameter): Question ID

**Response:** `200 OK`
```json
{
  "question": {
    "id": 2,
    "question_text": "Is Python a programming language?",
    "question_type": "yes_no",
    "options": null,
    "correct_answer": "YES",
    "is_active": false,
    "created_at": "2025-11-13T10:35:00.000Z",
    "closed_at": "2025-11-13T10:40:00.000Z",
    "response_count": 5
  },
  "total_responses": 5,
  "summary": {
    "YES": 4,
    "NO": 1
  },
  "percentages": {
    "YES": 80.0,
    "NO": 20.0
  },
  "responses": [
    {
      "id": 15,
      "question_id": 2,
      "student_id": "STUDENT_001",
      "response": "YES",
      "timestamp": "2025-11-13T10:36:00.000Z",
      "device_timestamp": 123456789
    }
    // ... more responses
  ]
}
```

**Errors:**
- `404 Not Found`: Question does not exist

---

## Students

### Get All Students

Retrieve all registered students.

**Endpoint:** `GET /api/students`

**Response:** `200 OK`
```json
{
  "students": [
    {
      "id": 1,
      "student_id": "STUDENT_001",
      "name": "John Doe",
      "last_seen": "2025-11-13T10:36:00.000Z",
      "is_active": true
    },
    {
      "id": 2,
      "student_id": "STUDENT_002",
      "name": "STUDENT_002",
      "last_seen": "2025-11-13T10:35:30.000Z",
      "is_active": true
    }
  ]
}
```

---

### Update Student

Update student information (e.g., set real name).

**Endpoint:** `PUT /api/students/:student_id`

**Parameters:**
- `student_id` (path parameter): Student ID

**Request Body:**
```json
{
  "name": "John Doe"
}
```

**Response:** `200 OK`
```json
{
  "message": "Student updated successfully",
  "student": {
    "id": 1,
    "student_id": "STUDENT_001",
    "name": "John Doe",
    "last_seen": "2025-11-13T10:36:00.000Z",
    "is_active": true
  }
}
```

**Errors:**
- `404 Not Found`: Student does not exist

---

## WebSocket Events

Connect to WebSocket for real-time updates:

```javascript
const socket = io('http://localhost:5000');
```

### Client → Server Events

#### connect

Automatically triggered when client connects.

```javascript
socket.on('connect', () => {
  console.log('Connected to server');
});
```

#### request_active_question

Request the currently active question.

```javascript
socket.emit('request_active_question');
```

#### disconnect

Automatically triggered when client disconnects.

```javascript
socket.on('disconnect', () => {
  console.log('Disconnected from server');
});
```

---

### Server → Client Events

#### connection_status

Sent when client connects successfully.

**Payload:**
```json
{
  "status": "connected"
}
```

#### new_question

Broadcast when a new question is created.

**Payload:**
```json
{
  "id": 2,
  "question_text": "Is Python a programming language?",
  "question_type": "yes_no",
  "options": null,
  "correct_answer": "YES",
  "is_active": true,
  "created_at": "2025-11-13T10:35:00.000Z",
  "closed_at": null,
  "response_count": 0
}
```

#### new_response

Broadcast when a student submits a response.

**Payload:**
```json
{
  "id": 15,
  "question_id": 2,
  "student_id": "STUDENT_001",
  "response": "YES",
  "timestamp": "2025-11-13T10:36:00.000Z",
  "device_timestamp": 123456789
}
```

#### question_closed

Broadcast when a question is closed.

**Payload:**
```json
{
  "question_id": 2
}
```

---

## Error Responses

All error responses follow this format:

```json
{
  "error": "Error message describing what went wrong"
}
```

### Common HTTP Status Codes

- `200 OK`: Request successful
- `201 Created`: Resource created successfully
- `400 Bad Request`: Invalid request data
- `404 Not Found`: Resource does not exist
- `500 Internal Server Error`: Server error

---

## Rate Limiting

Currently no rate limiting. For production, consider implementing:
- Per-IP rate limits
- Per-student rate limits
- API key quotas

---

## CORS

CORS is enabled for all origins in development. For production, configure specific allowed origins in `app.py`:

```python
CORS(app, origins=['https://yourschool.edu'])
```

---

## Examples

### Python Example (ESP32-like)

```python
import requests
import json

SERVER_URL = "http://192.168.1.100:5000"

def send_response(student_id, response):
    url = f"{SERVER_URL}/api/responses"
    data = {
        "student_id": student_id,
        "response": response,
        "timestamp": int(time.time() * 1000)
    }

    response = requests.post(url, json=data)
    return response.json()

# Usage
result = send_response("STUDENT_001", "A")
print(result)
```

### JavaScript Example (Frontend)

```javascript
// Create question
async function createQuestion(questionText, type) {
  const response = await fetch('/api/questions', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      question_text: questionText,
      question_type: type
    })
  });

  return await response.json();
}

// Get responses
async function getResponses(questionId) {
  const response = await fetch(`/api/responses?question_id=${questionId}`);
  return await response.json();
}
```

---

## Security Considerations

For production deployment:

1. **Add Authentication**: Implement API keys or JWT tokens
2. **HTTPS Only**: Use SSL/TLS certificates
3. **Input Validation**: Validate all input data
4. **Rate Limiting**: Prevent abuse
5. **CORS Configuration**: Restrict allowed origins
6. **SQL Injection Protection**: Use parameterized queries (already implemented with SQLAlchemy)
7. **XSS Protection**: Sanitize user input

---

## Support

For API questions or issues, please create an issue in the GitHub repository.
