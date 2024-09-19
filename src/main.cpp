#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// #define STASSID "SafeHouse"
// #define STAPSK  "Rg@2k24+-*/"
#define STASSID "Nothing"
#define STAPSK  "Goel@2462"

#define relay 27
const char* ssid = STASSID;
const char* password = STAPSK;

const char* serverName = "http://192.168.0.238:3000/api/setCurrent";

// Define input pins and their corresponding PIDs
const int numPins = 1;
const int inputPins[numPins] = {34};  // Analog-capable pins on ESP32
const char* pids[numPins] = {"PID1"};

void setup() {
  Serial.begin(9600);
  digitalWrite(relay, 0);
  pinMode(relay, OUTPUT);

  // Initialize input pins
  for (int i = 0; i < numPins; i++) {
    pinMode(inputPins[i], INPUT);
  }

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
    
    for (int i = 0; i < numPins; i++) {
      // Read analog value from the input pin
      int adcValue = analogRead(inputPins[i]);
      Serial.println(adcValue);
      // Convert ADC value to voltage (assuming 12-bit ADC and 3.3V reference)
      float voltage = (adcValue * 3.3) / 4095.0;
      Serial.println(voltage);
      if (voltage<3){
        Serial.println("HELPPP");
        digitalWrite(relay, 1);
      }
      
      // Prepare JSON payload
      StaticJsonDocument<200> doc;
      doc["pid"] = pids[i];
      doc["leakage"] = voltage;  // Sending voltage as leakage value
      


      String jsonString;
      serializeJson(doc, jsonString);

      // Send HTTP POST request
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(jsonString);

      if (httpResponseCode > 0) {
        String response = http.getString();
        // Serial.println("Response: " + response);
      } else {
        Serial.println("Error on sending POST: " + String(httpResponseCode));
      }

      http.end();
      delay(100);  // Small delay between requests
    }
  } else {
    Serial.println("WiFi Disconnected");
  }

}
