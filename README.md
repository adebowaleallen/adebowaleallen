# Student Response Button System

A hardware-software interactive classroom system where students use ESP32-based devices with physical buttons to respond to teacher questions in real-time.

## Features

- **Hardware**: ESP32 devices with 6 buttons (YES, NO, A, B, C, D)
- **Real-time Responses**: Students' answers sent instantly to the web server
- **Teacher Dashboard**: Upload questions and view live student responses
- **Multi-student Support**: Track individual student responses
- **WiFi Connectivity**: ESP32 devices connect via WiFi to the server

## System Architecture

```
[ESP32 Device (Student)] --WiFi--> [Web Server] <--Browser-- [Teacher Dashboard]
     |                                  |
  Buttons                          Database
 (YES/NO/A/B/C/D)            (Questions & Responses)
```

## Project Structure

```
├── esp32_firmware/          # ESP32 Arduino code
│   ├── student_device/      # Main firmware for student devices
│   └── README.md            # Hardware setup instructions
├── server/                  # Web server backend
│   ├── app.py              # Flask application
│   ├── database.py         # Database models
│   ├── requirements.txt    # Python dependencies
│   └── static/             # Frontend files
│       ├── index.html      # Teacher dashboard
│       ├── styles.css      # Styling
│       └── app.js          # Frontend JavaScript
└── docs/                   # Documentation
    ├── hardware_setup.md   # ESP32 wiring guide
    └── api_documentation.md # API endpoints
```

## Quick Start

### 1. ESP32 Setup

1. Install Arduino IDE with ESP32 board support
2. Wire buttons to ESP32 GPIO pins (see `docs/hardware_setup.md`)
3. Configure WiFi credentials in `esp32_firmware/student_device/config.h`
4. Upload firmware to ESP32

### 2. Server Setup

```bash
cd server
pip install -r requirements.txt
python app.py
```

Server runs on `http://localhost:5000`

### 3. Teacher Dashboard

Open browser and navigate to `http://localhost:5000`

## Hardware Requirements

- ESP32 Development Board (per student)
- 6x Push Buttons
- 6x 10kΩ Resistors (pull-down)
- Breadboard and jumper wires
- USB cable for programming

## Software Requirements

- Python 3.8+
- Arduino IDE 1.8+ or PlatformIO
- Modern web browser (Chrome, Firefox, Safari)

## Usage

1. **Teacher**: Create a question via the web dashboard
2. **Students**: Press buttons on their ESP32 devices to respond
3. **Teacher**: View real-time responses on the dashboard
4. **Teacher**: Close question and view statistics

## API Endpoints

- `POST /api/questions` - Create new question
- `GET /api/questions` - Get all questions
- `POST /api/responses` - Submit student response
- `GET /api/responses/:question_id` - Get responses for a question

## License

MIT License

## Author

@adebowaleallen
