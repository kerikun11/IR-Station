/*
   IR-station
   Infrared Remote Controller with ESP8266 WiFi Module

   Author:  kerikun11 (Github: kerikun11)
   Date:    2016.07.23
*/

#include <ESP8266WiFi.h>
#include <FS.h>
#include "config.h"
#include "station.h"
#include "ota.h"

OTA ota;
IR_Station station(PIN_IR_OUT, PIN_IR_IN, PIN_LED_R, PIN_LED_G, PIN_LED_B, PIN_BUTTON);

void setup() {
  // Prepare Serial debug
  Serial.begin(115200);
  delay(10);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02");

  // prepare SPIFFS
  SPIFFS.begin();

  // setup OTA
  ota.begin(HOSTNAME_DEFAULT);

  // IR-station setup
  station.begin();

  // Setup Completed
  println_dbg("Setup Completed");
}

void loop() {
  ota.handle();
  station.handle();
}

