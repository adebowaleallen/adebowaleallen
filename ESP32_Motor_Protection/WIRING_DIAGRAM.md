# ESP32 Motor Protection System - Wiring Diagram

## Complete Wiring Schematic

### Components Connection Table

| ESP32 Pin | Component | Pin/Terminal | Wire Color | Notes |
|-----------|-----------|--------------|------------|-------|
| GPIO 33 | E-Stop Button | Terminal 1 | Red | Button to GND when pressed |
| GND | E-Stop Button | Terminal 2 | Black | Common ground |
| GPIO 32 | Relay Module | IN | Orange | Control signal |
| 5V | Relay Module | VCC | Red | Power supply |
| GND | Relay Module | GND | Black | Common ground |
| GPIO 25 | Buzzer | Positive | Yellow | Through 100Ω resistor |
| GND | Buzzer | Negative | Black | Common ground |
| GPIO 5 | Status LED | Anode | Green | Through 220Ω resistor |
| GND | Status LED | Cathode | Black | Common ground |
| GPIO 21 | MPU6050 | SDA | Blue | I2C data line |
| GPIO 22 | MPU6050 | SCL | Yellow | I2C clock line |
| 3.3V | MPU6050 | VCC | Red | 3.3V power |
| GND | MPU6050 | GND | Black | Common ground |
| GPIO 19 | DS18B20 | Data | Purple | With 4.7kΩ pull-up |
| 3.3V | DS18B20 | VCC | Red | 3.3V power |
| GND | DS18B20 | GND | Black | Common ground |
| GPIO 34 | SCT-013 Phase A | Signal | White | Via voltage divider |
| GPIO 35 | SCT-013 Phase B | Signal | White | Via voltage divider |
| GPIO 36 | SCT-013 Phase C | Signal | White | Via voltage divider |
| GND | All SCT-013 | GND | Black | Common ground |

---

## Detailed Component Wiring

### 1. Emergency Stop Button (GPIO 33)

```
ESP32 GPIO 33 ──────┐
                    │
                  ┌─┴─┐
                  │ E │  E-Stop Button
                  │   │  (Normally Open)
                  └─┬─┘
                    │
ESP32 GND ──────────┘
```

**Connection Steps:**
1. Connect one terminal of button to GPIO 33
2. Connect other terminal to GND
3. No external pull-up needed (using INPUT_PULLUP)
4. When pressed, GPIO 33 reads LOW

**Safety Note:** In production, use Normally Closed (NC) button for fail-safe operation.

---

### 2. Relay Module (GPIO 32)

```
ESP32 5V ─────────────── VCC
ESP32 GPIO 32 ────┬───── IN
                  │
              [Optocoupler]
                  │
ESP32 GND ─────── GND


Relay Contacts:
COM ────┐
        │
NO ─────┤  Motor Contactor Coil
        │  (or control circuit)
NC ─────┘
```

**Connection Steps:**
1. Connect relay VCC to ESP32 5V (or external 5V)
2. Connect relay GND to ESP32 GND
3. Connect relay IN to GPIO 32
4. Wire relay COM to motor contactor control circuit
5. Wire relay NO (Normally Open) to contactor coil positive
6. Ground contactor coil negative

**Relay Logic:**
- GPIO 32 HIGH → Relay OFF → Motor De-energized
- GPIO 32 LOW → Relay ON → Motor Energized

**Important:** For 3-phase motors, use appropriate contactor rated for motor current. This relay should control the contactor coil, NOT the motor directly!

---

### 3. Buzzer (GPIO 25)

```
ESP32 GPIO 25 ──── [100Ω] ──── Buzzer (+)
                                  │
ESP32 GND ────────────────────── Buzzer (-)
```

**Connection Steps:**
1. Connect 100Ω resistor to GPIO 25
2. Connect resistor to buzzer positive (+) terminal
3. Connect buzzer negative (-) to GND

**Notes:**
- Use active buzzer (has internal oscillator)
- Voltage: 5V buzzer works with 3.3V GPIO (slightly quieter)
- Alternative: Use transistor driver for louder 5V buzzer

