# ESP32 3-Phase Motor Protection System v3.2

## Overview

A comprehensive industrial motor protection and monitoring system using ESP32 microcontroller with real-time vibration analysis, current monitoring, temperature sensing, and ML-based anomaly detection. This system provides predictive maintenance capabilities to prevent motor failures and reduce downtime.

## Features

### Core Protection
- **3-Phase Current Monitoring** - Real-time current sensing with overcurrent and imbalance detection
- **Vibration Analysis** - 3-axis accelerometer monitoring with FFT analysis
- **Temperature Monitoring** - DS18B20 waterproof temperature sensor
- **Emergency Stop** - Hardware E-stop button with immediate relay shutdown
- **Critical Condition Detection** - Automatic motor shutdown on fault conditions

### Advanced Features
- **Machine Learning Anomaly Detection** - Statistical baseline with Z-score analysis
- **Health Score Calculation** - Real-time motor health assessment (0-100%)
- **Web Dashboard** - Responsive web interface with live charts
- **WiFi Access Point** - No router required, direct connection to ESP32
- **Secured Access** - HTTP Basic Authentication
- **Auto-Recovery** - Intelligent fault recovery with 5-second cooldown
- **Watchdog Timer** - System stability protection

### Web Dashboard Features
- Real-time sensor data visualization
- Interactive Chart.js graphs for trends
- Dark/Light theme toggle
- Mobile responsive design
- Live status indicators with color coding
- Motor control buttons (Start/Stop/Reset)

## Hardware Requirements

### Components

| Component | Specification | Quantity | Purpose |
|-----------|--------------|----------|---------|
| ESP32 DevKit | 38-pin or 30-pin | 1 | Main controller |
| MPU6050 | 3-axis accelerometer/gyro | 1 | Vibration monitoring |
| SCT-013 | 30A/100A current transformer | 3 | Current sensing (3-phase) |
| DS18B20 | Waterproof temperature sensor | 1 | Temperature monitoring |
| Relay Module | 5V/12V, 10A+ | 1 | Motor contactor control |
| Buzzer | Active 5V | 1 | Audio alerts |
| LED | 5mm | 1 | Status indicator |
| Push Button | Momentary NO | 1 | Emergency stop |
| Resistors | 10kΩ, 100Ω | 3-5 | Pull-up/current limiting |
| Capacitors | 100nF, 10µF | 2-3 | Noise filtering |
| Burden Resistors | 100Ω (for SCT-013) | 3 | Current sensor biasing |

### Optional Components
- Buck converter (12V/24V to 5V) for industrial power supply
- Enclosure (IP65 rated for industrial environments)
- Terminal blocks for easy wiring
- Heat sink for ESP32 (if in high-temperature environment)

## Pin Configuration

### ESP32 Pinout

```
GPIO 33 → Emergency Stop Button (INPUT_PULLUP)
GPIO 32 → Relay Module (OUTPUT)
GPIO 25 → Buzzer (OUTPUT)
GPIO 5  → Status LED (OUTPUT)

GPIO 19 → DS18B20 Temperature Sensor (1-Wire)

GPIO 34 → Phase A Current Sensor (ADC1_CH6)
GPIO 35 → Phase B Current Sensor (ADC1_CH7)
GPIO 36 → Phase C Current Sensor (ADC1_CH0)

GPIO 21 → I2C SDA (MPU6050)
GPIO 22 → I2C SCL (MPU6050)

GND     → Common Ground
3.3V    → Power for MPU6050, DS18B20
5V      → Power for Relay, Buzzer (if needed)
```

### Important Notes
- ADC pins (34, 35, 36) are input-only, no pull-up/pull-down
- Use voltage divider if motor voltage > 3.3V
- Optocoupler recommended for relay control in high-voltage applications

## Software Requirements

### Arduino IDE Setup

