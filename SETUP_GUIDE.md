# Quick Setup Guide

Step-by-step guide to get your Student Response System up and running quickly.

## Prerequisites

- Python 3.8 or higher
- Arduino IDE (for ESP32 programming)
- WiFi network (2.4GHz)
- ESP32 development boards
- Basic electronic components (buttons, wires, breadboard)

## Step 1: Server Setup (10 minutes)

### 1.1 Install Python Dependencies

```bash
cd server
pip install -r requirements.txt
```

Or if using virtual environment (recommended):

```bash
cd server
python -m venv venv

# On Windows:
venv\Scripts\activate

# On Mac/Linux:
source venv/bin/activate

pip install -r requirements.txt
```

### 1.2 Start the Server

```bash
python app.py
```

You should see:

```
==================================================
  Student Response System - Server Starting
==================================================

Server will be available at:
  - Local:   http://localhost:5000
  - Network: http://<your-ip>:5000
...
```

### 1.3 Find Your Server IP Address

**Windows:**
```bash
ipconfig
```
Look for "IPv4 Address" under your active network adapter.

**Mac/Linux:**
```bash
ifconfig
# or
ip addr show
```
Look for "inet" address under your active network interface (usually starts with 192.168.x.x or 10.x.x.x).

**Example:** `192.168.1.100`

### 1.4 Test the Server

Open your web browser and go to:
```
http://localhost:5000
```

You should see the Teacher Dashboard!

---

## Step 2: Hardware Setup (20 minutes per device)

### 2.1 Wire the ESP32

Follow the wiring diagram in `docs/hardware_setup.md`. Basic connections:

```
Button YES → GPIO 13
Button NO  → GPIO 12
Button A   → GPIO 14
Button B   → GPIO 27
Button C   → GPIO 26
Button D   → GPIO 25

All buttons other terminal → GND
```

### 2.2 Install Arduino IDE

1. Download from: https://www.arduino.cc/en/software
2. Install for your operating system

### 2.3 Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "ESP32"
6. Install "ESP32 by Espressif Systems"

### 2.4 Install ArduinoJson Library

1. Go to **Sketch → Include Library → Manage Libraries**
2. Search for "ArduinoJson"
3. Install "ArduinoJson by Benoit Blanchon" (version 6.x.x)

---

## Step 3: Configure ESP32 Firmware (5 minutes per device)

### 3.1 Open the Firmware

1. Navigate to `esp32_firmware/student_device/`
2. Open `student_device.ino` in Arduino IDE

### 3.2 Configure Settings

Open `config.h` and update:

```cpp
// Your WiFi credentials
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourWiFiPassword"

// Your server IP address (from Step 1.3)
#define SERVER_URL "http://192.168.1.100:5000"

// Unique student ID for this device
#define STUDENT_ID "STUDENT_001"
```

**Important:** Change `STUDENT_ID` for each device:
- Device 1: "STUDENT_001"
- Device 2: "STUDENT_002"
- Device 3: "STUDENT_003"
- etc.

### 3.3 Upload to ESP32

1. Connect ESP32 to computer via USB
2. Select board: **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
3. Select port: **Tools → Port → (your ESP32 port)**
4. Click **Upload** button (→)
5. Wait for "Done uploading"

### 3.4 Test the Device

1. Open Serial Monitor: **Tools → Serial Monitor**
2. Set baud rate to **115200**
3. Press Reset button on ESP32

You should see:

```
=== Student Response System ===
Student ID: STUDENT_001
Connecting to WiFi: YourWiFiName
.....
WiFi Connected!
IP Address: 192.168.1.101
System ready. Press any button to respond.
```

4. Press any button
5. Check Serial Monitor for confirmation
6. Check Teacher Dashboard - you should see the response appear!

---

## Step 4: Create Your First Question (2 minutes)

### 4.1 Open Teacher Dashboard

In your web browser, go to:
```
http://localhost:5000
```

Or from another computer on the same network:
```
http://192.168.1.100:5000
```

### 4.2 Create a Question

1. In the "Create Question" section:
   - Enter question: "Is this system working?"
   - Select type: "Yes/No"
   - Click "Create Question"

2. The question appears in "Active Question" section

### 4.3 Test Student Response