---

### 4. Status LED (GPIO 5)

```
ESP32 GPIO 5 ──── [220Ω] ──── LED Anode (+)
                                 │
ESP32 GND ──────────────────── LED Cathode (-)
```

**Connection Steps:**
1. Connect 220Ω resistor to GPIO 5
2. Connect resistor to LED anode (long leg, +)
3. Connect LED cathode (short leg, -) to GND

**LED Blink Patterns:**
- Normal: Slow blink (700ms)
- Emergency Stop: Medium blink (300ms)
- Critical Shutdown: Fast blink (150ms)

---

### 5. MPU6050 Accelerometer (I2C - GPIO 21/22)

```
ESP32 3.3V ────────── VCC
ESP32 GPIO 21 (SDA) ── SDA
ESP32 GPIO 22 (SCL) ── SCL
ESP32 GND ───────────── GND
     GND ──────────[ADO]    (Optional: for 0x69 address)
```

**Connection Steps:**
1. Connect MPU6050 VCC to ESP32 3.3V (NOT 5V!)
2. Connect SDA to GPIO 21
3. Connect SCL to GPIO 22
4. Connect GND to GND
5. Optional: Ground AD0 pin for 0x68 address (default)
           Connect AD0 to 3.3V for 0x69 address (if conflict)

**I2C Pull-ups:**
- Most MPU6050 modules have built-in 4.7kΩ pull-ups
- If not, add 4.7kΩ resistors: SDA to 3.3V, SCL to 3.3V

**Mounting:**
- Mount MPU6050 firmly to motor frame or bearing housing
- Use vibration-dampening material for very high vibration
- Align axes with motor orientation for meaningful readings

**I2C Address Check:**
```cpp
// In Serial Monitor after power-on:
Scanning I2C...
✅ Device at 0x68  // MPU6050 found
```

---

### 6. DS18B20 Temperature Sensor (1-Wire - GPIO 19)

```
ESP32 3.3V ──────────┬──── VCC (Red)
                     │
                  [4.7kΩ]  Pull-up resistor
                     │
ESP32 GPIO 19 ───────┼──── Data (Yellow)

ESP32 GND ───────────────── GND (Black)
```

**Connection Steps:**
1. Connect DS18B20 VCC (red) to ESP32 3.3V or 5V
2. Connect Data (yellow) to GPIO 19
3. Add 4.7kΩ pull-up resistor between Data and VCC
4. Connect GND (black) to ESP32 GND

**Waterproof Sensor Wiring:**
| Wire Color | Function |
|------------|----------|
| Red | VCC (3.3V or 5V) |
| Yellow | Data |
| Black | GND |

**Sensor Placement:**
- Mount on motor body near bearing
- Use thermal paste for better contact
- Avoid direct sunlight or external heat sources

**Troubleshooting:**
- If not detected, check 4.7kΩ pull-up resistor
- Try different GPIO pin (code change needed)
- Test with I2C scanner (different protocol, but verifies wiring)

---

### 7. SCT-013 Current Sensors (3× ADC - GPIO 34/35/36)

#### Circuit Diagram (per phase)

```
Motor Phase Wire ─┐
                  │
              ┌───┴───┐
              │ SCT   │  Current Transformer
              │ -013  │  (30A or 100A)
              └───┬───┘
                  │
                  │ AC Output
                  ├────── [100Ω Burden]
                  │            │
             [100nF Cap]      GND
                  │
                  ├────── [10kΩ] ──┬── 3.3V
                  │                 │
                  ├────── [10kΩ] ───┘
                  │
                  └────── ESP32 ADC Pin (34/35/36)
```

#### Detailed SCT-013 Wiring (Each Phase Identical)

**Phase A → GPIO 34**
**Phase B → GPIO 35**
**Phase C → GPIO 36**

**Component Values:**
- Burden Resistor: 100Ω (across SCT-013 output)
- Voltage Divider: 2× 10kΩ (creates 1.65V bias)
- Filter Capacitor: 100nF (parallel with burden resistor)

