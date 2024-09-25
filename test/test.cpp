//blink inbuild led

#include <Arduino.h>

#define LED_BUILTIN 16

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("On");
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Off");
  delay(100);
}