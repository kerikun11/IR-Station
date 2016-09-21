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

#define IR_STATION_MODE_SETUP   0
#define IR_STATION_MODE_STATION 1
#define IR_STATION_MODE_AP      2

class IR_Station {
  public:
    IR_Station(int pin_tx, int pin_rx, int pin_red, int pin_green, int pin_blue, int pin_button):
      _pin_tx(pin_tx), _pin_rx(pin_rx), indicator(pin_red, pin_green, pin_blue), _pin_button(pin_button) , server(80), httpUpdater(true) {
    }
    void begin();
    void reset();
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

    IR ir;
    int _pin_tx;
    int _pin_rx;
    Indicator indicator;
    int _pin_button;
    ESP8266WebServer server;
    ESP8266HTTPUpdateServer httpUpdater;
    DNSServer dnsServer;

    void buttonIsr();
    void displayRequest();

    bool irSendSignal(int id);
    bool irRecordSignal(String name, uint32_t timeout_ms = 5000);
    bool renameSignal(int id, String name);
    bool uploadSignal(String data);
    bool clearSignal(int id);
    bool clearSignals();
    bool changeIPSetting(String local_ip_s, String gateway_s, String subnetmask_s);

    void apiSetupWifilist();
    void apiSetupConfirm();
    void apiSetupIsconnected();
    void apiSetupSetapmode();
    void apiSetupTest();
    void apiSetupNotfound();

    void apiInfo();
    void apiSignalsSend();
    void apiSignalsRecord();
    void apiSignalsRename();
    void apiSignalsUpload();
    void apiSignalsClear();
    void apiSignalsClearall();
    void apiWifiDisconnect();
    void apiWifiChangeip();
    void apiDiscription();
    void apiNotfound();

    bool settingsRestoreFromFile();
    bool settingsBackupToFile();

    String resultJson(int code, String message);

    String settingsCrcSerial();
};

#endif