**Step-by-Step Wiring:**

1. **Burden Resistor (100Ω)**
   ```
   SCT-013 Output Terminal 1 ─┬─ [100Ω] ─┬─ GND
                              │           │
                         [100nF Cap]     GND
   ```

2. **Voltage Divider (creates 1.65V center point)**
   ```
   ESP32 3.3V ── [10kΩ] ─┬─ [10kΩ] ── GND
                         │
                   (1.65V bias)
                         │
                    To ADC Pin
   ```

3. **Complete Connection**
   ```
   SCT-013 Pin 1 ── [100Ω] ─┬─ GND
                            │
                       [100nF Cap]
                            │
   SCT-013 Pin 2 ──────────┼───── Voltage Divider ──── ADC Pin
   ```

**PCB Layout Recommendation:**

```
┌──────────────────────────────────────────┐
│  Phase A Circuit (GPIO 34)               │
├──────────────────────────────────────────┤
│                                          │
│  SCT-013 A                               │
│    │  │                                  │
│    │  └─── [100Ω] ─┬─ GND               │
│    │               │                     │
│    │          [100nF]                    │
│    │               │                     │
│    └───────────────┼─── [10kΩ] ─┬─ 3.3V │
│                    │             │       │
│                    │    [10kΩ] ──┘       │
│                    │      │              │
│                    └──────┴──── GPIO 34  │
│                                          │
└──────────────────────────────────────────┘

Repeat for Phase B (GPIO 35) and Phase C (GPIO 36)
```

**Physical Installation:**

1. **Clip SCT-013 around motor wire**
   - Open clip by squeezing sides
   - Position around ONE wire only (not both supply and return)
   - Ensure wire centered in clip opening
   - Close clip firmly (should click)

2. **Correct Installation**
   ```
   ✅ CORRECT:
   ──────────────────────────────────────
        L1  L2  L3  (3-phase wires)
        │   │   │
       ╱│╲ ╱│╲ ╱│╲   SCT clips around
      (A) (B) (C)    EACH INDIVIDUAL wire
   ```

3. **Wrong Installation**
   ```
   ❌ WRONG:
   ──────────────────────────────────────
        L1  L2  L3
        ││  ││  ││  Wires together in conduit
       ╱││╲╱││╲╱││╲
      (  A+B+C  )   Clip around all wires
                    → Readings will be incorrect!
   ```

**Calibration:**

Initial calibration value in code:
```cpp
#define SCT013_CALIBRATION  30.0  // Adjust this value
```

Calibration procedure:
1. Run motor at known current (use clamp meter)
2. Compare ESP32 reading with actual current
3. Calculate: `NEW_CAL = OLD_CAL × (ACTUAL_CURRENT / ESP32_READING)`
4. Update code and re-upload

Example:
- Current calibration: 30.0
- Actual current: 10.5A (from clamp meter)
- ESP32 reading: 7.2A
- New calibration: 30.0 × (10.5 / 7.2) = 43.75

---

## Power Supply Options

### Option 1: USB Power (Development/Testing)
```
USB Cable (5V, 500mA) ──── ESP32 Micro-USB Port
```
**Pros:** Simple, safe for testing
**Cons:** Limited current, cable may disconnect

---

### Option 2: External 5V Power Supply
```
5V DC Supply (2A+) ─┬─── ESP32 5V Pin
                    └─── ESP32 GND
```
**Pros:** More reliable, higher current capacity
**Cons:** Need regulated 5V supply

---

### Option 3: Industrial Power (Recommended for production)
```
24V DC ──── [Buck Converter] ──── 5V ──── ESP32
                   │
             [LM2596 Module]
```

**Components:**
- Input: 24V DC (common in industrial control panels)
- Buck converter: LM2596 or equivalent (24V → 5V, 3A)
- Output: 5V @ 2A minimum

**Wiring:**
```
24V+ ──── Buck IN+
24V- ──── Buck IN-

Buck OUT+ ──── ESP32 5V
Buck OUT- ──── ESP32 GND
```

