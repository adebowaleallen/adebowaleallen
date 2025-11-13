# Hardware Setup Guide

Complete guide for building and wiring the ESP32 student response devices.

## Bill of Materials (per device)

### Required Components

| Component | Quantity | Estimated Cost | Notes |
|-----------|----------|----------------|-------|
| ESP32 Development Board | 1 | $5-10 | ESP32-WROOM-32, NodeMCU-32S, or similar |
| Push Buttons | 6 | $0.10 each | Momentary push buttons, normally open |
| Jumper Wires | 15-20 | $3 (pack) | Male-to-male for breadboard |
| Breadboard | 1 | $3-5 | 830 or 400 tie points |
| USB Cable | 1 | $2-5 | Micro-USB or USB-C (depending on ESP32 board) |
| Optional: 10kΩ Resistors | 6 | $0.05 each | Only if not using internal pull-ups |
| Optional: LEDs | 6 | $0.10 each | Visual feedback for each button |
| Optional: 220Ω Resistors | 6 | $0.05 each | For LED current limiting |

**Total Cost per Device:** $10-15 (basic) or $15-20 (with LEDs)

## ESP32 Pin Reference

### Default Pin Configuration

| Button | GPIO Pin | Physical Pin | Alternative Pin |
|--------|----------|--------------|-----------------|
| YES | GPIO 13 | D13 | GPIO 32 |
| NO | GPIO 12 | D12 | GPIO 33 |
| A | GPIO 14 | D14 | GPIO 15 |
| B | GPIO 27 | D27 | GPIO 4 |
| C | GPIO 26 | D26 | GPIO 16 |
| D | GPIO 25 | D25 | GPIO 17 |
| Status LED | GPIO 2 | D2 | Built-in LED |

### Pins to AVOID

Do NOT use these pins for buttons:
- **GPIO 0**: Boot mode selection
- **GPIO 1, 3**: UART (USB serial)
- **GPIO 6-11**: Connected to flash memory
- **GPIO 5**: Often used for SPI

## Wiring Diagrams

### Basic Wiring (Using Internal Pull-ups)

This is the simplest and recommended method.

```
ESP32                          Button
┌─────────────┐
│             │
│  GPIO 13 ●──┼───────┐
│             │       │
│             │      [ ]  Push Button
│             │       │
│             │       │
│      GND ●──┼───────┴───────────┐
│             │                   │
└─────────────┘                   ┴ (Common Ground)
```

#### Connection Steps:

1. Connect one terminal of each button to its designated GPIO pin
2. Connect the other terminal of all buttons to a common ground rail
3. The ESP32's internal pull-up resistors will handle the rest

**Repeat for all 6 buttons**

### Wiring with External Pull-down Resistors

If you prefer active-HIGH logic or want more robust debouncing:

```
                    +3.3V
                      │
ESP32                 │
┌─────────────┐       │
│             │      [ ]  Push Button
│  GPIO 13 ●──┼───────┤
│             │       │
│             │      ┌┴┐
│             │      │ │  10kΩ Resistor
│             │      │ │
│             │      └┬┘
│      GND ●──┼───────┴───────────┐
│             │                   │
└─────────────┘                   ┴ (Common Ground)
```

**Note:** If using external pull-downs, modify the code to use `INPUT` instead of `INPUT_PULLUP` and change logic to active-HIGH.

### Complete 6-Button Layout

```
                    ESP32 Development Board
                    ┌────────────────────┐
                    │                    │
                    │  [USB Port]        │
                    │                    │
    YES Button ─────┤ GPIO 13      3.3V ├──── Power Rail (+)
    NO Button ──────┤ GPIO 12      GND  ├──── Ground Rail (-)
    A Button ───────┤ GPIO 14      ...  │
    B Button ───────┤ GPIO 27           │
    C Button ───────┤ GPIO 26           │
    D Button ───────┤ GPIO 25           │
                    │                    │
                    └────────────────────┘
                             │
                             │ (All button other terminals)
                             ▼
                        Ground Rail
```

## Adding LED Indicators (Optional)

To add LED feedback for each button press:

```
ESP32                    Button & LED Circuit
┌─────────────┐
│  GPIO 13 ●──┼───────┬──────[ Button ]──── GND
│             │       │
│             │       │
│  GPIO XX ●──┼───────┴───[220Ω]───[LED>]─── GND
│             │           Resistor  (Anode to resistor,
└─────────────┘                      Cathode to GND)
```

LED Pin Mapping (if using separate pins):
- YES LED: GPIO 19
- NO LED: GPIO 18
- A LED: GPIO 5
- B LED: GPIO 17
- C LED: GPIO 16
- D LED: GPIO 4

## Physical Assembly

### Option 1: Breadboard Prototype

