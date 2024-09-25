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
const int DELAY = 5000;
const float THRESHOLD = 2.5;


// Define input pins and their corresponding PIDs
const int numPins = 1;
bool relay = true;
unsigned long mytimer;

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
            //flash builtin led twice quickly with loop
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
        for(int i=0;i<2;i++)
        {
              digitalWrite(LED_BUILTIN, HIGH);
              delay(100);
              digitalWrite(LED_BUILTIN, LOW);
              delay(40);
        }
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
    // Convert ADC value to voltage (assuming 12-bit ADC and 3.3V reference)
    
    // Prepare JSON payload

    StaticJsonDocument<200> doc;
    doc["pid"] = poleid;
    doc["leakage"] = voltage;  // Sending voltage as leakage value
    if (voltage>THRESHOLD) {
      doc["critical"] = true;
      Serial.println("Critical");
    }
    else {
      doc["critical"] = false;
    }

    String jsonString;
    serializeJson(doc, jsonString);

    // Send HTTP POST request
    http.begin(nextApi);
    // http.addHeader("Content-Type", "application/json");


    int httpResponseCode = http.POST(jsonString);


    if (httpResponseCode > 0) {
      // String response = http.getString();
      // Serial.println("Response: " + response);
      Serial.println("Success, voltage written: " + String(voltage));
      digitalWrite(LED_BUILTIN, HIGH);
      delay(30);
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
      Serial.println("Failed to write voltage: " + String(voltage));
    }

    http.end();
}


/*
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
  // http.addHeader("Content-Type", "application/json");


  // Send PATCH request
  const int httpResponseCode = http.PATCH(jsonString);

  // Handle the response
  if (httpResponseCode > 0) {
    // Serial.println("PATCH request sent successfully.");
    Serial.println("Success: " + String(voltage));
    // Optionally, read the response payload
    // String response = http.getString();
    // Serial.println("Response: " + response);
  } else {
    // Serial.println("Error on sending PATCH: " + String(httpResponseCode));
    Serial.println("Failure: " + String(voltage));
  }

  // End the HTTP connection
  http.end();
}
*/

void setup() {
  Serial.begin(9600);
  digitalWrite(RELAY, 0);
  pinMode(RELAY, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

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

      
    // writeToDBdirectly(poleRecordid, voltage);

    if (voltage>THRESHOLD && relay) {
      Serial.println("HELPPP");
      if(relay){
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