**Safety:**
- Add fuse (1A) on 24V input
- Use TVS diode for surge protection
- Ground ESP32 to control panel ground

---

## Relay to Motor Contactor Wiring

### Basic Contactor Control Circuit

```
24V DC Control Power
    │
    ├──── [Emergency Stop NC] ────┐
    │                              │
    ├──── [Start Button NO] ───────┤
    │                              │
    └──── Relay COM                │
                                   │
            Relay NO ──────────────┼──── Contactor A1 (coil +)
                                   │
    Control GND ───────────────────┴──── Contactor A2 (coil -)


Motor Power:
L1, L2, L3 (3-phase) ──── Contactor Main Contacts ──── Motor U, V, W
```

**Safety Features:**
1. Hardware E-stop button in series (NC contact)
2. Relay controlled by ESP32 (additional protection)
3. Contactor protects motor from overload

**Contactor Selection:**
- Rated for motor voltage (230V, 400V, etc.)
- Current rating: 1.5× motor full-load current
- Coil voltage: Match your control voltage (24V DC typical)

---

## Complete System Block Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     ESP32 DEVKIT                            │
│                                                             │
│  GPIO 33 ◄── E-Stop Button                                 │
│  GPIO 32 ──► Relay ──► Motor Contactor                     │
│  GPIO 25 ──► Buzzer                                        │
│  GPIO 5  ──► Status LED                                    │
│                                                             │
│  GPIO 21 ◄─► MPU6050 (SDA) ◄─ Motor Vibration             │
│  GPIO 22 ◄─► MPU6050 (SCL)                                │
│                                                             │
│  GPIO 19 ◄── DS18B20 ◄─ Motor Temperature                 │
│                                                             │
│  GPIO 34 ◄── SCT-013 Phase A ◄─ Motor Current             │
│  GPIO 35 ◄── SCT-013 Phase B                              │
│  GPIO 36 ◄── SCT-013 Phase C                              │
│                                                             │
│  WiFi ◄──► Access Point (192.168.4.1)                     │
│            ◄──► Web Dashboard                              │
└─────────────────────────────────────────────────────────────┘
              │
              ▼
    ┌──────────────────┐
    │  3-Phase Motor   │
    │  U  V  W         │
    └──────────────────┘
```

---

## Breadboard Prototype Layout

For initial testing without motor:

```
┌────────────────────────────────────────────────────────────┐
│                      BREADBOARD                            │
│                                                            │
│   3.3V Rail ════════════════════════════════════ +Rail    │
│                                                            │
│   MPU6050    DS18B20   Button    LED    Buzzer   Relay   │
│   [Module]   [Sensor]  [E-Stop]  [□]    [≈]      [Mod]   │
│      │          │         │       │       │        │      │
│   ───┴──────────┴─────────┴───────┴───────┴────────┴───   │
│                                                            │
│                     ESP32 DevKit                           │
│   ┌──────────────────────────────────────────────────┐   │
│   │  3V3  GND  D21 D22 D19  D5  D25 D32 D33  D34-36 │   │
│   └──────────────────────────────────────────────────┘   │
│                                                            │
│   GND Rail ═════════════════════════════════════ -Rail    │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

**Breadboard Testing Steps:**
1. Power ESP32 via USB only
2. Connect sensors one at a time
3. Verify each sensor in Serial Monitor
4. Test web dashboard without motor running
5. Only connect to real motor after full verification

---

## PCB Design Recommendations

For permanent installation, consider custom PCB:

**Features:**
- Screw terminals for all external connections
- Optocoupler isolation for relay
- TVS diodes for protection
- Status LED indicators
- Mounting holes (M3)
- Proper ground plane

**Layout Tips:**
- Separate analog (ADC) and digital ground planes
- Keep current sensor traces short
- Use wide traces for power (minimum 0.5mm)
- Add test points for debugging

**PCB Tools:**
- KiCad (free, open-source)
- EasyEDA (online, beginner-friendly)
- Altium Designer (professional)

---