1. Press the "YES" button on your ESP32 device
2. Watch the response appear in real-time on the dashboard!
3. Check the "Live Response Feed" and response statistics

---

## Step 5: Deploy Multiple Devices (5 minutes per device)

For each additional student device:

1. Wire hardware (same as Step 2.1)
2. Change `STUDENT_ID` in `config.h`:
   ```cpp
   #define STUDENT_ID "STUDENT_002"  // Increment for each device
   ```
3. Upload firmware
4. Test with Serial Monitor
5. Label device with student ID

**Pro Tip:** Keep a spreadsheet mapping Student IDs to physical devices and actual student names.

---

## Common Issues & Solutions

### Issue: ESP32 Won't Connect to WiFi

**Solutions:**
- Verify SSID and password are correct
- Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check if network has MAC address filtering
- Move closer to WiFi router
- Try restarting router

### Issue: Server Not Receiving Responses

**Solutions:**
- Verify server is running (check terminal)
- Check SERVER_URL in config.h matches server IP
- Ensure ESP32 and server are on same network
- Check firewall settings on server computer
- Verify port 5000 is not blocked

### Issue: Cannot Upload to ESP32

**Solutions:**
- Check USB cable (must support data, not just power)
- Try different USB port
- Press and hold BOOT button during upload
- Verify correct board and port selected
- Install USB drivers for your ESP32 board

### Issue: Buttons Not Responding

**Solutions:**
- Check wiring connections
- Verify button is normally-open type
- Test button with multimeter
- Check GPIO pin numbers in code match wiring
- Try increasing debounce delay

### Issue: Dashboard Not Loading

**Solutions:**
- Verify server is running
- Check browser console for errors (F12)
- Try different browser
- Clear browser cache
- Check server terminal for error messages

---

## Production Deployment Checklist

Before deploying in a classroom:

- [ ] Test all devices individually
- [ ] Label each device with Student ID
- [ ] Create device inventory spreadsheet
- [ ] Test with multiple devices simultaneously
- [ ] Verify WiFi coverage in classroom
- [ ] Create backup power solution (power banks)
- [ ] Prepare setup/troubleshooting guide for teacher
- [ ] Test question types (Yes/No, Multiple Choice, True/False)
- [ ] Verify dashboard works on teacher's device
- [ ] Create sample questions for first use
- [ ] Plan device distribution/collection procedure

---

## Tips for Classroom Use

1. **Pre-configure All Devices:** Upload firmware and test before class
2. **Label Clearly:** Use stickers or permanent marker with student names
3. **Charge Overnight:** If using battery power
4. **Have Spares:** Keep 2-3 backup devices ready
5. **Quick Start Card:** Create laminated card with WiFi/server info
6. **Student Instructions:** Simple 1-page guide for students
7. **Practice Round:** Do a test question to verify all devices work

---

## Next Steps

- Read `docs/hardware_setup.md` for detailed wiring diagrams
- Read `docs/api_documentation.md` for API reference
- Customize frontend styling in `server/static/styles.css`
- Add authentication for production use
- Implement student roster management
- Export response data to CSV

---

## Getting Help

- Check GitHub Issues: [Repository Issues]
- Review documentation in `docs/` folder
- Check Serial Monitor output for ESP32 debugging
- Check browser console (F12) for frontend errors
- Check server terminal for backend errors

---

## Updating the System

### Update Server

```bash
cd server
git pull
pip install -r requirements.txt --upgrade
python app.py
```

### Update ESP32 Firmware

1. Download latest firmware
2. Update config.h with your settings
3. Upload to ESP32
4. Test functionality

---

## Backup & Data Export

### Backup Database

The database is stored in `server/student_responses.db`. To backup:

```bash
cp server/student_responses.db server/backup_$(date +%Y%m%d).db
```

### Export Response Data

Use Python script to export to CSV:

```python
import sqlite3
import csv

conn = sqlite3.connect('student_responses.db')
cursor = conn.cursor()
cursor.execute('SELECT * FROM responses')

with open('responses_export.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['ID', 'Question ID', 'Student ID', 'Response', 'Timestamp'])
    writer.writerows(cursor.fetchall())

conn.close()
```

---

Congratulations! Your Student Response System is now ready to use! 🎉
