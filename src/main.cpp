#include <Arduino.h> // Comment this out if you are using Arduino IDE
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const String nextApi = "http://192.168.0.238:3000/api/setCurrent";
const String directDBapi = "http://192.168.0.238:8090/api/collections/Poles/records/";

#define RELAY 27 // Pin D27 on ESP32 for relay
#define IREAD 34  // Pin D34 on ESP32 for reading the leakage current
#define LED_BUILTIN 2 // Pin D2 on ESP32 for builtin led

const String poleid = "PID1"; // Pole ID to modify
const String poleRecordid = "vkeyhhpgwlrlzix";

const int numPins = 1;
bool relay = true;

// void connecctToWiFi();
void connectToWiFi() {
  const char* known_ssids[] = {
    "Macle",
    "Nothing",
    "SafeHouse"
  };

  const char* known_passwords[] = {
    "Jolty@2462",
    "Goel@2462",
    "Rg@2k24+-*/"
  };

  const int num_known_networks = sizeof(known_ssids) / sizeof(known_ssids[0]);

  // Scan for available networks
  int n = WiFi.scanNetworks();
  
  if (n != 0) {
    // Attempt to connect to known networks
    bool connected = false;
    for (int i = 0; i < num_known_networks; ++i) {
      for (int j = 0; j < n; ++j) {
        if (WiFi.SSID(j) == known_ssids[i]) {

          WiFi.begin(known_ssids[i], known_passwords[i]);

          // Wait for connection
          int attempts = 0;
          while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            delay(1000);
            attempts++;
          }

          if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            // Flash builtin led twice quickly with loop
            for(int i=0;i<2;i++) {
              digitalWrite(LED_BUILTIN, HIGH);
              delay(100);
              digitalWrite(LED_BUILTIN, LOW);
              delay(40);
            }
            break;  // Exit the loop if connected
          } else {
            WiFi.disconnect();
          }
        }
      }
      if (connected) {
        break;  // Exit outer loop if connected
      }
    }
  }
}

void writeToNextAPI(String poleid, float voltage) {
  HTTPClient http;

  // Prepare JSON payload
  StaticJsonDocument<200> doc;
  doc["pid"] = poleid;
  doc["leakage"] = voltage;  // Sending voltage as leakage value

  String jsonString;
  serializeJson(doc, jsonString);

  // Send HTTP POST request
  http.begin(nextApi);
  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(30);
    digitalWrite(LED_BUILTIN, LOW);
  }

  http.end();
}

void writeToDBdirectly(String poleRecordid ,float voltage) {
  HTTPClient http;
  // Prepare JSON payload
  StaticJsonDocument<200> doc;
  doc["leakage"] = voltage;  // Creating {"leakage": 8.00}

  String jsonString;
  serializeJson(doc, jsonString);

  // Define the PATCH endpoint URL
  const String patchUrl = directDBapi + poleRecordid;

  // Begin HTTP connection
  http.begin(patchUrl);

  // Send PATCH request
  const int httpResponseCode = http.PATCH(jsonString);

  http.end();
}

void setup() {
  digitalWrite(RELAY, 0);
  pinMode(RELAY, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Initialize input pins
  pinMode(IREAD, INPUT);

  // Connect to Wi-Fi
  connectToWiFi();
}

void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() == WL_CONNECTED) {
    int time = millis();

    // Read ADC value
    int adcValue = analogRead(IREAD);
    float voltage = (adcValue * 3.3) / 4095.0;

    writeToNextAPI(poleid, voltage);

    if (voltage < 3 && relay) {
      digitalWrite(RELAY, 1);
      relay = false;
    }

    int timetaken = millis() - time;
    if (timetaken < 950) {
      delay(950 - timetaken);
    }

  }
}
