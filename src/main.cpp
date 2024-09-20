#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// #define STASSID "SafeHouse"
// #define STAPSK  "Rg@2k24+-*/"
#define STASSID "Nothing"
#define STAPSK  "Goel@2462"
const char* serverName = "http://192.168.0.238:3000/api/setCurrent";


#define RELAY 0 // Pin D27 on ESP32 for relay
#define IREAD A0  // Pin D34 on ESP32 for reading the leakage current

String poleid = "PID1"; // Pole ID to modify

const char* ssid = STASSID;
const char* password = STAPSK;
WiFiClient client;


// Define input pins and their corresponding PIDs
const int numPins = 1;

void setup() {
  Serial.begin(9600);
  digitalWrite(RELAY, 0);
  pinMode(RELAY, OUTPUT);

  // Initialize input pins
  pinMode(IREAD, INPUT);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
    delay(5000);
  }

  Serial.println("WiFi connected.");
}

void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    int adcValue = analogRead(IREAD);
    // Convert ADC value to voltage (assuming 12-bit ADC and 3.3V reference)
    float voltage = (adcValue * 1.0) / 1023.0;
    if (voltage<3){
      Serial.println("HELPPP");
      digitalWrite(RELAY, 1);
    }
    
    // Prepare JSON payload
    StaticJsonDocument<200> doc;
    doc["pid"] = poleid;
    doc["leakage"] = voltage;  // Sending voltage as leakage value

    String jsonString;
    serializeJson(doc, jsonString);

    // Send HTTP POST request
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0) {
      // String response = http.getString();
      // Serial.println("Response: " + response);
      Serial.println("Success, voltage written: " + String(voltage));
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
      Serial.println("Failed to write voltage: " + String(voltage));
    }

    http.end();
    delay(100);  // Small delay between requests
  } else {
    Serial.println("WiFi Disconnected");
  }
}
