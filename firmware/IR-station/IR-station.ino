/*
   IR-station Ver.1.0.0
   Infrared Remote Controller with ESP8266 WiFi Module

   Author:  kerikun11 (Github: kerikun11)
   Date:    2016.01.22

   1. Add ESP8266 Board to Arduino IDE in Preferences.
      Put URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
   2. Tool -> Board Settings:
          Board:           ESPDuino (ESP-13 Module)
          Upload Using:    Serial
          CPU Frequency:   80MHz
          Flash Size:      4M (3M SPIFFS)
          Upload Speed:    115200
   3. Upload the program to the ESP8266 WiFi Module.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "config.h"
#include "IR-lib.h"
#include "IR_op.h"
#include "WiFi_op.h"
#include "server_op.h"

void setup() {
  ESP.wdtFeed();
  // Prepare Serial debug
  Serial.begin(115200);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02");

  // prepare GPIO
  pinMode(Indicate_LED, OUTPUT);
  pinMode(ERROR_LED, OUTPUT);
  pinMode(IR_IN, INPUT);
  pinMode(IR_OUT, OUTPUT);

  digitalWrite(IR_IN, LOW);
  digitalWrite(Indicate_LED, LOW);
  digitalWrite(ERROR_LED, LOW);

  // Setup Start
  digitalWrite(ERROR_LED, HIGH);

  // Prepare SPIFFS
  SPIFFS.begin();

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
  server.handleClient();
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ERROR_LED, HIGH);
  } else {
    digitalWrite(ERROR_LED, LOW);
  }
}

