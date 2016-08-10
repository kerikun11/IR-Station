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
#include "httpServer.h"
#include "led.h"
#include "wifi.h"

void setup() {
  // Prepare Serial debug
  Serial.begin(115200);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02");

  // prepare GPIO
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // IR-station setup
  station.begin();

  // Setup Completed
  println_dbg("Setup Completed");
}

void loop() {
  OTATask();
  serverTask();
  ir.handle();

  switch (station.mode) {
    case IR_STATION_MODE_NULL:
      if ((WiFi.status() == WL_CONNECTED)) {
        indicator.set(0, 0, 1023);
      }
      break;
    case IR_STATION_MODE_STA:
      static bool lost = false;
      if ((WiFi.status() != WL_CONNECTED)) {
        if (lost == false) {
          println_dbg("Lost WiFi: " + station.ssid);
          WiFi.mode(WIFI_AP_STA);
          delay(1000);
          setupAP(SOFTAP_SSID, SOFTAP_PASS);
          indicator.set(1023, 0, 0);
        }
        lost = true;
      } else {
        if (lost == true) {
          println_dbg("Found WiFi: " + station.ssid);
          WiFi.mode(WIFI_STA);
          indicator.set(0, 0, 1023);
        }
        lost = false;
      }
      break;
    case IR_STATION_MODE_AP:
      break;
  }
}

