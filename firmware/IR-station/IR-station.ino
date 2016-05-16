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
#include "LCD_op.h"
#include "OTA_op.h"
#include "WiFi_op.h"
#include "server_op.h"

void setup() {
  // Prepare Serial debug
  Serial.begin(115200);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02");

  // prepare GPIO
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_IR_IN, INPUT);
  pinMode(PIN_IR_OUT, OUTPUT);

  //digitalWrite(PIN_IR_OUT, LOW);
  digitalWrite(PIN_LED1, LOW);

  // Setup Start
  digitalWrite(PIN_LED1, HIGH);

  // Prepare SPIFFS
  SPIFFS.begin();

  // Restore reserved data
  irDataRestoreFromFile();
  settingsRestoreFromFile();

  // LCD setup
  setupLcd();

  // WiFi setup
  modeSetup();

  // OTA setup
  setupOTA();

  // WebServer Setup
  setupServer();

  // Setup Completed
  digitalWrite(PIN_LED1, LOW);
  settingsBackupToFile();
  println_dbg("Setup Completed");
}

void loop() {
  OTATask();
  serverTask();
  lcdTask();
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(PIN_LED1, HIGH);
  } else {
    digitalWrite(PIN_LED1, LOW);
  }
}

