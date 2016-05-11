#include "OTA_op.h"

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

void setupOTA() {
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname((const char *)OTA_HOSTNAME);

  // No authentication by default
  //ArduinoOTA.setPassword((const char *)OTA_PASSWORD);

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
  println_dbg("OTA Ready");
}

void OTATask() {
  // handle OTA update
  ArduinoOTA.handle();
}

