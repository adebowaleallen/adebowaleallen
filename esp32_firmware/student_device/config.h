/*
 * Configuration File for Student Response System
 *
 * IMPORTANT: Update these settings before uploading to your ESP32
 */

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Credentials
#define WIFI_SSID "YourWiFiNetworkName"      // Replace with your WiFi SSID
#define WIFI_PASSWORD "YourWiFiPassword"     // Replace with your WiFi password

// Server Configuration
#define SERVER_URL "http://192.168.1.100:5000"  // Replace with your server IP and port
                                                 // Example: "http://192.168.1.100:5000"
                                                 // For local testing: "http://192.168.1.100:5000"
                                                 // For cloud: "https://yourserver.com"

// Student Identification
#define STUDENT_ID "STUDENT_001"  // Unique ID for each student device
                                  // Examples: "STUDENT_001", "STUDENT_002", etc.
                                  // Or use names: "JOHN_DOE", "JANE_SMITH", etc.

/*
 * SETUP INSTRUCTIONS:
 *
 * 1. Update WIFI_SSID and WIFI_PASSWORD with your WiFi credentials
 * 2. Update SERVER_URL with your server's IP address
 *    - To find your server IP: Run 'ipconfig' (Windows) or 'ifconfig' (Mac/Linux)
 *    - Make sure the ESP32 can reach this IP address (same network)
 * 3. Set a unique STUDENT_ID for each device
 * 4. Upload the sketch to your ESP32
 * 5. Open Serial Monitor (115200 baud) to see connection status
 */

#endif // CONFIG_H
