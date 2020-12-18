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
#include <LittleFS.h>
#include "config.h"
#include "station.h"

IR_Station *station;
volatile bool reset_flag = false;

void ICACHE_RAM_ATTR rst_isr() {
  reset_flag = true;
}

void setup() {
  // prepare serial debug
  Serial.begin(74880);
  delay(10);
  println_dbg("");
  println_dbg("Hello, I'm ESP-WROOM-02");

  // prepare internal filesystem
  LittleFS.begin();

  // IR-station setup
  station = new IR_Station(PIN_IR_OUT, PIN_IR_IN, PIN_LED_R, PIN_LED_G, PIN_LED_B);
  station->begin();

  // hardware button reset setup
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(PIN_BUTTON, rst_isr, RISING);

  // Setup Completed
  println_dbg("Setup Completed");
}

void loop() {
  station->handle();
  if (reset_flag) station->reset();
}
