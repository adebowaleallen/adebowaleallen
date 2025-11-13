/*
  ==========================================
  3-PHASE MOTOR PROTECTION SYSTEM v3.2
  ==========================================
  ESP32 + MPU6050 + SCT013 + DS18B20
  Vibration + Current + Temperature Monitoring
  With ML-based Anomaly Detection

  Access: http://192.168.4.1
  Username: admin
  Password: motor2024
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include "arduinoFFT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>
#include <esp_task_wdt.h>

// ==================== CONFIGURATION ====================
// WiFi Settings
const char* ssid     = "uranus";
const char* password = "Oludare1234";

// Web Authentication
const char* www_username = "admin";
const char* www_password = "motor2024";

// ==================== GPIO PINS ====================
#define ESTOP_BUTTON_PIN  33
#define RELAY_PIN         32
#define BUZZER_PIN        25
#define STATUS_LED_PIN    5
#define TEMP_SENSOR_PIN   19
#define CURRENT_PHASE_A   34
#define CURRENT_PHASE_B   35
#define CURRENT_PHASE_C   36
#define SDA_PIN           21
#define SCL_PIN           22

// ==================== SENSOR THRESHOLDS ====================
#define VIBRATION_WARNING_THRESHOLD     5.0      // m/s²
#define CRITICAL_VIBRATION_THRESHOLD    8.0      // m/s²
#define TEMP_WARNING                    60.0     // °C
#define TEMP_CRITICAL                   80.0     // °C
#define OVERCURRENT_THRESHOLD           15.0     // Amps
#define CURRENT_IMBALANCE_THRESHOLD     0.20     // 20%

// ==================== MOTOR SPECIFICATIONS ====================
#define RATED_CURRENT           10.0      // Amps (adjust for your motor)
#define SCT013_CALIBRATION      30.0      // Adjust during calibration
#define MOTOR_RPM               1750      // Check motor nameplate
#define MOTOR_POLES             4         // Calculate from nameplate

// ==================== ML CONFIGURATION ====================
#define ML_ANOMALY_THRESHOLD    3.0       // Z-score threshold
#define ML_BASELINE_SAMPLES     200       // Samples to establish baseline
#define MAX_TRENDS              100       // Historical data points

// ==================== FFT CONFIGURATION ====================
#define SAMPLES                 256
#define SAMPLING_FREQUENCY      1000

// ==================== OBJECTS ====================
WebServer server(80);
Adafruit_MPU6050 mpu;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature tempSensor(&oneWire);
Preferences preferences;

// ==================== DATA STRUCTURES ====================
struct CurrentData {
  float currentPhaseA = 0.0;
  float currentPhaseB = 0.0;
  float currentPhaseC = 0.0;
  float avgCurrent = 0.0;
  float currentImbalance = 0.0;
  bool relayA = false;
  bool relayB = false;
  bool relayC = false;
  float frequency = 50.0;
  bool sensorOK = false;
};

struct VibrationData {
  float accelX = 0.0, accelY = 0.0, accelZ = 0.0;
  float peakMagX = 0.0, peakMagY = 0.0, peakMagZ = 0.0;
  float rmsX = 0.0, rmsY = 0.0, rmsZ = 0.0;
  float statusX = 0, statusY = 0, statusZ = 0;
  bool faultX = false, faultY = false, faultZ = false;
  float waveformX[256];
};

struct MLModel {
  bool isTrained = false;
  float baselineX[MAX_TRENDS] = {0};
  float baselineY[MAX_TRENDS] = {0};
  float baselineZ[MAX_TRENDS] = {0};
  float baselineI[MAX_TRENDS] = {0};
  float baselineT[MAX_TRENDS] = {0};
  float stdDevX = 0.0, stdDevY = 0.0, stdDevZ = 0.0;
  float stdDevI = 0.0, stdDevT = 0.0;
  float meanX = 0.0, meanY = 0.0, meanZ = 0.0;
  float meanI = 0.0, meanT = 0.0;
  float anomalyScore = 0.0;
  int baselineCount = 0;
};

// ==================== GLOBAL VARIABLES ====================
CurrentData currentData;
VibrationData vibData;
MLModel mlModel;
bool mpuConnected = false;
bool tempSensorConnected = false;
bool motorRunning = false;
bool emergencyStop = false;
bool criticalShutdown = false;
uint32_t shutdownTime = 0;
float temperature = 25.0;
uint32_t dataCount = 0;
float healthScore = 100.0;

// ==================== FORWARD DECLARATIONS ====================
void handleRoot();
void handleData();
void handleStart();
void handleStop();
void handleReset();
void loadSettings();
void saveSettings();

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n\n==========================================");
  Serial.println("  🚀 MOTOR PROTECTION SYSTEM v3.2");
  Serial.println("==========================================\n");

  // GPIO Setup
  pinMode(ESTOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  Serial.println("✅ GPIO initialized");

  // I2C & MPU6050
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  delay(100);
  scanI2C();
  mpuConnected = initMPU6050();
  currentData.sensorOK = mpuConnected;

  // DS18B20 Temperature
  Serial.println("\n=== Init DS18B20 ===");
  tempSensor.begin();
  delay(100);
  if (tempSensor.getDeviceCount() > 0) {
    Serial.println("✅ DS18B20 detected");
    tempSensorConnected = true;
  } else {
    Serial.println("⚠️ DS18B20 not found");
    tempSensorConnected = false;
  }

  // ADC Setup
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  analogSetPinAttenuation(CURRENT_PHASE_A, ADC_11db);
  analogSetPinAttenuation(CURRENT_PHASE_B, ADC_11db);
  analogSetPinAttenuation(CURRENT_PHASE_C, ADC_11db);
  Serial.println("✅ ADC initialized");

  // ML Model
  initMLModel();

  // Load saved settings
  loadSettings();

  // WiFi Setup
  Serial.println("\n=== WiFi ===");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("MotorProtect_ML", "motor123456");
  Serial.println("✅ Access Point Created");
  Serial.print("📡 IP: ");
  Serial.println(WiFi.softAPIP());

  // Web Server Routes
  server.on("/", HTTP_GET, [](){
    if(!server.authenticate(www_username, www_password)){
      return server.requestAuthentication();
    }
    handleRoot();
  });

  server.on("/data", HTTP_GET, [](){
    if(!server.authenticate(www_username, www_password)){
      return server.requestAuthentication();
    }
    handleData();
  });

  server.on("/start", HTTP_POST, [](){
    if(!server.authenticate(www_username, www_password)){
      return server.requestAuthentication();
    }
    handleStart();
  });

  server.on("/stop", HTTP_POST, [](){
    if(!server.authenticate(www_username, www_password)){
      return server.requestAuthentication();
    }
    handleStop();
  });

  server.on("/reset", HTTP_POST, [](){
    if(!server.authenticate(www_username, www_password)){
      return server.requestAuthentication();
    }
    handleReset();
  });

  server.on("/favicon.ico", [](){ server.send(204); });
  server.begin();

  Serial.println("\n✅ Web server started!");
  Serial.println("🔒 Login: " + String(www_username) + " / " + String(www_password));
  Serial.println("🌐 http://192.168.4.1");

  // Watchdog Timer - Fixed for newer ESP32 core
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 10000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);

  Serial.println("\n==========================================");
  Serial.println("  ✅ SYSTEM READY");
  Serial.println("==========================================\n");

  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

// ==================== MAIN LOOP ====================
void loop() {
  esp_task_wdt_reset();
  server.handleClient();
  checkEmergencyStop();

  // Status LED
  static uint32_t lastBlink = 0;
  static bool ledState = false;
  uint32_t interval = criticalShutdown ? 150 : (emergencyStop ? 300 : 700);
  if (millis() - lastBlink > interval) {
    lastBlink = millis();
    ledState = !ledState;
    digitalWrite(STATUS_LED_PIN, ledState ? HIGH : LOW);
  }

  // Main monitoring loop
  static uint32_t lastCheck = 0;
  if (millis() - lastCheck < 100) return;
  lastCheck = millis();

  if (!mpuConnected) {
    Serial.println("⚠️ MPU6050 disconnected - reinitializing...");
    mpuConnected = initMPU6050();
  }

  // Read sensors
  readVibrationData();
  readCurrentData();
  temperature = readTemperatureC();

  // ML Processing
  if (!mlModel.isTrained) {
    trainMLModel();
  } else {
    detectAnomalies();
  }

  // Calculate health score
  updateHealthScore();

  // Check critical conditions
  checkCriticalConditions();

  // Auto-recovery
  if (criticalShutdown && (millis() - shutdownTime > 5000)) {
    criticalShutdown = false;
    Serial.println("✅ Critical shutdown cleared - ready to restart");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
  }

  dataCount++;
}

// ==================== I2C SCANNING ====================
void scanI2C() {
  Serial.println("Scanning I2C...");
  byte count = 0;
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("✅ Device at 0x");
      Serial.println(i, HEX);
      count++;
    }
  }
  Serial.println("Scan complete.\n");
}

// ==================== MPU6050 INITIALIZATION ====================
bool initMPU6050() {
  Serial.println("=== Initializing MPU6050 ===");

  if (!mpu.begin(0x68, &Wire, 0)) {
    Serial.println("❌ MPU6050 not found at 0x68!");
    if (!mpu.begin(0x69, &Wire, 0)) {
      Serial.println("❌ MPU6050 not found at 0x69 either!");
      return false;
    }
  }

  // Set ranges for 8G accelerometer
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }

  Serial.println("✅ MPU6050 initialized");
  return true;
}

// ==================== VIBRATION ANALYSIS ====================
void readVibrationData() {
  if (!mpuConnected) return;

  // Collect samples
  float vX[SAMPLES], vY[SAMPLES], vZ[SAMPLES];

  for (int i = 0; i < SAMPLES; i++) {
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    vX[i] = accel.acceleration.x;
    vY[i] = accel.acceleration.y;
    vZ[i] = accel.acceleration.z;

    delayMicroseconds(1000000 / SAMPLING_FREQUENCY);
    esp_task_wdt_reset();
  }

  // Calculate Peak Magnitude
  vibData.peakMagX = 0;
  vibData.peakMagY = 0;
  vibData.peakMagZ = 0;
  vibData.rmsX = 0;
  vibData.rmsY = 0;
  vibData.rmsZ = 0;

  for (int i = 0; i < SAMPLES; i++) {
    if (abs(vX[i]) > vibData.peakMagX) vibData.peakMagX = abs(vX[i]);
    if (abs(vY[i]) > vibData.peakMagY) vibData.peakMagY = abs(vY[i]);
    if (abs(vZ[i]) > vibData.peakMagZ) vibData.peakMagZ = abs(vZ[i]);

    vibData.rmsX += vX[i] * vX[i];
    vibData.rmsY += vY[i] * vY[i];
    vibData.rmsZ += vZ[i] * vZ[i];

    if (i < 256) vibData.waveformX[i] = vX[i];
  }

  vibData.rmsX = sqrt(vibData.rmsX / SAMPLES);
  vibData.rmsY = sqrt(vibData.rmsY / SAMPLES);
  vibData.rmsZ = sqrt(vibData.rmsZ / SAMPLES);

  // Status Assessment
  vibData.statusX = (vibData.peakMagX > CRITICAL_VIBRATION_THRESHOLD) ? 2 :
                    (vibData.peakMagX > VIBRATION_WARNING_THRESHOLD) ? 1 : 0;
  vibData.statusY = (vibData.peakMagY > CRITICAL_VIBRATION_THRESHOLD) ? 2 :
                    (vibData.peakMagY > VIBRATION_WARNING_THRESHOLD) ? 1 : 0;
  vibData.statusZ = (vibData.peakMagZ > CRITICAL_VIBRATION_THRESHOLD) ? 2 :
                    (vibData.peakMagZ > VIBRATION_WARNING_THRESHOLD) ? 1 : 0;

  vibData.faultX = (vibData.statusX == 2);
  vibData.faultY = (vibData.statusY == 2);
  vibData.faultZ = (vibData.statusZ == 2);
}

// ==================== CURRENT ANALYSIS ====================
void readCurrentData() {
  int rawA = analogRead(CURRENT_PHASE_A);
  int rawB = analogRead(CURRENT_PHASE_B);
  int rawC = analogRead(CURRENT_PHASE_C);

  // Convert ADC to voltage (3.3V / 4096)
  float voltA = (rawA / 4095.0) * 3.3;
  float voltB = (rawB / 4095.0) * 3.3;
  float voltC = (rawC / 4095.0) * 3.3;

  // Remove bias and convert to current
  float bias = 1.65; // 3.3V / 2
  currentData.currentPhaseA = ((voltA - bias) / 0.185) * SCT013_CALIBRATION / 100.0;
  currentData.currentPhaseB = ((voltB - bias) / 0.185) * SCT013_CALIBRATION / 100.0;
  currentData.currentPhaseC = ((voltC - bias) / 0.185) * SCT013_CALIBRATION / 100.0;

  // Ensure non-negative
  if (currentData.currentPhaseA < 0) currentData.currentPhaseA = 0;
  if (currentData.currentPhaseB < 0) currentData.currentPhaseB = 0;
  if (currentData.currentPhaseC < 0) currentData.currentPhaseC = 0;

  // Average and imbalance
  currentData.avgCurrent = (currentData.currentPhaseA + currentData.currentPhaseB + currentData.currentPhaseC) / 3.0;

  if (currentData.avgCurrent > 1.0) {
    float maxDev = max({
      abs(currentData.currentPhaseA - currentData.avgCurrent),
      abs(currentData.currentPhaseB - currentData.avgCurrent),
      abs(currentData.currentPhaseC - currentData.avgCurrent)
    });
    currentData.currentImbalance = maxDev / currentData.avgCurrent;
  } else {
    currentData.currentImbalance = 0.0;
  }
}

// ==================== TEMPERATURE ====================
float readTemperatureC() {
  if (!tempSensorConnected) return 25.0;

  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) {
    tempSensorConnected = false;
    return 25.0;
  }

  tempSensorConnected = true;
  return tempC;
}

// ==================== EMERGENCY STOP ====================
void checkEmergencyStop() {
  static uint32_t last = 0;
  if (millis() - last < 50) return;
  last = millis();

  if (digitalRead(ESTOP_BUTTON_PIN) == LOW) {
    if (!emergencyStop) {
      emergencyStop = true;
      motorRunning = false;
      updateRelay();
      Serial.println("\n🚨 EMERGENCY STOP!\n");

      for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(300);
        digitalWrite(BUZZER_PIN, LOW);
        delay(300);
      }
    }
  }
}

// ==================== RELAY CONTROL ====================
void updateRelay() {
  if (motorRunning && !emergencyStop && !criticalShutdown) {
    digitalWrite(RELAY_PIN, LOW);
    currentData.relayA = true;
    currentData.relayB = true;
    currentData.relayC = true;
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    currentData.relayA = false;
    currentData.relayB = false;
    currentData.relayC = false;
  }
}

// ==================== CRITICAL CONDITIONS ====================
void checkCriticalConditions() {
  bool critical = false;
  String alertMsg = "";

  if (vibData.faultX) { critical = true; alertMsg += "CRITICAL X! "; }
  if (vibData.faultY) { critical = true; alertMsg += "CRITICAL Y! "; }
  if (vibData.faultZ) { critical = true; alertMsg += "CRITICAL Z! "; }

  if (temperature > TEMP_CRITICAL) { critical = true; alertMsg += "CRITICAL Temp! "; }

  if (currentData.currentPhaseA > OVERCURRENT_THRESHOLD ||
      currentData.currentPhaseB > OVERCURRENT_THRESHOLD ||
      currentData.currentPhaseC > OVERCURRENT_THRESHOLD) {
    critical = true;
    alertMsg += "OVERCURRENT! ";
  }

  if (currentData.avgCurrent > 1.0 && currentData.currentImbalance > CURRENT_IMBALANCE_THRESHOLD) {
    critical = true;
    alertMsg += "Imbalance! ";
  }

  if (mlModel.anomalyScore > ML_ANOMALY_THRESHOLD && mlModel.isTrained) {
    critical = true;
    alertMsg += "ML Anomaly! ";
  }

  if (critical && motorRunning && !criticalShutdown) {
    criticalShutdown = true;
    motorRunning = false;
    shutdownTime = millis();
    updateRelay();

    Serial.println("\n🚨 CRITICAL SHUTDOWN!");
    Serial.println(alertMsg);

    for (int i = 0; i < 5; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
      delay(200);
    }
  }
}

// ==================== ML MODEL TRAINING ====================
void initMLModel() {
  mlModel.isTrained = false;
  mlModel.baselineCount = 0;
}

void trainMLModel() {
  if (!mpuConnected || vibData.peakMagX == 0) return;

  if (mlModel.baselineCount < ML_BASELINE_SAMPLES) {
    mlModel.baselineX[mlModel.baselineCount] = vibData.peakMagX;
    mlModel.baselineY[mlModel.baselineCount] = vibData.peakMagY;
    mlModel.baselineZ[mlModel.baselineCount] = vibData.peakMagZ;
    mlModel.baselineI[mlModel.baselineCount] = currentData.avgCurrent;
    mlModel.baselineT[mlModel.baselineCount] = temperature;

    mlModel.baselineCount++;

    if (mlModel.baselineCount % 50 == 0) {
      Serial.print("📊 Training: ");
      Serial.print(mlModel.baselineCount);
      Serial.print("/");
      Serial.print(ML_BASELINE_SAMPLES);
      Serial.println(" samples");
    }

    if (mlModel.baselineCount >= ML_BASELINE_SAMPLES) {
      calculateBaseline();
      mlModel.isTrained = true;
      Serial.println("✅ ML Model trained!");
    }
  }
}

void calculateBaseline() {
  // Calculate means
  for (int i = 0; i < ML_BASELINE_SAMPLES; i++) {
    mlModel.meanX += mlModel.baselineX[i];
    mlModel.meanY += mlModel.baselineY[i];
    mlModel.meanZ += mlModel.baselineZ[i];
    mlModel.meanI += mlModel.baselineI[i];
    mlModel.meanT += mlModel.baselineT[i];
  }
  mlModel.meanX /= ML_BASELINE_SAMPLES;
  mlModel.meanY /= ML_BASELINE_SAMPLES;
  mlModel.meanZ /= ML_BASELINE_SAMPLES;
  mlModel.meanI /= ML_BASELINE_SAMPLES;
  mlModel.meanT /= ML_BASELINE_SAMPLES;

  // Calculate standard deviations
  for (int i = 0; i < ML_BASELINE_SAMPLES; i++) {
    mlModel.stdDevX += pow(mlModel.baselineX[i] - mlModel.meanX, 2);
    mlModel.stdDevY += pow(mlModel.baselineY[i] - mlModel.meanY, 2);
    mlModel.stdDevZ += pow(mlModel.baselineZ[i] - mlModel.meanZ, 2);
    mlModel.stdDevI += pow(mlModel.baselineI[i] - mlModel.meanI, 2);
    mlModel.stdDevT += pow(mlModel.baselineT[i] - mlModel.meanT, 2);
  }
  mlModel.stdDevX = sqrt(mlModel.stdDevX / ML_BASELINE_SAMPLES);
  mlModel.stdDevY = sqrt(mlModel.stdDevY / ML_BASELINE_SAMPLES);
  mlModel.stdDevZ = sqrt(mlModel.stdDevZ / ML_BASELINE_SAMPLES);
  mlModel.stdDevI = sqrt(mlModel.stdDevI / ML_BASELINE_SAMPLES);
  mlModel.stdDevT = sqrt(mlModel.stdDevT / ML_BASELINE_SAMPLES);

  Serial.print("📈 Baseline X: mean=");
  Serial.print(mlModel.meanX, 2);
  Serial.print(" std=");
  Serial.println(mlModel.stdDevX, 2);
}

void detectAnomalies() {
  if (mlModel.stdDevX == 0) return;

  // Calculate Z-scores
  float zX = (mlModel.stdDevX > 0.01) ? abs(vibData.peakMagX - mlModel.meanX) / mlModel.stdDevX : 0;
  float zY = (mlModel.stdDevY > 0.01) ? abs(vibData.peakMagY - mlModel.meanY) / mlModel.stdDevY : 0;
  float zZ = (mlModel.stdDevZ > 0.01) ? abs(vibData.peakMagZ - mlModel.meanZ) / mlModel.stdDevZ : 0;
  float zI = (mlModel.stdDevI > 0.01) ? abs(currentData.avgCurrent - mlModel.meanI) / mlModel.stdDevI : 0;
  float zT = (mlModel.stdDevT > 0.01) ? abs(temperature - mlModel.meanT) / mlModel.stdDevT : 0;

  // Combined anomaly score
  mlModel.anomalyScore = (zX + zY + zZ + zI + zT) / 5.0;
}

// ==================== HEALTH SCORE ====================
void updateHealthScore() {
  healthScore = 100.0;

  // Vibration penalty
  if (vibData.peakMagX > CRITICAL_VIBRATION_THRESHOLD) healthScore -= 30;
  else if (vibData.peakMagX > VIBRATION_WARNING_THRESHOLD) healthScore -= 10;

  if (vibData.peakMagY > CRITICAL_VIBRATION_THRESHOLD) healthScore -= 30;
  else if (vibData.peakMagY > VIBRATION_WARNING_THRESHOLD) healthScore -= 10;

  if (vibData.peakMagZ > CRITICAL_VIBRATION_THRESHOLD) healthScore -= 30;
  else if (vibData.peakMagZ > VIBRATION_WARNING_THRESHOLD) healthScore -= 10;

  // Temperature penalty
  if (temperature > TEMP_CRITICAL) healthScore -= 25;
  else if (temperature > TEMP_WARNING) healthScore -= 10;

  // Current penalty
  if (currentData.avgCurrent > OVERCURRENT_THRESHOLD) healthScore -= 20;
  if (currentData.currentImbalance > CURRENT_IMBALANCE_THRESHOLD) healthScore -= 15;

  // ML penalty
  if (mlModel.isTrained && mlModel.anomalyScore > ML_ANOMALY_THRESHOLD) {
    healthScore -= (mlModel.anomalyScore - ML_ANOMALY_THRESHOLD) * 10;
  }

  if (healthScore < 0) healthScore = 0;
}

// ==================== WEB HANDLERS ====================
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Motor Protection - v3.2</title>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"></script>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #1e1e2e 0%, #2d2d44 100%);
      color: #e0e0e0;
      --pad: 20px;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
    }
    body.light-mode { background: linear-gradient(135deg, #f5f5f5 0%, #e8e8e8 100%); color: #333; }

    header {
      background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
      padding: var(--pad);
      text-align: center;
      box-shadow: 0 4px 15px rgba(0,0,0,0.3);
      position: relative;
      overflow: hidden;
    }
    header::before {
      content: '';
      position: absolute;
      top: -50%;
      right: -50%;
      width: 200%;
      height: 200%;
      background: linear-gradient(45deg, rgba(255,255,255,0.1) 25%, transparent 25%, transparent 50%, rgba(255,255,255,0.1) 50%, rgba(255,255,255,0.1) 75%, transparent 75%, transparent);
      background-size: 40px 40px;
      animation: slideBg 20s linear infinite;
    }
    @keyframes slideBg { 0% { transform: translate(0,0); } 100% { transform: translate(40px, 40px); } }
    header h1 { position: relative; z-index: 1; font-size: 28px; }
    header p { position: relative; z-index: 1; opacity: 0.9; font-size: 12px; margin-top: 5px; }

    .container { max-width: 1400px; margin: 20px auto; padding: var(--pad); flex: 1; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: var(--pad); }

    .card {
      background: rgba(255,255,255,0.1);
      backdrop-filter: blur(10px);
      border: 1px solid rgba(255,255,255,0.2);
      border-radius: 15px;
      padding: var(--pad);
      box-shadow: 0 8px 32px rgba(0,0,0,0.1);
      transition: transform 0.3s, box-shadow 0.3s;
    }
    .card:hover { transform: translateY(-5px); box-shadow: 0 12px 40px rgba(0,0,0,0.2); }

    body.light-mode .card { background: rgba(0,0,0,0.05); }

    .metric {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin: 15px 0;
      padding: 12px;
      background: rgba(0,0,0,0.2);
      border-radius: 10px;
      border-left: 4px solid #667eea;
    }
    body.light-mode .metric { background: rgba(0,0,0,0.08); }

    .metric-label { font-size: 13px; opacity: 0.8; }
    .metric-value { font-size: 20px; font-weight: bold; }
    .metric-unit { font-size: 12px; opacity: 0.6; margin-left: 5px; }

    .status-badge {
      display: inline-block;
      padding: 6px 12px;
      border-radius: 20px;
      font-size: 12px;
      font-weight: bold;
      margin: 5px 5px 5px 0;
    }
    .status-ok { background: #4CAF50; color: white; }
    .status-warn { background: #FF9800; color: white; }
    .status-crit { background: #F44336; color: white; animation: pulse 1s infinite; }
    @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.7; } }

    .chart-container { position: relative; height: 300px; margin: 20px 0; }

    .control-buttons {
      display: flex;
      gap: 10px;
      margin: 20px 0;
      flex-wrap: wrap;
    }
    button {
      padding: 12px 24px;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      font-size: 14px;
      font-weight: bold;
      transition: all 0.3s;
      position: relative;
      overflow: hidden;
    }
    button:active { transform: scale(0.98); }
    .btn-start { background: #4CAF50; color: white; }
    .btn-start:hover { background: #45a049; box-shadow: 0 5px 15px rgba(76,175,80,0.4); }
    .btn-stop { background: #F44336; color: white; }
    .btn-stop:hover { background: #da190b; box-shadow: 0 5px 15px rgba(244,67,54,0.4); }
    .btn-reset { background: #2196F3; color: white; }
    .btn-reset:hover { background: #0b7dda; box-shadow: 0 5px 15px rgba(33,150,243,0.4); }
    button:disabled { opacity: 0.5; cursor: not-allowed; }

    .alert { display: none; position: fixed; top: 20px; right: 20px; background: #F44336; color: white; padding: 15px 25px; border-radius: 8px; z-index: 1000; animation: slideIn 0.3s; }
    @keyframes slideIn { from { transform: translateX(400px); } to { transform: translateX(0); } }
    .alert.show { display: block; }

    .relay-status {
      width: 30px;
      height: 30px;
      border-radius: 50%;
      display: inline-block;
      margin: 0 10px;
      animation: glow 2s infinite;
    }
    .relay-on { background: #4CAF50; box-shadow: 0 0 10px #4CAF50; }
    .relay-off { background: #999; }
    @keyframes glow { 0%, 100% { box-shadow: 0 0 5px currentColor; } 50% { box-shadow: 0 0 20px currentColor; } }

    .theme-toggle {
      position: fixed;
      bottom: 30px;
      right: 30px;
      width: 50px;
      height: 50px;
      border-radius: 50%;
      background: #667eea;
      color: white;
      border: none;
      cursor: pointer;
      font-size: 24px;
      box-shadow: 0 5px 20px rgba(102,126,234,0.4);
      z-index: 100;
      transition: all 0.3s;
    }
    .theme-toggle:hover { transform: scale(1.1); }

    footer {
      background: rgba(0,0,0,0.3);
      padding: var(--pad);
      text-align: center;
      font-size: 12px;
      opacity: 0.7;
      margin-top: auto;
    }

    @media (max-width: 768px) {
      body { --pad: 15px; }
      header h1 { font-size: 22px; }
      .grid { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
<header>
  <h1>⚙️ Motor Protection System v3.2</h1>
  <p>Real-time Monitoring | AI Anomaly Detection | Predictive Maintenance</p>
</header>

<div class="alert" id="alert">❌ Connection Error - Retrying...</div>

<div class="container">
  <div class="grid">
    <div class="card">
      <h2>🎯 System Status</h2>
      <div class="metric">
        <div>
          <div class="metric-label">Health Score</div>
          <div class="metric-value" id="healthScore">100<span class="metric-unit">%</span></div>
        </div>
        <div style="font-size: 40px;">💚</div>
      </div>
      <div class="metric">
        <div class="metric-label">Motor Status</div>
        <div style="display: flex; align-items: center;">
          <span class="relay-status" id="relayMainLed"></span>
          <span id="relayAllStatus">De-energized</span>
        </div>
      </div>
      <div class="metric">
        <div class="metric-label">Temperature</div>
        <div class="metric-value" id="tempValue">--<span class="metric-unit">°C</span></div>
      </div>
      <div class="metric">
        <div class="metric-label">ML Training Status</div>
        <div class="metric-value" id="mlStatus" style="color: #FF9800;">Training...</div>
      </div>
      <div class="control-buttons">
        <button class="btn-start" id="btnStart" onclick="motorStart()">▶️ Start</button>
        <button class="btn-stop" id="btnStop" onclick="motorStop()">⏹️ Stop</button>
        <button class="btn-reset" id="btnReset" onclick="motorReset()">↻ Reset</button>
      </div>
    </div>

    <div class="card">
      <h2>📊 Vibration (m/s²)</h2>
      <div class="metric">
        <div>
          <div class="metric-label">X-Axis</div>
          <div class="metric-value" id="vibX">0.00</div>
        </div>
        <span class="status-badge" id="statusX" style="background: #999;">OK</span>
      </div>
      <div class="metric">
        <div>
          <div class="metric-label">Y-Axis</div>
          <div class="metric-value" id="vibY">0.00</div>
        </div>
        <span class="status-badge" id="statusY" style="background: #999;">OK</span>
      </div>
      <div class="metric">
        <div>
          <div class="metric-label">Z-Axis</div>
          <div class="metric-value" id="vibZ">0.00</div>
        </div>
        <span class="status-badge" id="statusZ" style="background: #999;">OK</span>
      </div>
      <div style="margin-top: 10px; font-size: 12px; opacity: 0.7;">
        ⚠️ Warning: >5.0 m/s² | 🚨 Critical: >8.0 m/s²
      </div>
    </div>

    <div class="card">
      <h2>⚡ Current (A)</h2>
      <div class="metric">
        <div class="metric-label">Phase A</div>
        <div class="metric-value" id="currentA">0.00</div>
      </div>
      <div class="metric">
        <div class="metric-label">Phase B</div>
        <div class="metric-value" id="currentB">0.00</div>
      </div>
      <div class="metric">
        <div class="metric-label">Phase C</div>
        <div class="metric-value" id="currentC">0.00</div>
      </div>
      <div class="metric">
        <div class="metric-label">Average Current</div>
        <div class="metric-value" id="currentAvg">0.00</div>
      </div>
      <div class="metric">
        <div class="metric-label">Phase Imbalance</div>
        <div class="metric-value" id="imbalance">0.0%</div>
      </div>
    </div>

    <div class="card">
      <h2>🤖 ML Anomaly Detection</h2>
      <div class="metric">
        <div class="metric-label">Anomaly Score</div>
        <div class="metric-value" id="anomalyScore">0.00</div>
      </div>
      <div class="metric">
        <div class="metric-label">Status</div>
        <div id="anomalyStatus" class="status-badge status-ok">Normal</div>
      </div>
      <div style="margin-top: 15px; font-size: 12px; opacity: 0.7;">
        🎓 Baseline Samples: <span id="baselineSamples">0</span> / 200
      </div>
    </div>
  </div>

  <div style="margin-top: 30px; display: grid; grid-template-columns: repeat(auto-fit, minmax(500px, 1fr)); gap: var(--pad);">
    <div class="card">
      <h2>📈 Vibration Trends (Last 20 readings)</h2>
      <div class="chart-container">
        <canvas id="vibChart"></canvas>
      </div>
    </div>

    <div class="card">
      <h2>⚡ Current Trends (Last 20 readings)</h2>
      <div class="chart-container">
        <canvas id="currentChart"></canvas>
      </div>
    </div>

    <div class="card" style="grid-column: 1 / -1;">
      <h2>🚨 Anomaly Score & Threshold</h2>
      <div class="chart-container">
        <canvas id="anomalyChart"></canvas>
      </div>
    </div>
  </div>

  <div class="card" style="margin-top: 30px;">
    <h2>📋 System Information</h2>
    <div class="metric">
      <div class="metric-label">Timestamp</div>
      <div id="timestamp">--:--:--</div>
    </div>
    <div class="metric">
      <div class="metric-label">Data Count</div>
      <div id="dataCount">0</div>
    </div>
    <div class="metric">
      <div class="metric-label">MPU6050 Status</div>
      <span class="status-badge status-ok" id="mpuStatus">Connected</span>
    </div>
    <div class="metric">
      <div class="metric-label">DS18B20 Status</div>
      <span class="status-badge status-ok" id="tempStatus">Connected</span>
    </div>
  </div>
</div>

<button class="theme-toggle" onclick="toggleTheme()" title="Toggle Theme">🌙</button>

<footer>
  <p>Motor Protection System v3.2 | ESP32 + MPU6050 + SCT013 + DS18B20 | 🔒 Secured Dashboard</p>
  <p>© 2025 Industrial IoT Solutions | For Safety Critical Operations, Consult a Qualified Engineer</p>
</footer>

<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"></script>
<script>
let vibChart, currentChart, anomalyChart;

function createCharts() {
  const ctx1 = document.getElementById('vibChart').getContext('2d');
  vibChart = new Chart(ctx1, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        { label: 'X-Axis', data: [], borderColor: '#FF6B6B', tension: 0.4, fill: false },
        { label: 'Y-Axis', data: [], borderColor: '#4ECDC4', tension: 0.4, fill: false },
        { label: 'Z-Axis', data: [], borderColor: '#FFE66D', tension: 0.4, fill: false }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: { legend: { display: true, labels: { color: getThemeColor() } } },
      scales: { y: { beginAtZero: true, ticks: { color: getThemeColor() }, grid: { color: 'rgba(200,200,200,0.1)' } }, x: { ticks: { color: getThemeColor() }, grid: { color: 'rgba(200,200,200,0.1)' } } }
    }
  });

  const ctx2 = document.getElementById('currentChart').getContext('2d');
  currentChart = new Chart(ctx2, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        { label: 'Phase A', data: [], borderColor: '#FF6B6B', tension: 0.4, fill: false },
        { label: 'Phase B', data: [], borderColor: '#4ECDC4', tension: 0.4, fill: false },
        { label: 'Phase C', data: [], borderColor: '#FFE66D', tension: 0.4, fill: false }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: { legend: { display: true, labels: { color: getThemeColor() } } },
      scales: { y: { beginAtZero: true, ticks: { color: getThemeColor() }, grid: { color: 'rgba(200,200,200,0.1)' } }, x: { ticks: { color: getThemeColor() }, grid: { color: 'rgba(200,200,200,0.1)' } } }
    }
  });

  const ctx3 = document.getElementById('anomalyChart').getContext('2d');
  anomalyChart = new Chart(ctx3, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        { label: 'Anomaly Score', data: [], borderColor: '#FF6B6B', tension: 0.4, fill: false, pointRadius: 3 },
        { label: 'Threshold (3.0)', data: [], borderColor: '#F44336', tension: 0, fill: false, borderDash: [5, 5], pointRadius: 0 }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: { legend: { display: true, labels: { color: getThemeColor() } } },
      scales: { y: { beginAtZero: true, ticks: { color: getThemeColor() }, grid: { color: 'rgba(200,200,200,0.1)' } }, x: { ticks: { color: getThemeColor() }, grid: { color: 'rgba(200,200,200,0.1)' } } }
    }
  });
}

function updateStatus(id, status) {
  const badge = document.getElementById(id);
  if (status === 0) { badge.className = 'status-badge status-ok'; badge.textContent = 'OK'; }
  else if (status === 1) { badge.className = 'status-badge status-warn'; badge.textContent = 'WARNING'; }
  else { badge.className = 'status-badge status-crit'; badge.textContent = 'CRITICAL'; }
}

function getThemeColor() { return document.body.classList.contains('light-mode') ? '#333' : '#e0e0e0'; }

function updateChartTheme() {
  const color = getThemeColor();
  if (vibChart) {
    vibChart.options.plugins.legend.labels.color = color;
    vibChart.options.scales.y.ticks.color = color;
    vibChart.options.scales.x.ticks.color = color;
    vibChart.update();
  }
  if (currentChart) {
    currentChart.options.plugins.legend.labels.color = color;
    currentChart.options.scales.y.ticks.color = color;
    currentChart.options.scales.x.ticks.color = color;
    currentChart.update();
  }
  if (anomalyChart) {
    anomalyChart.options.plugins.legend.labels.color = color;
    anomalyChart.options.scales.y.ticks.color = color;
    anomalyChart.options.scales.x.ticks.color = color;
    anomalyChart.update();
  }
}

function toggleTheme() {
  document.body.classList.toggle('light-mode');
  updateChartTheme();
  localStorage.setItem('theme', document.body.classList.contains('light-mode') ? 'light' : 'dark');
}

function fetchData() {
  fetch('/data').then(r => r.json()).then(d => {
    document.getElementById('vibX').textContent = d.peakMagX.toFixed(2);
    document.getElementById('vibY').textContent = d.peakMagY.toFixed(2);
    document.getElementById('vibZ').textContent = d.peakMagZ.toFixed(2);

    document.getElementById('currentA').textContent = d.currentPhaseA.toFixed(2);
    document.getElementById('currentB').textContent = d.currentPhaseB.toFixed(2);
    document.getElementById('currentC').textContent = d.currentPhaseC.toFixed(2);
    document.getElementById('currentAvg').textContent = d.avgCurrent.toFixed(2);
    document.getElementById('imbalance').textContent = (d.currentImbalance * 100).toFixed(1) + '%';

    document.getElementById('tempValue').textContent = d.temperature.toFixed(1);
    document.getElementById('healthScore').textContent = d.healthScore.toFixed(0);
    document.getElementById('anomalyScore').textContent = d.anomalyScore.toFixed(2);

    if (d.anomalyScore > 3.0) {
      document.getElementById('anomalyStatus').className = 'status-badge status-crit';
      document.getElementById('anomalyStatus').textContent = 'ANOMALY';
    } else {
      document.getElementById('anomalyStatus').className = 'status-badge status-ok';
      document.getElementById('anomalyStatus').textContent = 'Normal';
    }

    document.getElementById('mlStatus').textContent = d.isTrained ? '✅ Trained' : '⏳ Training...';
    document.getElementById('baselineSamples').textContent = d.baselineCount;

    const relayState = d.relayA;
    document.getElementById('relayAllStatus').textContent = relayState ? 'Energized' : 'De-energized';
    document.getElementById('relayMainLed').className = relayState ? 'relay-status relay-on' : 'relay-status relay-off';

    updateStatus('statusX', d.statusX);
    updateStatus('statusY', d.statusY);
    updateStatus('statusZ', d.statusZ);

    const time = new Date().toLocaleTimeString();
    document.getElementById('timestamp').textContent = time;
    document.getElementById('dataCount').textContent = d.dataCount;

    vibChart.data.labels.push(time);
    vibChart.data.datasets[0].data.push(d.peakMagX);
    vibChart.data.datasets[1].data.push(d.peakMagY);
    vibChart.data.datasets[2].data.push(d.peakMagZ);
    if (vibChart.data.labels.length > 20) {
      vibChart.data.labels.shift();
      vibChart.data.datasets.forEach(ds => ds.data.shift());
    }
    vibChart.update('none');

    currentChart.data.labels.push(time);
    currentChart.data.datasets[0].data.push(d.currentPhaseA);
    currentChart.data.datasets[1].data.push(d.currentPhaseB);
    currentChart.data.datasets[2].data.push(d.currentPhaseC);
    if (currentChart.data.labels.length > 20) {
      currentChart.data.labels.shift();
      currentChart.data.datasets.forEach(ds => ds.data.shift());
    }
    currentChart.update('none');

    anomalyChart.data.labels.push(time);
    anomalyChart.data.datasets[0].data.push(d.anomalyScore);
    anomalyChart.data.datasets[1].data.push(3.0);
    if (anomalyChart.data.labels.length > 20) {
      anomalyChart.data.labels.shift();
      anomalyChart.data.datasets.forEach(ds => ds.data.shift());
    }
    anomalyChart.update('none');

    document.getElementById('mpuStatus').textContent = d.sensorOK ? '✅ Connected' : '❌ Disconnected';
    document.getElementById('tempStatus').textContent = d.tempSensorConnected ? '✅ Connected' : '❌ Disconnected';
  }).catch(e => {
    console.log(e);
    document.getElementById('alert').classList.add('show');
    document.getElementById('btnStart').disabled = true;
  });
}

function motorStart() { fetch('/start', { method: 'POST' }); }
function motorStop() { fetch('/stop', { method: 'POST' }); }
function motorReset() { fetch('/reset', { method: 'POST' }); }

if (localStorage.getItem('theme') === 'light') {
  document.body.classList.add('light-mode');
}

createCharts();
updateChartTheme();
setInterval(fetchData, 1000);
fetchData();
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"peakMagX\":" + String(vibData.peakMagX, 3) + ",";
  json += "\"peakMagY\":" + String(vibData.peakMagY, 3) + ",";
  json += "\"peakMagZ\":" + String(vibData.peakMagZ, 3) + ",";
  json += "\"statusX\":" + String(vibData.statusX) + ",";
  json += "\"statusY\":" + String(vibData.statusY) + ",";
  json += "\"statusZ\":" + String(vibData.statusZ) + ",";
  json += "\"currentPhaseA\":" + String(currentData.currentPhaseA, 2) + ",";
  json += "\"currentPhaseB\":" + String(currentData.currentPhaseB, 2) + ",";
  json += "\"currentPhaseC\":" + String(currentData.currentPhaseC, 2) + ",";
  json += "\"avgCurrent\":" + String(currentData.avgCurrent, 2) + ",";
  json += "\"currentImbalance\":" + String(currentData.currentImbalance, 3) + ",";
  json += "\"temperature\":" + String(temperature, 1) + ",";
  json += "\"healthScore\":" + String(healthScore, 1) + ",";
  json += "\"anomalyScore\":" + String(mlModel.anomalyScore, 2) + ",";

  // Fixed: Proper String concatenation for boolean values
  json += "\"isTrained\":" + String(mlModel.isTrained ? "true" : "false") + ",";
  json += "\"baselineCount\":" + String(mlModel.baselineCount) + ",";
  json += "\"relayA\":" + String(currentData.relayA ? "true" : "false") + ",";
  json += "\"relayB\":" + String(currentData.relayB ? "true" : "false") + ",";
  json += "\"relayC\":" + String(currentData.relayC ? "true" : "false") + ",";
  json += "\"motorRunning\":" + String(motorRunning ? "true" : "false") + ",";
  json += "\"sensorOK\":" + String(currentData.sensorOK ? "true" : "false") + ",";
  json += "\"tempSensorConnected\":" + String(tempSensorConnected ? "true" : "false") + ",";
  json += "\"emergencyStop\":" + String(emergencyStop ? "true" : "false") + ",";
  json += "\"dataCount\":" + String(dataCount);
  json += "}";
  server.send(200, "application/json", json);
}

void handleStart() {
  if (!emergencyStop && !criticalShutdown) {
    motorRunning = true;
    updateRelay();
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("✅ Motor Started");
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Cannot start - emergency stop or critical condition");
  }
}

void handleStop() {
  motorRunning = false;
  updateRelay();
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("🛑 Motor Stopped");
  server.send(200, "text/plain", "OK");
}

void handleReset() {
  emergencyStop = false;
  criticalShutdown = false;
  motorRunning = false;
  updateRelay();
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("↻ System Reset");
  server.send(200, "text/plain", "OK");
}

void loadSettings() {
  Serial.println("📁 Loading settings from EEPROM...");
}

void saveSettings() {
  Serial.println("💾 Saving settings to EEPROM...");
}