## Enclosure and Mounting

### Enclosure Requirements
- **IP Rating**: IP65 minimum (dust/water resistant)
- **Material**: ABS plastic or metal
- **Size**: 150×100×50mm minimum
- **Ventilation**: Small vents with mesh (prevent overheating)
- **Cable Glands**: PG7 or PG9 for wire entry

### Panel Layout
```
┌──────────────────────────────┐
│    Motor Protection Box      │
│                              │
│  [●] Power LED               │
│  [●] Status LED              │
│                              │
│  [🔴] E-Stop Button          │
│                              │
│  [USB] Programming Port      │
│                              │
└──────────────────────────────┘

Bottom cable entries:
 │  │  │  │  │  │  │
 │  │  │  │  │  │  └─ Motor Phase C
 │  │  │  │  │  └──── Motor Phase B
 │  │  │  │  └─────── Motor Phase A
 │  │  │  └────────── Temperature Sensor
 │  │  └───────────── Relay to Contactor
 │  └──────────────── 24V Power In
 └─────────────────── Vibration Sensor
```

---

## Final Wiring Checklist

### Before Power-On
- [ ] All GND connections secure
- [ ] No short circuits (multimeter continuity check)
- [ ] Correct voltage to sensors (3.3V for MPU6050/DS18B20)
- [ ] SCT-013 clips around individual wires
- [ ] Pull-up resistor for DS18B20 installed
- [ ] Relay wiring correct (NO/NC verified)
- [ ] E-stop button wired correctly

### After Power-On (Serial Monitor)
- [ ] ESP32 boots successfully
- [ ] MPU6050 detected at 0x68 or 0x69
- [ ] DS18B20 detected (device count > 0)
- [ ] Current readings not showing errors
- [ ] WiFi AP created ("MotorProtect_ML")
- [ ] Web server started (http://192.168.4.1)

### Functional Tests
- [ ] E-stop button triggers emergency stop
- [ ] Start button energizes relay
- [ ] Stop button de-energizes relay
- [ ] Buzzer sounds on alerts
- [ ] LED blinks with correct pattern
- [ ] Web dashboard accessible
- [ ] All sensor readings update in dashboard

---

## Troubleshooting Wiring Issues

### No Power
- Check USB cable connection
- Verify power supply voltage (5V)
- Measure voltage at ESP32 5V pin with multimeter

### MPU6050 Not Found
- Check I2C wiring (SDA=21, SCL=22)
- Verify 3.3V power to sensor
- Try pulling AD0 high or low (changes address)

### DS18B20 Not Found
- Verify 4.7kΩ pull-up resistor installed
- Check data line connection (GPIO 19)
- Test sensor separately with example code

### Current Always Reads 0
- Verify burden resistor (100Ω) installed
- Check voltage divider (should read ~1.65V)
- Ensure SCT-013 clip fully closed
- Confirm wire passes through clip

### Relay Not Clicking
- Check GPIO 32 connection
- Verify relay power (5V or 12V)
- Test relay separately (apply voltage directly)
- Replace relay if faulty

---

## Safety Ground Connections

**CRITICAL:** Properly ground all metal enclosures and motor frame!

```
Earth Ground ─────┬──── ESP32 GND
                  ├──── Enclosure Metal Case
                  ├──── Motor Frame
                  ├──── Relay Ground
                  └──── Power Supply Ground
```

**Grounding prevents:**
- Electric shock hazards
- EMI/RFI interference
- Surge damage to electronics

---

## Additional Resources

- ESP32 Pinout: [https://randomnerdtutorials.com/esp32-pinout-reference-gpios/](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- SCT-013 Tutorial: [https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/how-to-build-an-arduino-energy-monitor](https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/how-to-build-an-arduino-energy-monitor)
- MPU6050 Wiring: [https://randomnerdtutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/](https://randomnerdtutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/)

---

**Need Help?** Open an issue on GitHub with:
- Photos of your wiring
- Serial Monitor output
- Description of the problem

**⚡ Safety First! Always disconnect power before wiring! ⚡**