1. **Install Arduino IDE**
   - Download from [arduino.cc](https://www.arduino.cc/en/software)
   - Version 2.0+ recommended

2. **Add ESP32 Board Support**
   - Open Arduino IDE
   - Go to: File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to: Tools → Board → Board Manager
   - Search "ESP32" and install "esp32 by Espressif Systems" (v2.0.11+)

3. **Select Board**
   - Tools → Board → ESP32 Arduino → ESP32 Dev Module
   - Tools → Upload Speed → 115200
   - Tools → Flash Frequency → 80MHz
   - Tools → Partition Scheme → Default 4MB with spiffs

### Required Libraries

Install via Arduino Library Manager (Sketch → Include Library → Manage Libraries):

| Library | Version | Purpose |
|---------|---------|---------|
| Adafruit MPU6050 | ≥2.2.4 | Accelerometer driver |
| Adafruit Unified Sensor | ≥1.1.9 | Sensor abstraction |
| DallasTemperature | ≥3.9.0 | DS18B20 interface |
| OneWire | ≥2.3.7 | 1-Wire protocol |
| arduinoFFT | ≥1.6.1 | FFT calculations (optional) |
| WiFi | Built-in | WiFi connectivity |
| WebServer | Built-in | HTTP server |
| Wire | Built-in | I2C communication |
| Preferences | Built-in | NVS storage |

### Library Installation

```bash
# Option 1: Arduino IDE Library Manager
Sketch → Include Library → Manage Libraries
Search and install each library

# Option 2: PlatformIO
lib_deps =
    adafruit/Adafruit MPU6050@^2.2.4
    adafruit/Adafruit Unified Sensor@^1.1.9
    milesburton/DallasTemperature@^3.9.0
    paulstoffregen/OneWire@^2.3.7
    kosme/arduinoFFT@^1.6.1
```

## Installation & Setup

### 1. Clone or Download

```bash
git clone https://github.com/yourusername/ESP32_Motor_Protection.git
cd ESP32_Motor_Protection
```

### 2. Configuration

Open `ESP32_Motor_Protection.ino` and configure:

```cpp
// WiFi Settings
const char* ssid     = "YourSSID";         // Change to your WiFi name
const char* password = "YourPassword";     // Change to your WiFi password

// Web Authentication
const char* www_username = "admin";        // Dashboard login username
const char* www_password = "motor2024";    // Dashboard login password

// Motor Specifications (Adjust for your motor)
#define RATED_CURRENT           10.0       // Motor rated current (Amps)
#define SCT013_CALIBRATION      30.0       // Current sensor calibration
#define MOTOR_RPM               1750       // Motor RPM from nameplate
#define MOTOR_POLES             4          // Number of poles

// Thresholds (Adjust based on your application)
#define VIBRATION_WARNING_THRESHOLD     5.0   // m/s²
#define CRITICAL_VIBRATION_THRESHOLD    8.0   // m/s²
#define TEMP_WARNING                    60.0  // °C
#define TEMP_CRITICAL                   80.0  // °C
#define OVERCURRENT_THRESHOLD           15.0  // Amps
#define CURRENT_IMBALANCE_THRESHOLD     0.20  // 20%
```

### 3. Upload to ESP32

1. Connect ESP32 to computer via USB
2. Select correct COM port: Tools → Port → COM# (Windows) or /dev/ttyUSB# (Linux)
3. Click Upload button (→)
4. Wait for "Done uploading" message
5. Open Serial Monitor (115200 baud) to verify operation

### 4. Hardware Assembly

See `WIRING_DIAGRAM.md` for detailed connection instructions.

## Usage

### First Time Setup

1. **Power On**
   - Connect ESP32 to power (USB or external 5V)
   - Wait for startup sequence (buzzer beep)

2. **Connect to WiFi**
   - Look for WiFi network: `MotorProtect_ML`
   - Password: `motor123456`
   - Connect from computer or smartphone

3. **Access Dashboard**
   - Open browser
   - Navigate to: `http://192.168.4.1`
   - Login with:
     - Username: `admin`
     - Password: `motor2024`

### Operating the System

#### Starting the Motor
1. Verify all sensors show "Connected" status
2. Click "▶️ Start" button on dashboard
3. Wait for ML model training (200 samples, ~20 seconds)
4. Monitor health score and sensor readings

#### Emergency Stop
- Press physical E-stop button on hardware
- OR click "⏹️ Stop" button on dashboard
- Buzzer will sound 3 times
- Motor immediately de-energized

#### System Reset
- Click "↻ Reset" button to clear emergency stop
- System ready to restart after 5-second cooldown
- Buzzer double-beep confirms reset

### Dashboard Features

#### System Status Card
- **Health Score**: 0-100% overall motor condition
- **Motor Status**: Energized/De-energized indicator
- **Temperature**: Real-time °C reading
- **ML Status**: Training progress or Trained status

#### Vibration Card
- X, Y, Z axis acceleration (m/s²)
- Color-coded status badges:
  - 🟢 OK: < 5.0 m/s²
  - 🟡 WARNING: 5.0-8.0 m/s²
  - 🔴 CRITICAL: > 8.0 m/s²

#### Current Card
- Individual phase currents (A, B, C)
- Average current
- Phase imbalance percentage

#### ML Anomaly Detection
- Real-time anomaly score
- Baseline training progress
- Automatic fault detection

#### Charts
- **Vibration Trends**: Last 20 readings, 3-axis visualization
- **Current Trends**: Last 20 readings, 3-phase comparison
- **Anomaly Score**: Real-time with threshold line

## Calibration Guide

### Current Sensor Calibration

1. **No-Load Calibration**
   ```cpp
   // Run motor without load
   // Observe Serial Monitor output
   // Adjust SCT013_CALIBRATION until reading ≈ 0A
   ```

2. **Full-Load Calibration**
   ```cpp
   // Run motor at rated load
   // Compare ESP32 reading with clamp meter
   // Calculate: NEW_CAL = OLD_CAL × (ACTUAL / ESP32_READING)
   // Update SCT013_CALIBRATION value
   ```

### Vibration Threshold Calibration

1. Run motor under normal conditions for 10 minutes
2. Note peak vibration readings from Serial Monitor
3. Set thresholds:
   ```cpp
   WARNING = Normal_Peak × 1.5
   CRITICAL = Normal_Peak × 2.0
   ```

### Temperature Calibration

1. Verify DS18B20 reading with external thermometer
2. If offset needed, apply in `readTemperatureC()`:
   ```cpp
   float tempC = tempSensor.getTempCByIndex(0);
   tempC = tempC + OFFSET; // Add offset if needed
   ```

## Machine Learning Model

### How It Works

The system uses **Statistical Anomaly Detection** based on Z-score analysis:

1. **Training Phase** (First 200 samples, ~20 seconds)
   - Collects baseline data for all sensors
   - Calculates mean and standard deviation
   - Establishes normal operating parameters

2. **Detection Phase**
   - Calculates Z-score for each sensor reading
   - Z-score = |Current Value - Mean| / Std Dev
   - Anomaly Score = Average of all Z-scores
   - Triggers alarm if Anomaly Score > 3.0

### ML Parameters

```cpp
#define ML_ANOMALY_THRESHOLD    3.0   // Z-score threshold (3σ rule)
#define ML_BASELINE_SAMPLES     200   // Training samples
#define MAX_TRENDS              100   // Historical data storage
```

### Improving ML Accuracy

- Run training under stable, normal operating conditions
- Increase `ML_BASELINE_SAMPLES` for better accuracy (trade-off: longer training)
- Adjust `ML_ANOMALY_THRESHOLD`:
  - Lower (2.5): More sensitive, more false alarms
  - Higher (3.5): Less sensitive, fewer false alarms

## Troubleshooting

### Common Issues

#### 1. MPU6050 Not Detected
```
❌ MPU6050 not found at 0x68!
```
**Solutions:**
- Check I2C wiring (SDA=21, SCL=22)
- Verify MPU6050 power (3.3V)
- Try alternate I2C address (0x69) - solder AD0 pin high
- Check with I2C scanner: `scanI2C()` in Serial Monitor

#### 2. DS18B20 Not Found
```
⚠️ DS18B20 not found
```
**Solutions:**
- Verify 1-Wire connection (GPIO 19)
- Check 4.7kΩ pull-up resistor between Data and VCC
- Ensure correct power (3.3V or 5V, check sensor spec)
- Try multiple DS18B20 sensors (one may be faulty)

#### 3. Current Readings Always 0
**Solutions:**
- Verify SCT-013 is clipped around **one** wire only
- Check burden resistor (100Ω across sensor output)
- Confirm ADC pins: 34, 35, 36
- Increase `SCT013_CALIBRATION` value
- Check for loose connections

#### 4. WiFi Connection Failed
**Solutions:**
- Verify SSID and password in code
- Check WiFi router 2.4GHz (ESP32 doesn't support 5GHz)
- Try moving ESP32 closer to router
- Use AP mode if router connection fails

#### 5. Web Dashboard Not Loading
**Solutions:**
- Verify IP address from Serial Monitor
- Try `http://192.168.4.1` (AP mode default)
- Clear browser cache
- Try different browser (Chrome, Firefox)
- Check authentication credentials

#### 6. Motor Won't Start
**Solutions:**
- Check emergency stop not active
- Verify no critical conditions (check Serial Monitor)
- Ensure relay wiring correct (NO/NC terminals)
- Test relay independently (should click when activated)
- Check relay power supply (5V or 12V)

#### 7. False Alarms / Frequent Shutdowns
**Solutions:**
- Re-run ML training under stable conditions
- Increase threshold values:
  ```cpp
  VIBRATION_WARNING_THRESHOLD 7.0  // was 5.0
  ML_ANOMALY_THRESHOLD 4.0         // was 3.0
  ```
- Check for external vibration sources
- Verify sensor mounting (MPU6050 should be firmly attached)

### Serial Monitor Debug

Enable verbose output by monitoring Serial at 115200 baud:
```
=== MPU6050 Initialization ===
✅ Device at 0x68
Accelerometer range: +-8G
...
📊 Training: 150/200 samples
✅ ML Model trained!
Baseline X: mean=1.23 std=0.45
```

## Safety Warnings

### IMPORTANT - READ BEFORE USE

1. **High Voltage Hazard**
   - This system controls motors that may operate at dangerous voltages
   - Only qualified electricians should install this system
   - Always disconnect power before wiring
   - Use appropriate safety relays and contactors

2. **Not a Replacement for Industrial Protection**
   - This is a DIY educational/monitoring system
   - DO NOT use as sole protection for critical infrastructure
   - Always use certified motor protection relays for industrial applications
   - Consult a qualified engineer for safety-critical systems

3. **Testing Requirements**
   - Test emergency stop functionality before deployment
   - Verify all sensors working correctly
   - Run extended testing under supervision
   - Have backup shutdown mechanisms

4. **Limitations**
   - ML model requires stable training conditions
   - False positives may occur during training
   - Current sensors must be calibrated for accuracy
   - WiFi range limited (AP mode ~50m, depends on environment)

5. **Maintenance**
   - Regularly check sensor connections
   - Re-calibrate current sensors if motor load changes
   - Monitor for sensor drift over time
   - Keep firmware updated

## Technical Specifications

### System Performance
- **Sampling Rate**: 1000 Hz (vibration), 10 Hz (current/temp)
- **FFT Resolution**: 256 samples
- **Response Time**: < 100ms (critical shutdown)
- **ML Training Time**: ~20 seconds (200 samples)
- **Web Update Rate**: 1 Hz (1 second refresh)

### Power Consumption
- ESP32: ~500mA @ 5V (WiFi active)
- MPU6050: ~3.5mA @ 3.3V
- DS18B20: ~1mA @ 3.3V
- Relay: ~70mA @ 5V (energized)
- **Total**: ~600mA @ 5V (3W)

### Memory Usage
- Flash: ~1.2MB (program)
- RAM: ~120KB (runtime)
- Free Heap: ~180KB (available)

## API Reference

### Web Endpoints

#### GET /
Returns main HTML dashboard (requires authentication)

#### GET /data
Returns JSON with all sensor data
```json
{
  "peakMagX": 2.34,
  "peakMagY": 1.89,
  "peakMagZ": 3.12,
  "statusX": 0,
  "statusY": 0,
  "statusZ": 0,
  "currentPhaseA": 8.45,
  "currentPhaseB": 8.67,
  "currentPhaseC": 8.23,
  "avgCurrent": 8.45,
  "currentImbalance": 0.025,
  "temperature": 45.6,
  "healthScore": 95.0,
  "anomalyScore": 1.23,
  "isTrained": true,
  "baselineCount": 200,
  "relayA": true,
  "motorRunning": true,
  "sensorOK": true,
  "tempSensorConnected": true,
  "emergencyStop": false,
  "dataCount": 12456
}
```

#### POST /start
Starts motor (if no emergency stop or critical condition)

#### POST /stop
Stops motor immediately

#### POST /reset
Clears emergency stop and critical shutdown flags

## Future Enhancements

### Planned Features
- [ ] MQTT integration for IoT platforms
- [ ] Data logging to SD card
- [ ] Email/SMS alerts on critical conditions
- [ ] Mobile app (React Native)
- [ ] FFT frequency analysis visualization
- [ ] Historical data export (CSV)
- [ ] OTA (Over-The-Air) firmware updates
- [ ] Multi-motor support
- [ ] Cloud dashboard integration
- [ ] Predictive maintenance algorithms

### Community Contributions
Pull requests welcome! Areas needing help:
- PCB design for production
- 3D-printable enclosure
- Additional ML algorithms
- Documentation translations
- Unit tests

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Credits

**Author**: [Your Name]
**Version**: 3.2
**Last Updated**: 2025

### Libraries Used
- Adafruit Industries (MPU6050 library)
- Miles Burton (DallasTemperature)
- Paul Stoffregen (OneWire)
- Espressif Systems (ESP32 Arduino Core)

## Support

### Getting Help
- GitHub Issues: [Report bugs or ask questions](https://github.com/yourusername/ESP32_Motor_Protection/issues)
- Discussions: [Community forum](https://github.com/yourusername/ESP32_Motor_Protection/discussions)
- Email: your.email@example.com

### Useful Resources
- [ESP32 Pinout Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [MPU6050 Datasheet](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/)
- [SCT-013 Current Sensor Guide](https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/how-to-build-an-arduino-energy-monitor)
- [DS18B20 Temperature Sensor Guide](https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/)

## Disclaimer

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND. USE AT YOUR OWN RISK. THE AUTHORS ARE NOT RESPONSIBLE FOR ANY DAMAGE, INJURY, OR LOSS RESULTING FROM THE USE OF THIS SYSTEM.

For industrial applications, always consult certified electrical engineers and use UL/CE approved equipment.

---

**⚡ Happy Monitoring! Keep your motors healthy! ⚙️**
