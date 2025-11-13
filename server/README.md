# Server Documentation

Flask-based web server for the Student Response System.

## Installation

### Quick Start

```bash
pip install -r requirements.txt
python app.py
```

### Using Virtual Environment (Recommended)

```bash
# Create virtual environment
python -m venv venv

# Activate virtual environment
# On Windows:
venv\Scripts\activate
# On Mac/Linux:
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt

# Run server
python app.py
```

## Configuration

### Environment Variables

Create a `.env` file (optional):

```env
FLASK_ENV=development
FLASK_DEBUG=True
SECRET_KEY=your-secret-key-here
DATABASE_URL=sqlite:///student_responses.db
PORT=5000
HOST=0.0.0.0
```

### Production Settings

For production deployment, update `app.py`:

```python
# Change secret key
app.config['SECRET_KEY'] = 'your-production-secret-key'

# Use production database (PostgreSQL recommended)
app.config['SQLALCHEMY_DATABASE_URI'] = 'postgresql://user:pass@localhost/dbname'

# Disable debug mode
app.config['DEBUG'] = False
```

## API Endpoints

See `docs/api_documentation.md` for complete API reference.

### Quick Reference

- `GET /api/health` - Health check
- `POST /api/questions` - Create question
- `GET /api/questions` - Get all questions
- `GET /api/questions/active` - Get active question
- `POST /api/questions/:id/close` - Close question
- `POST /api/responses` - Submit response
- `GET /api/responses` - Get responses
- `GET /api/responses/:id/summary` - Get response summary
- `GET /api/students` - Get all students
- `PUT /api/students/:id` - Update student

## Database

### Schema

**questions**
- id (Integer, Primary Key)
- question_text (Text)
- question_type (String)
- options (JSON)
- correct_answer (String)
- is_active (Boolean)
- created_at (DateTime)
- closed_at (DateTime)

**responses**
- id (Integer, Primary Key)
- question_id (Integer, Foreign Key)
- student_id (String)
- response (String)
- timestamp (DateTime)
- device_timestamp (Integer)

**students**
- id (Integer, Primary Key)
- student_id (String, Unique)
- name (String)
- last_seen (DateTime)
- is_active (Boolean)

### Database Commands

Reset database:
```python
from app import app, db
with app.app_context():
    db.drop_all()
    db.create_all()
```

Query data:
```python
from app import app
from database import Question, Response, Student

with app.app_context():
    questions = Question.query.all()
    responses = Response.query.filter_by(question_id=1).all()
    students = Student.query.all()
```

## WebSocket Events

Real-time communication using Socket.IO.

### Client Events
- `connect` - Client connected
- `disconnect` - Client disconnected
- `request_active_question` - Request current active question

### Server Events
- `connection_status` - Connection established
- `new_question` - New question created
- `new_response` - New response submitted
- `question_closed` - Question closed

## Development

### Run in Debug Mode

```bash
python app.py
```

Debug mode includes:
- Auto-reload on code changes
- Detailed error pages
- SQLite database

### Run Tests

```bash
# Install test dependencies
pip install pytest pytest-flask

# Run tests
pytest tests/
```

## Deployment

### Deployment Checklist

- [ ] Change SECRET_KEY to strong random value
- [ ] Set DEBUG = False
- [ ] Use production database (PostgreSQL)
- [ ] Configure CORS for specific origins
- [ ] Add authentication/authorization
- [ ] Enable HTTPS (SSL/TLS)
- [ ] Set up rate limiting
- [ ] Configure logging
- [ ] Set up monitoring
- [ ] Configure backup strategy

### Deployment Options

#### Option 1: Heroku

```bash
# Install Heroku CLI
# Create Procfile
echo "web: python app.py" > Procfile

# Deploy
heroku create your-app-name
git push heroku main
```

#### Option 2: AWS EC2

```bash
# SSH to EC2 instance
ssh -i your-key.pem ec2-user@your-instance-ip

# Install dependencies
sudo yum install python3 python3-pip
pip3 install -r requirements.txt

# Run with gunicorn
pip3 install gunicorn
gunicorn -w 4 -b 0.0.0.0:5000 app:app
```

#### Option 3: Docker

```dockerfile
FROM python:3.9-slim

WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt

COPY . .

CMD ["python", "app.py"]
```

```bash
docker build -t student-response-system .
docker run -p 5000:5000 student-response-system
```

### Production WSGI Server

Use Gunicorn instead of Flask development server:

```bash
pip install gunicorn

# Run with 4 worker processes
gunicorn -w 4 -b 0.0.0.0:5000 app:app

# With WebSocket support
gunicorn -k geventwebsocket.gunicorn.workers.GeventWebSocketWorker -w 1 -b 0.0.0.0:5000 app:app
```

## Monitoring

### Logging

Add logging to `app.py`:

```python
import logging

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('app.log'),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger(__name__)
```

### Health Monitoring

Use `/api/health` endpoint for monitoring:

```bash
curl http://localhost:5000/api/health
```

## Security

### Recommendations

1. **Authentication**: Add login system for teachers
2. **HTTPS**: Use SSL/TLS certificates
3. **Rate Limiting**: Prevent abuse
4. **Input Validation**: Validate all inputs
5. **CORS**: Configure allowed origins
6. **SQL Injection**: Use parameterized queries (already implemented)
7. **XSS Protection**: Sanitize outputs

### Add Authentication Example

```python
from flask_login import LoginManager, login_required

login_manager = LoginManager()
login_manager.init_app(app)

@app.route('/admin')
@login_required
def admin():
    return render_template('admin.html')
```

## Troubleshooting

### Port Already in Use

```bash
# Kill process using port 5000
# Mac/Linux:
lsof -ti:5000 | xargs kill -9

# Windows:
netstat -ano | findstr :5000
taskkill /PID <PID> /F
```

### Database Locked Error

SQLite may lock with concurrent writes. Solutions:
- Use PostgreSQL for production
- Add retry logic
- Reduce write frequency

### CORS Errors

Update CORS configuration in `app.py`:

```python
CORS(app, resources={
    r"/api/*": {
        "origins": ["http://localhost:3000", "https://yourschool.edu"],
        "methods": ["GET", "POST", "PUT", "DELETE"],
        "allow_headers": ["Content-Type"]
    }
})
```

## Performance

### Optimization Tips

1. Use connection pooling for database
2. Add caching for frequently accessed data
3. Optimize database queries
4. Use CDN for static files
5. Enable gzip compression
6. Use Redis for session storage

### Load Testing

```bash
# Install Apache Bench
sudo apt-get install apache2-utils

# Test with 1000 requests, 10 concurrent
ab -n 1000 -c 10 http://localhost:5000/api/health
```

## Support

For server issues, check:
- Server terminal output
- `app.log` file
- Database file exists and has proper permissions
- Port 5000 is not blocked by firewall
- Python version is 3.8+

## License

MIT License
