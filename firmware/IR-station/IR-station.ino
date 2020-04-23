/*
  IR-station
  Infrared Remote Controller with ESP8266 WiFi Module

  Author:  kerikun11 (Github: kerikun11)
  Date:    2016.07.23

  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <ESP8266WiFi.h>
#include <FS.h>

#include "config.h"
#include "station.h"
#include "ota.h"

IR_Station *station;
volatile bool btn_fall = false;
uint32_t btn_time = 0;

void ICACHE_RAM_ATTR trap_change() {
  btn_fall = (digitalRead(PIN_BUTTON)==LOW);
}

OTA ota;

void setup() {
  // prepare serial debug
  Serial.begin(74880);
  delay(10);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02");

  // prepare SPIFFS
  SPIFFS.begin();

  // IR-station setup
  station = new IR_Station(PIN_IR_OUT, PIN_IR_IN, PIN_LED_R, PIN_LED_G, PIN_LED_B);
  station->begin();

  // hardware button reset setup
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(PIN_BUTTON, trap_change,  CHANGE);

  ota.begin();

  // Setup Completed
  println_dbg("Setup Completed");
}

void loop() {
  station->handle();

  if (btn_fall) {
    if (btn_time == 0)
      btn_time = millis();
    else {
      if (millis() - btn_time > 3000)
        station->reset();
    }
  } else {
    if (btn_time != 0) {
      if (millis() - btn_time < 500)
        ESP.reset();
      btn_time = 0;
    }
  }

  ota.handle();
}
