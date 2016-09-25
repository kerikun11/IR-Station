/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef IR_OPERATION
#define IR_OPERATION

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266SSDP.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include "config.h"
#include "ir.h"
#include "led.h"
#include "ota.h"

#define IR_STATION_MODE_SETUP   0
#define IR_STATION_MODE_STATION 1
#define IR_STATION_MODE_AP      2

const int DNS_PORT = 53;
const int HTTP_PORT = 80;

class IR_Station {
  public:
    IR_Station(int pin_ir_tx, int pin_ir_rx, int pin_red, int pin_green, int pin_blue):
      indicator(pin_red, pin_green, pin_blue), server(HTTP_PORT), httpUpdater(true) {
      ir.begin(pin_ir_tx, pin_ir_rx);
    }
    void begin();
    void reset();
    void disconnect();
    void handle();

  private:
    uint8_t mode;
    bool is_stealth_ssid;
    String ssid;
    String password;
    String hostname;
    bool is_static_ip;
    IPAddress local_ip;
    IPAddress gateway;
    IPAddress subnetmask;

    int signalCount;
    String signalName[SIGNAL_COUNT_MAX + 1];

    IR ir;
    Indicator indicator;
    ESP8266WebServer server;
    ESP8266HTTPUpdateServer httpUpdater;
    DNSServer dnsServer;
    OTA ota;

    void restoreSignalName();
    bool irSendSignal(int ch);
    bool irRecordSignal(int ch, String name, uint32_t timeout_ms = 5000);
    bool renameSignal(int ch, String name);
    bool uploadSignal(int ch, String data);
    bool clearSignal(int ch);

    bool changeIp(String local_ip_s, String gateway_s, String subnetmask_s);

    void displayRequest();
    String resultJson(int code, String message);
    void attachSetupApi();
    void attachStationApi();

    String settingsCrcSerial();
    bool settingsRestoreFromFile();
    bool settingsBackupToFile();
};

#endif

