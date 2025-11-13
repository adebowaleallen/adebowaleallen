/*
 * Student Response Button System - ESP32 Firmware
 *
 * This firmware handles button inputs from students and sends
 * responses to the web server via WiFi.
 *
 * Buttons: YES, NO, A, B, C, D
 *
 * Author: @adebowaleallen
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Button GPIO Pins (adjust based on your wiring)
#define BTN_YES   13
#define BTN_NO    12
#define BTN_A     14
#define BTN_B     27
#define BTN_C     26
#define BTN_D     25

// LED Pin for status indication
#define LED_PIN   2  // Built-in LED on most ESP32 boards

// Button state tracking
struct Button {
  int pin;
  String label;
  bool lastState;
  unsigned long lastDebounceTime;
};

Button buttons[] = {
  {BTN_YES, "YES", HIGH, 0},
  {BTN_NO, "NO", HIGH, 0},
  {BTN_A, "A", HIGH, 0},
  {BTN_B, "B", HIGH, 0},
  {BTN_C, "C", HIGH, 0},
  {BTN_D, "D", HIGH, 0}
};

const int numButtons = 6;
const unsigned long debounceDelay = 50; // milliseconds

// WiFi and Server Configuration
String serverURL = SERVER_URL;
String studentID = STUDENT_ID;

// Connection status
bool wifiConnected = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // 5 seconds

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== Student Response System ===");
  Serial.println("Student ID: " + studentID);

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize buttons with internal pull-up resistors
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    buttons[i].lastState = HIGH;
  }

  // Connect to WiFi
  connectToWiFi();

  Serial.println("System ready. Press any button to respond.");
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    digitalWrite(LED_PIN, LOW);

    // Attempt reconnection every 5 seconds
    if (millis() - lastReconnectAttempt > reconnectInterval) {
      lastReconnectAttempt = millis();
      Serial.println("WiFi disconnected. Reconnecting...");
      connectToWiFi();
    }
  } else {
    wifiConnected = true;
    digitalWrite(LED_PIN, HIGH); // LED on when connected
  }

  // Check each button
  for (int i = 0; i < numButtons; i++) {
    checkButton(buttons[i]);
  }

  delay(10); // Small delay to prevent excessive CPU usage
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;

    // Blink LED to indicate successful connection
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("\nWiFi Connection Failed!");
    wifiConnected = false;
  }
}

void checkButton(Button &btn) {
  int reading = digitalRead(btn.pin);

  // Button pressed (LOW because of pull-up resistor)
  if (reading == LOW && btn.lastState == HIGH) {
    unsigned long currentTime = millis();

    // Debounce check
    if (currentTime - btn.lastDebounceTime > debounceDelay) {
      btn.lastDebounceTime = currentTime;

      Serial.println("\n--- Button Pressed: " + btn.label + " ---");

      // Send response to server
      if (wifiConnected) {
        sendResponse(btn.label);
      } else {
        Serial.println("ERROR: No WiFi connection. Response not sent.");
        // Blink LED rapidly to indicate error
        for (int i = 0; i < 5; i++) {
          digitalWrite(LED_PIN, LOW);
          delay(100);
          digitalWrite(LED_PIN, HIGH);
          delay(100);
        }
      }
    }
  }

  btn.lastState = reading;
}

void sendResponse(String response) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send response: WiFi not connected");
    return;
  }

  HTTPClient http;
  String endpoint = serverURL + "/api/responses";

  Serial.println("Sending to: " + endpoint);

  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload
  StaticJsonDocument<200> doc;
  doc["student_id"] = studentID;
  doc["response"] = response;
  doc["timestamp"] = millis();

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  Serial.println("Payload: " + jsonPayload);

  // Send POST request
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Server Response Code: " + String(httpResponseCode));
    Serial.println("Server Response: " + response);

    // Quick blink to confirm successful send
    digitalWrite(LED_PIN, LOW);
    delay(50);
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.print("Error sending response. HTTP Code: ");
    Serial.println(httpResponseCode);
    Serial.println("Error: " + http.errorToString(httpResponseCode));
  }

  http.end();
}
