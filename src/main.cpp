#include <Arduino.h> // Comment this out if you are using Arduino IDE
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const String nextApi = "http://192.168.0.238:3000/api/setCurrent";
const String directDBapi = "http://192.168.0.238:8090/api/collections/Poles/records/";

#define RELAY 4 // Pin D1 on ESP8266 for relay
#define IREAD 5  // Pin D2 on ESP8266 for reading the leakage current
#define LED_BUILTIN 16 // Pin D0 on ESP8266 for builtin led

#define LED_ACTIVE_LOW true // Set to 'false' if the built-in LED is active HIGH  eso-32 will have it false

void ledOn() {
  if (LED_ACTIVE_LOW) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void ledOff() {
  if (LED_ACTIVE_LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

const String poleid = "PID1"; // Pole ID to modify
const String poleRecordid = "vkeyhhpgwlrlzix";
const int DELAY = 5000;
const float THRESHOLD = 2.5;
WiFiClient client; // Initialize WiFi client for HTTP requests

// Define input pins and their corresponding PIDs
const int numPins = 1;
bool relay = true;
unsigned long mytimer;

// void connectToWiFi();
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

  Serial.println("Scanning for WiFi networks...");

  // Scan for available networks
  int n = WiFi.scanNetworks();
  Serial.println("Scan complete.");
  
  if (n == 0) {
    Serial.println("No networks found.");
  } else {
    Serial.print(n);
    Serial.println(" networks found:");

    // List all detected networks
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (Signal Strength: ");
      Serial.print(WiFi.RSSI(i));
      Serial.println(" dBm)");
    }

    // Attempt to connect to known networks
    bool connected = false;
    WiFi.mode(WIFI_STA);
    for (int i = 0; i < num_known_networks; ++i) {
      for (int j = 0; j < n; ++j) {
        if (WiFi.SSID(j) == known_ssids[i]) {
          Serial.print("Attempting to connect to ");
          Serial.println(known_ssids[i]);

          WiFi.begin(known_ssids[i], known_passwords[i]);

          // Wait for connection
          int attempts = 0;
          while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            delay(1000);
            Serial.print(".");
            attempts++;
          }

          if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Connected successfully!");
            Serial.print("Connected to: ");
            Serial.println(known_ssids[i]);
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            connected = true;
            // Flash built-in LED twice quickly with loop
            for(int k = 0; k < 2; k++) {
              ledOn();
              delay(100);
              ledOff();
              delay(40);
            }
            break;  // Exit the loop if connected
          } else {
            Serial.println();
            Serial.print("Failed to connect to ");
            Serial.println(known_ssids[i]);
            WiFi.disconnect();
          }
        }
      }
      if (connected) {
        break;  // Exit outer loop if connected
      }
    }

    if (!connected) {
      Serial.println("Could not connect to any known networks.");
    }
  }
}

void writeToNextAPI(String poleid, float voltage) {
  HTTPClient http;
  // Prepare JSON payload
  StaticJsonDocument<200> doc;
  doc["pid"] = poleid;
  doc["leakage"] = voltage;  // Sending voltage as leakage value
  if (voltage > THRESHOLD) {
    doc["critical"] = true;
    Serial.println("Critical");
  } else {
    doc["critical"] = false;
  }

  String jsonString;
  serializeJson(doc, jsonString);

  // Send HTTP POST request
  http.begin(client, nextApi);

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    Serial.println("Success, voltage written: " + String(voltage));
    ledOn();
    delay(30);
    ledOff();
  } else {
    Serial.println("Error on sending POST: " + String(httpResponseCode));
    Serial.println("Failed to write voltage: " + String(voltage));
  }

  http.end();
}

void setup() {
  Serial.begin(9600);
  digitalWrite(RELAY, 0);
  pinMode(RELAY, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  ledOff(); // Ensure LED is off at the start

  // Initialize input pins
  pinMode(IREAD, INPUT);

  // Connect to Wi-Fi
  connectToWiFi();
  mytimer = millis();
}

void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() == WL_CONNECTED) {
    // Read ADC value
    int adcValue = analogRead(IREAD);
    float voltage = (((float)adcValue) * 3.0) / 4095.0;
    voltage = map(voltage, 0.0, 3.0, 3.0, 0.0);

    if (voltage > THRESHOLD && relay) {
      Serial.println("HELPPP");
      if (relay) {
        relay = false;
        mytimer = millis(); // Reset timer
        writeToNextAPI(poleid, voltage);
        digitalWrite(RELAY, 1);
      }
    }

    if ((millis() - mytimer) > DELAY) {
      writeToNextAPI(poleid, voltage);
      mytimer = millis();
    }

  } else {
    Serial.println("WiFi Disconnected");
  }
}
