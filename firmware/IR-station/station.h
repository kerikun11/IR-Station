/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __STATION_H__
#define __STATION_H__

#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#undef max(a,b)
#include <vector>
#include "config.h"
#include "ir.h"
#include "led.h"
#include "myssdp.h"

#if USE_ALEXA == true
#include <fauxmoESP.h>
#endif


#define IR_STATION_MODE_SETUP   0
#define IR_STATION_MODE_STATION 1
#define IR_STATION_MODE_AP      2

#define STATION_JSON_PATH       ("/station.json")
#define IR_DATA_PATH(id)        ("/main/signals/"+String(id,DEC)+".json")

const int DNS_PORT = 53;
const int HTTP_PORT = 80;

extern const char* _ssdp_schema_template;

struct Signal {
  int id;
  String name;
  String path;
  bool display;
  int row;
  int column;
};

struct Schedule {
  int schedule_id;
  int id;
  time_t time;
};

#if USE_ALEXA == true
struct Alexa {
  int on;
  int off;
  int brighter;
  int darker;
  uint8_t value;
  bool state;
};
#endif

class IR_Station {
  public:
    IR_Station(int pin_ir_tx, int pin_ir_rx, int pin_red, int pin_green, int pin_blue):
      indicator(pin_red, pin_green, pin_blue), server(HTTP_PORT)
#if USE_ALEXA == true
      , fauxmo()
#endif
    {
      ir.begin(pin_ir_tx, pin_ir_rx);
    }
    void begin();
    //void startWebUI();
    void stopWebUI();
    void reset(bool clean = true);
    void handle();

  private:
    String version;
    uint8_t mode;
    String hostname;

    bool is_stealth_ssid;
    String ssid;
    String password;

    bool is_static_ip;
    IPAddress local_ip;
    IPAddress gateway;
    IPAddress subnetmask;

    int next_id;
    std::vector<Signal> signals;

    int next_schedule_id;
    std::vector<Schedule> schedules;

    IR ir;
    Indicator indicator;
    AsyncWebServer server;
    DNSServer dnsServer;
    bool run_save = false;

#if USE_ALEXA == true
    fauxmoESP fauxmo;
    bool alexa_mode = false;
    std::map<String, Alexa> alexaDevs;
#endif

    void handleSchedule();
    int getNewId();
    int getNewScheduleId();
    Signal *getSignalById(int id);
    bool restore();
    void safe_save();
    bool save();

    void displayRequest(AsyncWebServerRequest *req);
    void attachSetupApi();
    void attachStationApi();
};

#endif

