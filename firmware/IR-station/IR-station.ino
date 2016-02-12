/*
   IR-station Ver.1.0.0
   Infrared Remote Controller with ESP8266 WiFi Module

   Author:  kerikun11
   Date:    2016.01.22

   Add ESP8266 Board URL:http://arduino.esp8266.com/stable/package_esp8266com_index.json
   Board Settings
       Board:           Generic ESP8266 Module
       Flash Mode:      QIO
       Flash Frequency: 40MHz
       Upload Using:    Serial
       CPU Frequency:   80MHz/160MHz
       Flash Size:      4M(3M SPIFFS)
       Reset Method:    ck
       Upload Speed:    115200
*/

#include <ESP8266WiFi.h>
#include <FS.h>
#include "config.h"
#include "IR-lib.h"
#include "IR_op.h"
#include "WiFi_op.h"
#include "String_op.h"
#include "server_op.h"

void setup() {
  ESP.wdtFeed();
  // Prepare Serial debug
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02");

  // prepare GPIO
  pinMode(Indicate_LED, OUTPUT);
  pinMode(ERROR_LED, OUTPUT);
  pinMode(IR_IN, INPUT);
  pinMode(IR_OUT, OUTPUT);

  digitalWrite(Indicate_LED, LOW);
  digitalWrite(ERROR_LED, LOW);

  // Setup Start
  digitalWrite(ERROR_LED, HIGH);

  // Prepare SPIFFS
  bool res = SPIFFS.begin();
  if (!res) println_dbg("SPIFFS.begin fail");

  // Restore reserved data
  wifiRestoreFromFile();
  irDataRestoreFromFile();

  // WiFi setup
  wifiSetup();

  // WebServer Setup
  setupServer();

  // Setup Completed
  digitalWrite(ERROR_LED, LOW);
  println_dbg("Setup Completed");
}

void loop() {
  ESP.wdtFeed();
  getClient();
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ERROR_LED, HIGH);
    connectWifi();
    digitalWrite(ERROR_LED, LOW);
  }
  delay(100);
}

