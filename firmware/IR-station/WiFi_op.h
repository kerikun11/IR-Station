#ifndef WIFI_OPERATION
#define WIFI_OPERATION

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <FS.h>
#include "config.h"
#include "IR_op.h"
#include "String_op.h"
#include "server_op.h"

extern const char softap_ssid[];
extern const char softap_pass[];
extern String target_ssid;
extern String target_pass;

void wifiSetup(void);

void setupAP(void);

void closeAP(void);

int connectWifi(void);

void wifiRestoreFromFile(void);

void wifiBackupToFile(void);

#endif

