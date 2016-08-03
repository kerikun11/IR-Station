#include "ir-stationTask.h"

#include <ArduinoJson.h>
#include <FS.h>
#include "config.h"
#include "httpServerTask.h"
#include "wifiTask.h"
#include "timeTask.h"
#include "otaTask.h"
#include "ledTask.h"
#include "crc8.h"

remocon ir[IR_CH_SIZE];
IR_Station station;

void IR_Station::modeSetup(void) {
  wdt_reset();
  indicator.green(1023);

  // Prepare SPIFFS
  SPIFFS.begin();

  // Restore reserved data
  irDataRestoreFromFile();
  if (settingsRestoreFromFile() == false) {
    reset();
  }

  setupButtonInterrupt();

  switch (mode) {
    case IR_STATION_MODE_NULL:
      println_dbg("Boot Mode: NULL");
      // set WiFi Mode
      WiFi.mode(WIFI_AP_STA);
      setupOTA();
      setupAP(SOFTAP_SSID, SOFTAP_PASS);
      setupFormServer();
      break;
    case IR_STATION_MODE_STA:
      println_dbg("Boot Mode: Station");
      WiFi.mode(WIFI_STA);
      connectWifi(ssid, password);
      setupOTA();
      setupServer();
      setupTime();
      indicator.green(0);
      indicator.blue(1023);
      break;
    case IR_STATION_MODE_AP:
      println_dbg("Boot Mode: AP");
      // set WiFi Mode
      WiFi.mode(WIFI_AP);
      setupAP(SOFTAP_SSID, SOFTAP_PASS);
      setupServer();
      indicator.green(0);
      indicator.blue(1023);
      break;
  }
}

void IR_Station::reset() {
  ssid = "";
  password = "";
  hostname = HOSTNAME_DEFAULT;
  setMode(IR_STATION_MODE_NULL);
  ESP.reset();
}

void IR_Station::setMode(uint8_t newMode) {
  mode = newMode;
  settingsBackupToFile();
}

void IR_Station::setupButtonInterrupt() {
  attachInterrupt(PIN_BUTTON, []() {
    static uint32_t prev_ms;
    if (digitalRead(PIN_BUTTON) == LOW) {
      prev_ms = millis();
      println_dbg("the button pressed");
    } else {
      println_dbg("the button released");
      if (millis() - prev_ms > 2000) {
        println_dbg("the button long pressed");
        station.reset();
      }
    }
  }, CHANGE);
  println_dbg("attached button interrupt");
}

void IR_Station::irSendSignal(int ch) {
  indicator.set(0, 1023, 0);
  ir[ch].sendSignal();
  indicator.set(0, 0, 1023);
}

int IR_Station::irRecodeSignal(int ch) {
  indicator.set(0, 1023, 0);
  int ret = ir[ch].recodeSignal();
  if (ret == 0) {
    irDataBackupToFile(ch);
  }
  indicator.set(0, 0, 1023);
  return ret;
}

bool IR_Station::irDataBackupToFile(int ch) {
  String dataString = ir[ch].getBackupString();
  return writeStringToFile(IR_DATA_PATH(ch + 1), dataString);
}

bool IR_Station::irDataRestoreFromFile(void) {
  for (uint8_t ch = 0; ch < IR_CH_SIZE; ch++) {
    String str;
    if (getStringFromFile(IR_DATA_PATH(ch + 1), str) == false)return false;
    ir[ch].restoreFromString(str);
  }
  return true;
}

String IR_Station::settingsCrcSerial(void) {
  return String(mode, DEC) + ssid + password + hostname;
}

bool IR_Station::settingsRestoreFromFile(void) {
  String s;
  if (getStringFromFile(SETTINGS_DATA_PATH, s) == false) return false;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(s);
  mode = (int)data["mode"];
  ssid = (const char*)data["ssid"];
  password = (const char*)data["password"];
  hostname = (const char*)data["hostname"];
  uint8_t crc = (uint8_t)data["crc"];
  String serial = settingsCrcSerial();
  if (crc != crc8((uint8_t*)serial.c_str(), serial.length(), CRC8INIT)) {
    println_dbg("CRC8 difference");
    return false;
  }
  println_dbg("CRC8 OK");
  return true;
}

bool IR_Station::settingsBackupToFile(void) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["mode"] = mode;
  data["ssid"] = ssid;
  data["password"] = password;
  data["hostname"] = hostname;
  String serial = settingsCrcSerial();
  data["crc"] = crc8((uint8_t*)serial.c_str(), serial.length(), CRC8INIT);
  String str;
  data.printTo(str);
  return writeStringToFile(SETTINGS_DATA_PATH, str);
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

