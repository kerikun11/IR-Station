/*
   IR-station Ver.1.0.0
   Infrared Remote Controller with ESP8266 WiFi Module

   Author:  kerikun11 (Github: kerikun11)
   Date:    2016.01.22
*/

#include <ESP8266WiFi.h>
#include <FS.h>
#include "config.h"
#include "IR_op.h"
#include "OTA_op.h"
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
  irDataRestoreFromFile();
  settingsRestoreFromFile();

  // WiFi setup
  modeSetup();

  // OTA setup
  setupOTA();

  // WebServer Setup
  setupServer();

  // Setup Completed
  digitalWrite(ERROR_LED, LOW);
  settingsBackupToFile();
  println_dbg("Setup Completed");
}

void loop() {
  OTATask();
  serverTask();
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ERROR_LED, HIGH);
  } else {
    digitalWrite(ERROR_LED, LOW);
  }
}

