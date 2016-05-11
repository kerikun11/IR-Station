#ifndef WIFI_OPERATION
#define WIFI_OPERATION

#include <ESP8266WiFi.h>

extern String target_ssid;
extern String target_pass;

void setupAP(void);

bool connectCachedWifi();
bool connectWifi();

#endif

