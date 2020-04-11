/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "config.h" // for print_dbg()
#include "ota.h"

#if USE_OTA_UPDATE == true
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#endif

void OTA::begin(String hostname, String password, int port) {
#if USE_OTA_UPDATE == true
  yield();

  // Port defaults to 8266
  ArduinoOTA.setPort(port);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(hostname.c_str());

  // No authentication by default
  //  ArduinoOTA.setPassword(password.c_str());

  ArduinoOTA.onStart([]() {
    println_dbg("Start");
  });
  ArduinoOTA.onEnd([]() {
    println_dbg("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    printf_dbg("Progress: %u%%\r\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    printf_dbg("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) println_dbg("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) println_dbg("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) println_dbg("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) println_dbg("Receive Failed");
    else if (error == OTA_END_ERROR) println_dbg("End Failed");
  });
  ArduinoOTA.begin();
  println_dbg("OTA is Ready");
#endif
}

void OTA::handle() {
#if USE_OTA_UPDATE == true
  // handle OTA update
  ArduinoOTA.handle();
#endif
}
