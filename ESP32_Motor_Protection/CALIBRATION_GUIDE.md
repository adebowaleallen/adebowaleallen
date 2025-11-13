# ESP32 Motor Protection System - Calibration Guide

## Table of Contents
1. [Overview](#overview)
2. [Required Equipment](#required-equipment)
3. [Pre-Calibration Checklist](#pre-calibration-checklist)
4. [Current Sensor Calibration](#current-sensor-calibration)
5. [Vibration Threshold Calibration](#vibration-threshold-calibration)
6. [Temperature Sensor Calibration](#temperature-sensor-calibration)
7. [ML Model Training](#ml-model-training)
8. [Validation Testing](#validation-testing)
9. [Troubleshooting](#troubleshooting)

---

## Overview

Proper calibration is essential for accurate motor monitoring and protection. This guide walks you through calibrating all sensors and establishing appropriate thresholds for your specific motor and application.

**Total Time Required:** 60-90 minutes
**Difficulty:** Intermediate
**Safety Level:** HIGH - involves live motor operation

---

## Required Equipment

### Measurement Tools
- [ ] **Digital Clamp Meter** (AC current measurement, 0.1A resolution)
- [ ] **Digital Multimeter** (voltage measurement)
- [ ] **Infrared Thermometer** (optional, for temperature verification)
- [ ] **Tachometer** (optional, for RPM verification)
- [ ] **Smartphone** or Computer (for web dashboard access)

### Motor Information Needed
- [ ] Motor nameplate data (voltage, current, RPM, poles)
- [ ] Motor rated load conditions
- [ ] Motor installation location and orientation

### Software Requirements
- [ ] Arduino IDE with project loaded
- [ ] Serial Monitor access (115200 baud)
- [ ] Web browser for dashboard monitoring
- [ ] Spreadsheet software (for recording data)

---

## Pre-Calibration Checklist

### Step 1: Verify Hardware Installation

```
✅ Checklist:
□ All sensors physically connected
□ SCT-013 clips firmly closed around wires
□ MPU6050 mounted rigidly to motor
□ DS18B20 in good thermal contact with motor
□ Relay wiring verified (no shorts)
□ E-stop button functional
□ Power supply stable (5V ±5%)
□ ESP32 programmed with firmware
```

### Step 2: Initial System Test

1. **Power on ESP32**
2. **Open Serial Monitor** (115200 baud)
3. **Verify startup sequence:**

```
Expected Serial Output:
==========================================
  🚀 MOTOR PROTECTION SYSTEM v3.2
==========================================

✅ GPIO initialized
=== Init MPU6050 ===
✅ Device at 0x68
Accelerometer range set to: +-8G
...
✅ MPU6050 initialized

=== Init DS18B20 ===
✅ DS18B20 detected

✅ ADC initialized
✅ Access Point Created
📡 IP: 192.168.4.1
✅ Web server started!
```

4. **Connect to WiFi:** `MotorProtect_ML` (password: `motor123456`)
5. **Open dashboard:** http://192.168.4.1
6. **Login:** admin / motor2024

### Step 3: Baseline Sensor Check

**Check MPU6050:**
- Serial Monitor should show: `✅ Device at 0x68`
- Dashboard shows: MPU6050 Status = Connected

**Check DS18B20:**
- Serial Monitor should show: `✅ DS18B20 detected`
- Dashboard shows reasonable temperature (15-30°C at rest)

**Check Current Sensors:**
- With motor OFF, current should be ~0.0A (±0.5A tolerance)
- If readings are high (>2A), check SCT-013 placement

---

## Current Sensor Calibration

### Phase 1: Zero-Current Verification

**Objective:** Ensure sensors read ~0A when motor is off

**Procedure:**
1. **Motor Status:** Ensure motor is completely OFF
2. **Open Dashboard:** Navigate to Current (A) card
3. **Record Readings:**
   ```
   Phase A: _____ A (should be 0.0-0.5A)
   Phase B: _____ A
   Phase C: _____ A
   ```

**If readings are > 1A:**
- Check for other loads on the same circuit
- Verify SCT-013 clip is around correct wire
- Ensure burden resistor is 100Ω (not 10Ω or 1kΩ)

**Adjustment (if needed):**
```cpp
// In code, modify readCurrentData() function:
float bias = 1.65; // Adjust between 1.60-1.70 if needed

// OR adjust zero offset:
currentData.currentPhaseA = ((voltA - bias) / 0.185) * SCT013_CALIBRATION / 100.0;
if (abs(currentData.currentPhaseA) < 0.5) currentData.currentPhaseA = 0; // Force zero
```

---

### Phase 2: No-Load Current Calibration

**Objective:** Verify readings at no-load (motor running without load)

**Procedure:**
1. **Prepare Clamp Meter:**
   - Set to AC current mode
   - Place around same wire as SCT-013 Phase A
   - Ensure good contact

2. **Start Motor (No Load):**
   - Click "▶️ Start" on dashboard
   - Wait for stable operation (30 seconds)

3. **Measure and Compare:**
   ```
   Clamp Meter Phase A:  _____ A  (reference)
   ESP32 Phase A:        _____ A  (dashboard reading)

   Clamp Meter Phase B:  _____ A
   ESP32 Phase B:        _____ A

   Clamp Meter Phase C:  _____ A
   ESP32 Phase C:        _____ A
   ```

4. **Calculate Error:**
   ```
   Error (%) = ((ESP32 - Clamp) / Clamp) × 100

   Example:
   Clamp reads: 5.2A
   ESP32 reads: 3.8A
   Error = ((3.8 - 5.2) / 5.2) × 100 = -26.9%  → Needs calibration!
   ```

**Acceptable Error:** ±10%
**If error > 10%:** Proceed to calibration adjustment

---

### Phase 3: Calibration Factor Adjustment

**Current Calibration Formula:**
```
NEW_CALIBRATION = OLD_CALIBRATION × (ACTUAL_CURRENT / ESP32_READING)
```

**Example Calculation:**
```
Given:
- Current calibration (SCT013_CALIBRATION): 30.0
- Actual current (clamp meter): 5.2A
- ESP32 reading (dashboard): 3.8A

Calculation:
NEW_CALIBRATION = 30.0 × (5.2 / 3.8)
NEW_CALIBRATION = 30.0 × 1.368
NEW_CALIBRATION = 41.05

Round to: 41.0
```

**Update Code:**
```cpp
// In ESP32_Motor_Protection.ino, line ~56:
#define SCT013_CALIBRATION      41.0      // Updated from 30.0
```

**Re-upload and Verify:**
1. Upload modified code to ESP32
2. Restart motor
3. Check readings match clamp meter (within ±10%)

**Repeat for all three phases if necessary**

---

### Phase 4: Full-Load Current Calibration

**Objective:** Verify accuracy at rated load

**Procedure:**
1. **Apply Rated Load:**
   - Connect motor to its normal operating load
   - Run at full rated capacity
   - Wait for thermal stabilization (5-10 minutes)

2. **Measure at Full Load:**
   ```
   Clamp Meter Phase A:  _____ A  (should be near motor nameplate rating)
   ESP32 Phase A:        _____ A

   Error (%): _____
   ```

3. **Fine-Tune if Necessary:**
   - If error > 10%, repeat calibration factor adjustment
   - SCT-013 sensors are most accurate at 50-100% of their rated capacity

**Motor Nameplate vs Measured:**
```
Motor Rated Current:  _____ A  (from nameplate)
Measured Current:     _____ A  (clamp meter average)
Load Percentage:      _____ %  (Measured / Rated × 100)
```

---

### Phase 5: Imbalance Detection Test

**Objective:** Verify current imbalance detection

**Procedure:**
1. **Measure All Three Phases:**
   ```
   Phase A: _____ A
   Phase B: _____ A
   Phase C: _____ A

   Average: (A + B + C) / 3 = _____ A
   ```

2. **Calculate Imbalance:**
   ```
   Max Deviation = max(|A-Avg|, |B-Avg|, |C-Avg|)
   Imbalance (%) = (Max Deviation / Average) × 100

   Example:
   Phase A: 10.2A
   Phase B: 10.5A
   Phase C: 9.8A
   Average: 10.17A

   Max Deviation = |10.5 - 10.17| = 0.33A
   Imbalance = (0.33 / 10.17) × 100 = 3.2%
   ```

3. **Verify Dashboard:**
   - Dashboard should show similar imbalance percentage
   - Acceptable imbalance: < 10% (NEMA MG-1 standard)
   - Warning threshold: 20% (configurable)

**Threshold Adjustment:**
```cpp
// In code, line ~64:
#define CURRENT_IMBALANCE_THRESHOLD     0.20     // 20% default
// Adjust to 0.10 (10%) for stricter monitoring
// or 0.30 (30%) for more lenient monitoring
```

---

## Vibration Threshold Calibration

### Phase 1: Baseline Vibration Measurement

**Objective:** Establish normal vibration levels for your motor

**Procedure:**
1. **Run Motor at Normal Load:**
   - Operate motor under typical working conditions
   - Ensure motor is warmed up (10-15 minutes)
   - Check for unusual vibrations manually

2. **Record Vibration Data:**
   - Monitor dashboard for 5 minutes
   - Record peak values every 30 seconds

   ```
   Time    | X-Axis | Y-Axis | Z-Axis | Notes
   --------|--------|--------|--------|------------------
   0:30    | 2.34   | 1.89   | 2.12   |
   1:00    | 2.41   | 1.95   | 2.08   |
   1:30    | 2.38   | 1.91   | 2.15   |
   ...
   5:00    | 2.36   | 1.88   | 2.11   |

   Average:| 2.37   | 1.91   | 2.11   | ← Normal baseline
   Max:    | 2.41   | 1.95   | 2.15   | ← Peak normal
   ```

3. **Calculate Thresholds:**
   ```
   Warning Threshold = Max Normal × 1.5
   Critical Threshold = Max Normal × 2.0

   Example (X-axis):
   Max Normal = 2.41 m/s²
   Warning = 2.41 × 1.5 = 3.62 m/s²
   Critical = 2.41 × 2.0 = 4.82 m/s²
   ```

**Update Thresholds:**
```cpp
// In code, lines ~59-60:
#define VIBRATION_WARNING_THRESHOLD     3.6      // Adjusted from 5.0
#define CRITICAL_VIBRATION_THRESHOLD    4.8      // Adjusted from 8.0
```

---

### Phase 2: MPU6050 Orientation Verification

**Objective:** Understand accelerometer axis orientation

**MPU6050 Axis Reference:**
```
         ┌─────────┐
         │ MPU6050 │
         │         │
    ↑ Y  │    ●    │  ↑ Z (out of board)
    │    │         │
    │    └─────────┘
    └──→ X
```

**Motor Installation:**
- **X-axis:** Typically along motor shaft (axial vibration)
- **Y-axis:** Perpendicular to shaft (radial vibration)
- **Z-axis:** Vertical (depends on motor orientation)

**Verify Orientation:**
1. Gently tap motor in known direction
2. Observe which axis shows spike on dashboard
3. Document orientation for future reference

---

### Phase 3: Vibration Fault Simulation

**Objective:** Test critical vibration shutdown

**Procedure:**
1. **Lower Critical Threshold (temporarily):**
   ```cpp
   #define CRITICAL_VIBRATION_THRESHOLD    2.0  // Very low for testing
   ```

2. **Induce Vibration:**
   - Safely introduce controlled vibration (e.g., slight motor misalignment)
   - OR tap motor body while running
   - Watch for critical shutdown

3. **Expected Behavior:**
   - Dashboard shows "CRITICAL" status
   - Buzzer sounds 5 rapid beeps
   - Motor automatically stops
   - Serial Monitor: `🚨 CRITICAL SHUTDOWN!`

4. **Reset and Restore Threshold:**
   - Click "↻ Reset" button
   - Restore normal threshold value
   - Re-upload code

---

## Temperature Sensor Calibration

### Phase 1: Ambient Temperature Verification

**Procedure:**
1. **Motor OFF, Room Temperature:**
   - Read dashboard temperature: _____ °C
   - Compare with room thermometer: _____ °C
   - Difference: _____ °C

**Acceptable Difference:** ±2°C

**If difference > 2°C:**
```cpp
// In readTemperatureC() function, add offset:
float tempC = tempSensor.getTempCByIndex(0);
tempC = tempC + TEMP_OFFSET;  // e.g., +2.5 or -1.5
return tempC;
```

---

### Phase 2: Operating Temperature Baseline

**Procedure:**
1. **Run Motor at Full Load:**
   - Operate for 30 minutes (thermal stabilization)
   - Record temperature every 5 minutes

   ```
   Time (min) | ESP32 Temp | IR Temp | Motor Feel
   -----------|------------|---------|------------
   5          | 35.2°C     | 36.1°C  | Warm
   10         | 42.8°C     | 43.5°C  | Hot
   15         | 48.3°C     | 49.1°C  | Hot
   20         | 52.1°C     | 52.9°C  | Very hot
   25         | 54.6°C     | 55.2°C  | Very hot
   30         | 55.8°C     | 56.4°C  | Stable

   Steady-State Temp: ~56°C
   ```

2. **Set Temperature Thresholds:**
   ```
   Normal Operating Range: 40-60°C (typical)
   Warning Threshold: 70% of motor max rated temp
   Critical Threshold: 90% of motor max rated temp

   Example (motor rated for 100°C max):
   Warning = 100 × 0.70 = 70°C
   Critical = 100 × 0.90 = 90°C
   ```

**Update Thresholds:**
```cpp
// In code, lines ~61-62:
#define TEMP_WARNING                    70.0     // °C
#define TEMP_CRITICAL                   90.0     // °C
```

---

### Phase 3: Temperature Alarm Test

**Procedure:**
1. **Lower Critical Threshold (temporarily):**
   ```cpp
   #define TEMP_CRITICAL    45.0  // Low value for testing
   ```

2. **Run Motor Until Alarm:**
   - Motor should trigger shutdown when temp exceeds 45°C
   - Verify automatic relay trip
   - Check buzzer and LED alerts

3. **Restore Normal Threshold:**
   - Reset system
   - Update threshold to normal value (e.g., 90°C)
   - Re-upload code

---

## ML Model Training

### Phase 1: Training Data Collection

**Objective:** Establish statistical baseline for anomaly detection

**Prerequisites:**
- Motor running under normal, stable conditions
- No known faults or abnormalities
- All sensors operational

**Procedure:**
1. **Start Motor:**
   - Click "▶️ Start" on dashboard
   - Let motor reach steady-state (5 minutes)

2. **Monitor Training Progress:**
   - Dashboard shows: "ML Training Status: Training..."
   - Baseline Samples: 0/200 → 200/200
   - Training takes ~20 seconds (200 samples @ 10 Hz)

3. **Training Complete:**
   ```
   Serial Monitor:
   📊 Training: 50/200 samples
   📊 Training: 100/200 samples
   📊 Training: 150/200 samples
   📊 Training: 200/200 samples
   ✅ ML Model trained!
   📈 Baseline X: mean=2.35 std=0.42
   ```

   Dashboard shows: "ML Training Status: ✅ Trained"

---

### Phase 2: ML Parameters Understanding

**Z-Score Anomaly Detection:**
```
Z-Score = |Current Value - Mean| / Standard Deviation

Example:
Mean vibration (X): 2.35 m/s²
Std Dev: 0.42 m/s²
Current reading: 3.50 m/s²

Z-Score = |3.50 - 2.35| / 0.42 = 2.74

If Z-Score > 3.0 → ANOMALY DETECTED
```

**Anomaly Score:**
```
Combined score from all sensors:
Anomaly Score = (Z_vib_X + Z_vib_Y + Z_vib_Z + Z_current + Z_temp) / 5
```

---

### Phase 3: ML Threshold Tuning

**Default Threshold:**
```cpp
#define ML_ANOMALY_THRESHOLD    3.0   // 3-sigma rule (99.7% confidence)
```

**Adjustment Guidelines:**

| Threshold | Sensitivity | False Alarms | Missed Faults |
|-----------|-------------|--------------|---------------|
| 2.0 | Very High | Many | Few |
| 2.5 | High | Some | Few |
| 3.0 | **Medium (recommended)** | **Rare** | **Low** |
| 3.5 | Low | Very Rare | Some |
| 4.0 | Very Low | Almost None | Many |

**Tuning Procedure:**
1. Run motor for 1 week with default (3.0)
2. Record false alarm frequency from logs
3. If too many false alarms: increase to 3.5 or 4.0
4. If faults missed: decrease to 2.5 or 2.0

---

### Phase 4: Re-Training

**When to Re-Train:**
- After motor maintenance
- After load profile changes
- After sensor replacement
- Seasonal temperature variations

**Re-Training Procedure:**
1. **Reset ML Model:**
   - Power cycle ESP32, OR
   - Modify code to force re-training:
   ```cpp
   void setup() {
     ...
     mlModel.isTrained = false;  // Force re-training
     mlModel.baselineCount = 0;
   }
   ```

2. **Run Training:**
   - Follow Phase 1 procedure
   - Ensure stable conditions

3. **Verify New Baseline:**
   - Check Serial Monitor for new mean/std values
   - Compare with previous baseline
   - Large differences indicate changed operating conditions

---

## Validation Testing

### Test 1: Emergency Stop

**Procedure:**
1. Start motor from dashboard
2. Press physical E-stop button
3. **Expected:**
   - Motor stops immediately
   - Buzzer sounds 3 times
   - Dashboard shows "De-energized"
   - LED blinks rapidly (300ms)
4. Click "↻ Reset" to clear

✅ PASS / ❌ FAIL

---

### Test 2: Overcurrent Protection

**Procedure:**
1. Temporarily lower threshold:
   ```cpp
   #define OVERCURRENT_THRESHOLD  5.0  // Low for testing
   ```
2. Run motor under load exceeding threshold
3. **Expected:**
   - Critical shutdown triggered
   - Serial: "🚨 OVERCURRENT!"
   - Automatic motor stop
4. Restore normal threshold

✅ PASS / ❌ FAIL

---

### Test 3: Vibration Fault Response

**Procedure:**
1. Start motor
2. Induce vibration (tap motor body firmly)
3. **Expected:**
   - Warning badge shows yellow/orange
   - If critical: automatic shutdown
   - Anomaly score increases

✅ PASS / ❌ FAIL

---

### Test 4: Temperature Alarm

**Procedure:**
1. Monitor temperature rise over 30 minutes
2. Verify warning at 60°C (or your threshold)
3. Verify critical shutdown at 80°C (or your threshold)

✅ PASS / ❌ FAIL

---

### Test 5: ML Anomaly Detection

**Procedure:**
1. After ML training complete
2. Introduce abnormal condition:
   - Increase load significantly
   - Induce vibration
   - Simulate bearing fault
3. **Expected:**
   - Anomaly score increases
   - If score > 3.0: critical shutdown
   - Dashboard shows "ANOMALY" status

✅ PASS / ❌ FAIL

---

### Test 6: Web Dashboard Functionality

**Checklist:**
- [ ] Dashboard loads successfully
- [ ] Authentication works (admin/motor2024)
- [ ] Real-time data updates (1Hz)
- [ ] Charts render correctly
- [ ] Start/Stop/Reset buttons functional
- [ ] Theme toggle works (dark/light)
- [ ] Mobile responsive layout

✅ PASS / ❌ FAIL

---

## Troubleshooting

### Issue: Current readings too high (>20A with motor off)

**Causes:**
- SCT-013 not properly clipped
- Multiple wires through sensor
- Wrong burden resistor value

**Solutions:**
1. Verify one wire only through SCT-013
2. Check burden resistor: should be 100Ω
3. Verify voltage divider: should read 1.65V at ADC pin

---

### Issue: Vibration always shows CRITICAL

**Causes:**
- Loose MPU6050 mounting
- Wrong threshold values
- Noisy environment

**Solutions:**
1. Firmly mount MPU6050 to motor
2. Increase thresholds:
   ```cpp
   #define CRITICAL_VIBRATION_THRESHOLD  10.0  // or higher
   ```
3. Check for external vibration sources

---

### Issue: Temperature sensor reads -127°C

**Causes:**
- DS18B20 not connected
- Missing pull-up resistor
- Wrong GPIO pin

**Solutions:**
1. Check DS18B20 wiring (red=3.3V, yellow=GPIO19, black=GND)
2. Verify 4.7kΩ pull-up resistor between data and VCC
3. Test with DS18B20 example code

---

### Issue: ML model never completes training

**Causes:**
- MPU6050 not connected
- Motor not running
- Rapid power cycles

**Solutions:**
1. Verify MPU6050 detection in Serial Monitor
2. Start motor before training begins
3. Let system run uninterrupted for 30+ seconds

---

### Issue: False alarms during normal operation

**Causes:**
- Thresholds too strict
- ML model trained during abnormal conditions
- Sensor noise

**Solutions:**
1. Increase all thresholds by 20-30%
2. Re-train ML model under stable conditions
3. Add filtering capacitors to current sensors

---

## Calibration Data Sheet

**Record your calibrated values here:**

### Motor Information
```
Motor Manufacturer: _________________________
Motor Model: ________________________________
Rated Voltage: _________ V
Rated Current: _________ A
Rated RPM: _________ RPM
Motor Poles: _________
Power Rating: _________ HP / kW
```

### Current Sensor Calibration
```
SCT013_CALIBRATION: _________

Phase A Actual: _____ A | ESP32: _____ A | Error: _____%
Phase B Actual: _____ A | ESP32: _____ A | Error: _____%
Phase C Actual: _____ A | ESP32: _____ A | Error: _____%

Imbalance Threshold: _________%
Overcurrent Threshold: _________ A
```

### Vibration Calibration
```
Baseline X-axis: _________ m/s²
Baseline Y-axis: _________ m/s²
Baseline Z-axis: _________ m/s²

Warning Threshold: _________ m/s²
Critical Threshold: _________ m/s²
```

### Temperature Calibration
```
Ambient Temp: _________ °C
Operating Temp (steady-state): _________ °C

Temp Offset: _________ °C
Warning Threshold: _________ °C
Critical Threshold: _________ °C
```

### ML Model Parameters
```
Anomaly Threshold: _________
Baseline Samples: _________
Training Date: _________________

Mean Vibration X: _________
Std Dev X: _________
Mean Current: _________
Std Dev Current: _________
```

### Validation Test Results
```
Emergency Stop: PASS / FAIL
Overcurrent Protection: PASS / FAIL
Vibration Response: PASS / FAIL
Temperature Alarm: PASS / FAIL
ML Anomaly Detection: PASS / FAIL
Web Dashboard: PASS / FAIL
```

### Notes
```
_____________________________________________________
_____________________________________________________
_____________________________________________________
```

**Calibrated by:** _________________  **Date:** __________
**Verified by:** ___________________  **Date:** __________

---

## Recalibration Schedule

**Recommended:**
- **Weekly:** Visual inspection, functional tests
- **Monthly:** Verify sensor readings with external instruments
- **Quarterly:** Full recalibration procedure
- **Annually:** Comprehensive system audit, sensor replacement if needed

---

**For technical support, refer to README.md or open a GitHub issue.**

**⚙️ Proper calibration ensures reliable motor protection! ⚙️**
