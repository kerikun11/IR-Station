#ifndef WIFI_OPERATION
#define WIFI_OPERATION

#include <ESP8266WiFi.h>

void setupAP(void);

bool connectWifi(String target_ssid, String target_pass);

#endif

