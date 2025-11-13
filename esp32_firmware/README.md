# ESP32 Firmware - Student Response Device

This directory contains the Arduino firmware for the ESP32-based student response devices.

## Hardware Requirements

- **ESP32 Development Board** (ESP32-WROOM-32 or similar)
- **6x Push Buttons** (normally open)
- **6x 10kΩ Resistors** (optional if using internal pull-ups)
- **Breadboard and Jumper Wires**
- **USB Cable** (for programming)

## Pin Configuration

Default GPIO pin assignments (can be modified in the main .ino file):

| Button | GPIO Pin | Description |
|--------|----------|-------------|
| YES    | GPIO 13  | Yes response |
| NO     | GPIO 12  | No response |
| A      | GPIO 14  | Option A |
| B      | GPIO 27  | Option B |
| C      | GPIO 26  | Option C |
| D      | GPIO 25  | Option D |
| LED    | GPIO 2   | Status LED (built-in) |

## Wiring Diagram

```
ESP32                    Button
GPIO Pin ----+
             |
            [ ]  Push Button
             |
GND ---------+

Note: Using internal pull-up resistors (INPUT_PULLUP)
Button press connects GPIO to GND (active LOW)
```

### Detailed Wiring for One Button

```
ESP32 GPIO Pin (e.g., GPIO 13)
    |
    |
   [ ] Push Button
    |
    |
ESP32 GND
```

## Software Requirements

1. **Arduino IDE** (version 1.8.13 or later)
   - Download from: https://www.arduino.cc/en/software

2. **ESP32 Board Support**
   - In Arduino IDE, go to File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools → Board → Boards Manager
   - Search for "ESP32" and install "ESP32 by Espressif Systems"

3. **Required Libraries**
   - **WiFi** (included with ESP32 board support)
   - **HTTPClient** (included with ESP32 board support)
   - **ArduinoJson** (Install via Library Manager)
     - Go to Sketch → Include Library → Manage Libraries
     - Search for "ArduinoJson" by Benoit Blanchon
     - Install version 6.x.x

## Setup Instructions

### 1. Configure Settings

Edit `student_device/config.h`:

```cpp
// WiFi credentials
#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PASSWORD "YourPassword"

// Server URL (IP address of your computer running the server)
#define SERVER_URL "http://192.168.1.100:5000"

// Unique student ID
#define STUDENT_ID "STUDENT_001"
```

### 2. Upload to ESP32

1. Connect ESP32 to computer via USB
2. Open `student_device/student_device.ino` in Arduino IDE
3. Select board: Tools → Board → ESP32 Arduino → ESP32 Dev Module
4. Select port: Tools → Port → (select your ESP32 port)
5. Click Upload button (→)

### 3. Monitor Serial Output

1. Open Serial Monitor: Tools → Serial Monitor
2. Set baud rate to **115200**
3. You should see connection status and button press events

## Testing

1. Power on the ESP32
2. Wait for WiFi connection (LED will turn on when connected)
3. Press any button
4. Check Serial Monitor for confirmation
5. Check server dashboard for received response

## LED Status Indicators

- **Solid ON**: Connected to WiFi
- **OFF**: Not connected to WiFi
- **3 Quick Blinks**: WiFi connection successful
- **Single Quick Blink**: Response sent successfully
- **Rapid Blinking (5 times)**: Error sending response

## Troubleshooting

### WiFi Not Connecting

- Check SSID and password in config.h
- Ensure ESP32 is within WiFi range
- Check that WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Reset ESP32 and try again

### Button Not Responding

- Check wiring connections
- Verify GPIO pin numbers in code match your wiring
- Check Serial Monitor for debounce issues
- Try increasing debounce delay in code

### Server Not Receiving Data

- Verify server is running
- Check SERVER_URL in config.h
- Ensure ESP32 and server are on same network
- Check firewall settings on server computer
- Verify server IP address (use ipconfig/ifconfig)

### Cannot Upload Code

- Check USB cable (must support data transfer)
- Try different USB port
- Press and hold BOOT button during upload
- Check correct board and port selected in Arduino IDE

## Multiple Devices

To deploy multiple student devices:

1. Configure first device with STUDENT_ID "STUDENT_001"
2. Upload and test
3. For second device, change config.h:
   ```cpp
   #define STUDENT_ID "STUDENT_002"
   ```
4. Upload to second device
5. Repeat for additional devices

**Pro Tip**: Keep a spreadsheet mapping Student IDs to physical device labels

## Customization

### Change Button Pins

In `student_device.ino`, modify these lines:

```cpp
#define BTN_YES   13  // Change to your desired pin
#define BTN_NO    12
// etc...
```

### Add More Buttons

1. Define new pin: `#define BTN_E 33`
2. Add to buttons array:
   ```cpp
   Button buttons[] = {
     // ... existing buttons
     {BTN_E, "E", HIGH, 0}
   };
   ```
3. Update `numButtons` variable

### Adjust Debounce Delay

In `student_device.ino`:

```cpp
const unsigned long debounceDelay = 50; // Increase if getting double-presses
```

## Power Options

- **USB Power**: Connect to USB power bank or wall adapter (5V)
- **Battery Power**: Use 3.7V LiPo battery with appropriate regulator
- **Typical Current Draw**: 80-160mA during WiFi transmission

## Next Steps

- Test with server running (see `server/` directory)
- Build protective case for classroom use
- Consider 3D printing button enclosure
- Label devices with student names or numbers

## Support

For issues or questions, please create an issue in the GitHub repository.
