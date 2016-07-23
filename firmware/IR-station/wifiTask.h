#ifndef WIFI_OPERATION
#define WIFI_OPERATION

#include <ESP8266WiFi.h>

void setupAP(String ssid, String password);

bool connectWifi(String ssid, String pass);

#endif

