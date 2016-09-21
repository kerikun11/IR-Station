#include "ota.h"

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "config.h" // for print_dbg()

void OTA::begin(String hostname, String password, int port) {
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
}

void OTA::handle() {
  // handle OTA update
  ArduinoOTA.handle();
}

