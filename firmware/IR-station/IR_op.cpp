#include "IR_op.h"

#include <ArduinoJson.h>
#include <FS.h>
#include "config.h"
#include "server_op.h"
#include "WiFi_op.h"
#include "time_op.h"
#include "OTA_op.h"
#include "led_op.h"

remocon ir[IR_CH_SIZE];
uint8_t mode;
String ssid;
String password;
String mdns_address;

void modeSetup(void) {
  wdt_reset();

  // Prepare SPIFFS
  SPIFFS.begin();

  // Restore reserved data
  irDataRestoreFromFile();
  settingsRestoreFromFile();

  setupButtonInterrupt();

  switch (mode) {
    case IR_STATION_MODE_NULL:
      println_dbg("Boot Mode: NULL");
      // set WiFi Mode
      WiFi.mode(WIFI_AP_STA);
      setupAP();
      setupFormServer();
      break;
    case IR_STATION_MODE_STA:
      println_dbg("Boot Mode: Station");
      // set WiFi Mode
      WiFi.mode(WIFI_STA);
      connectWifi(ssid, password);
      setupOTA();
      setupServer();
      break;
    case IR_STATION_MODE_AP:
      println_dbg("Boot Mode: AP");
      // set WiFi Mode
      WiFi.mode(WIFI_AP);
      setupAP();
      setupServer();
      break;
  }
}

void setMode(uint8_t newMode) {
  mode = newMode;
  settingsBackupToFile();
}

void setupButtonInterrupt() {
  attachInterrupt(PIN_BUTTON, []() {
    static uint32_t prev_ms;
    if (digitalRead(PIN_BUTTON) == LOW) {
      prev_ms = millis();
      println_dbg("the button pressed");
    } else {
      println_dbg("the button released");
      if (millis() - prev_ms > 2000) {
        println_dbg("the button long pressed");
        setMode(IR_STATION_MODE_NULL);
        ESP.reset();
      }
    }
  }, CHANGE);
  println_dbg("attached button interrupt");
}

void irSendSignal(int ch) {
  indicator.blue(1023);
  ir[ch].sendSignal();
  indicator.blue(0);
}

int irRecodeSignal(int ch) {
  int ret = (-1);
  indicator.blue(1023);
  if (ir[ch].recodeSignal() == 0) {
    irDataBackupToFile(ch);
  }
  indicator.blue(0);
  return ret;
}

void irDataBackupToFile(int ch) {
  String dataString = ir[ch].getBackupString();
  writeStringToFile(IR_DATA_PATH(ch + 1), dataString);
}

void irDataRestoreFromFile(void) {
  for (uint8_t ch = 0; ch < IR_CH_SIZE; ch++) {
    String str;
    getStringFromFile(IR_DATA_PATH(ch + 1), str);
    ir[ch].restoreFromString(str);
  }
}

void settingsRestoreFromFile(void) {
  String s;
  if (getStringFromFile(SETTINGS_DATA_PATH, s) == false) return;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(s);
  mode = (int)data["mode"];
  ssid = (const char*)data["ssid"];
  password = (const char*)data["password"];
  mdns_address = (const char*)data["mdns_address"];
}

void settingsBackupToFile(void) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["mode"] = mode;
  data["ssid"] = ssid;
  data["password"] = password;
  data["mdns_address"] = mdns_address;
  String str;
  data.printTo(str);
  writeStringToFile(SETTINGS_DATA_PATH, str);
}

bool writeStringToFile(String path, String dataString) {
  SPIFFS.remove(path);
  File file = SPIFFS.open(path, "w");
  if (!file) {
    println_dbg("File open Error: " + path);
    return false;
  }
  file.print(dataString);
  file.close();
  println_dbg("Backup successful: " + path);
  println_dbg("data: " + dataString);
  return true;
}

bool getStringFromFile(String path, String& dataString) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    println_dbg("File open Error: " + path);
    return false;
  }
  file.setTimeout(10);
  dataString = file.readString();
  file.close();
  println_dbg("Restore successful: " + path);
  println_dbg("data: " + dataString);
  return true;
}