1. Place ESP32 across the center divide of breadboard
2. Place buttons in a row on one side
3. Use jumper wires to connect buttons to GPIO pins
4. Connect common ground rail to ESP32 GND

**Pros:** Quick to build, easy to modify
**Cons:** Not durable for classroom use

### Option 2: Soldered Version

1. Use a prototyping board (perfboard)
2. Solder ESP32 headers to board
3. Solder buttons in ergonomic layout
4. Solder all connections
5. Add hot glue for strain relief

**Pros:** Durable, compact
**Cons:** Harder to modify

### Option 3: PCB Design

For production, consider designing a custom PCB with:
- ESP32 module footprint
- 6 buttons in ergonomic layout
- Power management circuit
- USB connector
- Mounting holes

### Option 4: 3D Printed Enclosure

Design a case with:
- Button cutouts labeled YES, NO, A, B, C, D
- ESP32 mounting points
- LED light pipes (if using LEDs)
- USB port access
- Ventilation holes

## Button Layout Recommendations

### Linear Layout
```
┌───┬───┬───┬───┬───┬───┐
│YES│NO │ A │ B │ C │ D │
└───┴───┴───┴───┴───┴───┘
```

### Separated Layout
```
┌───┬───┐        ┌───┬───┬───┬───┐
│YES│NO │        │ A │ B │ C │ D │
└───┴───┘        └───┴───┴───┴───┘
```

### Ergonomic Layout
```
        ┌───┐
    ┌───┤ A ├───┐
    │ B │   │ C │
    └───┴───┴───┘
        │ D │
    ┌───┴───┴───┐
    │ YES │ NO  │
    └─────┴─────┘
```

## Power Options

### Option 1: USB Power (Recommended for Development)

- Connect ESP32 to USB power adapter or power bank
- Pros: Simple, always available
- Cons: Requires cable

### Option 2: Battery Power

**LiPo Battery (3.7V):**
- Use ESP32 boards with built-in LiPo charger
- Typical capacity: 1000-2000mAh
- Runtime: 8-12 hours (depending on WiFi usage)

**Connections:**
- Red wire → BAT+ or VIN
- Black wire → GND

**Important:** Use boards with battery protection circuitry!

### Option 3: AA Battery Pack

- 3x AA batteries = 4.5V (use voltage regulator)
- 4x AA batteries = 6V (use voltage regulator to 3.3V)
- Connect through appropriate regulator to VIN

## Testing Procedure

### 1. Visual Inspection

- Check all solder joints (if soldered)
- Verify no short circuits
- Confirm button movement

### 2. Continuity Test

Use multimeter to verify:
- Button press closes circuit
- No shorts between adjacent pins
- Ground connections solid

### 3. Power Test

1. Connect USB cable
2. Check built-in LED lights up
3. Measure 3.3V on 3.3V pin (relative to GND)

### 4. Button Test

Upload test sketch:

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(13, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(13) == LOW) {
    Serial.println("Button pressed!");
    delay(500);
  }
}
```

Press button and verify serial output.

### 5. WiFi Test

Upload full firmware with correct WiFi credentials and verify connection.

## Troubleshooting

### Buttons Not Responding

- Check wiring connections
- Verify correct GPIO pins in code
- Test button continuity with multimeter
- Try increasing debounce delay

### ESP32 Won't Power On

- Check USB cable (must support data + power)
- Try different USB port
- Check for short circuits
- Verify voltage regulator (if using battery)

### False Button Triggers

- Add 100nF capacitor across button terminals
- Increase debounce delay in code
- Check for loose connections

### WiFi Connection Issues

- Verify SSID and password
- Check 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Reduce distance to router
- Check router firewall settings

## Maintenance

### Regular Checks

- Test all buttons weekly
- Check battery level (if battery-powered)
- Verify WiFi connection strength
- Update firmware as needed

### Cleaning

- Use compressed air to clean button contacts
- Wipe case with slightly damp cloth
- Avoid liquid near circuits

## Safety Notes

1. Never connect power supplies > 3.6V directly to ESP32 GPIO pins
2. Use appropriate current-limiting resistors for LEDs
3. Ensure good ventilation if using enclosed cases
4. Supervise young students during use
5. Use proper power supplies with protection circuitry

## Next Steps

After building hardware:

1. Test each device individually
2. Configure unique Student IDs
3. Label devices clearly
4. Create device inventory spreadsheet
5. Establish charging/storage procedure

## Resources

- ESP32 Pinout Diagram: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
- ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
- Arduino ESP32 Documentation: https://docs.espressif.com/projects/arduino-esp32/

## Support

For hardware questions or issues, please create an issue in the GitHub repository with:
- Photos of your setup
- Description of the problem
- What you've already tried
