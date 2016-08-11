#include "station.h"

#include <ArduinoJson.h>
#include <FS.h>
#include "config.h"
#include "httpServer.h"
#include "file.h"
#include "wifi.h"
#include "ota.h"
#include "led.h"
#include "crc8.h"

IR_Station station;
IR_Signal ir(PIN_IR_OUT, PIN_IR_IN);

void IR_Station::begin(void) {
  wdt_reset();
  indicator.green(1023);

  SPIFFS.begin();
  if (settingsRestoreFromFile() == false) {
    reset();
  }
  restoreChName();

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
      connectWifi(ssid, password, stealth);
      setupOTA();
      setupServer();
      indicator.green(0);
      indicator.blue(1023);
      attachInterrupt(ir.rxPin, []() {
        ir.isr();
      }, CHANGE);
      break;
    case IR_STATION_MODE_AP:
      println_dbg("Boot Mode: AP");
      // set WiFi Mode
      WiFi.mode(WIFI_AP);
      setupAP(SOFTAP_SSID, SOFTAP_PASS);
      setupServer();
      indicator.green(0);
      indicator.blue(1023);
      attachInterrupt(ir.rxPin, []() {
        ir.isr();
      }, CHANGE);
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

bool IR_Station::irSendSignal(int ch) {
  String json;
  if (getStringFromFile(IR_DATA_PATH(ch), json)) {
    indicator.set(0, 1023, 0);
    ir.send(json);
    indicator.set(0, 0, 1023);
    return true;
  }
  return false;
}

bool IR_Station::clearSignals() {
  for (uint8_t ch = 0; ch < IR_CH_SIZE; ch++) {
    clearSignal(ch);
  }
  println_dbg("Cleared All Signals");
  return true;
}

bool IR_Station::clearSignal(int ch) {
  station.chName[ch] = "ch " + String(ch + 1, DEC);
  return removeFile(IR_DATA_PATH(ch));
}

bool IR_Station::renameSignal(int ch, String name) {
  String data;
  if (!getStringFromFile(IR_DATA_PATH(ch), data)) {
    return false;
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data);
  root["name"] = name;
  chName[ch] = name;
  data = "";
  root.printTo(data);
  return writeStringToFile(IR_DATA_PATH(ch), data);
}

bool IR_Station::irRecodeSignal(int ch, String name, uint32_t timeout_ms) {
  indicator.set(0, 1023, 0);
  ir.resume();
  int timeStamp = millis();
  while (!ir.available()) {
    wdt_reset();
    ir.handle();
    if (millis() - timeStamp > timeout_ms) {
      indicator.set(1023, 0, 0);
      return false;
    }
  }
  String data = ir.read();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data);
  root["name"] = name;
  chName[ch] = name;
  data = "";
  root.printTo(data);
  indicator.set(0, 0, 1023);
  return writeStringToFile(IR_DATA_PATH(ch), data);
}

bool IR_Station::uploadSignal(int ch, String name, String data) {
  indicator.set(0, 1023, 0);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data);
  if (name == "")name = (const char*)root["name"];
  root["name"] = name;
  chName[ch] = name;
  data = "";
  root.printTo(data);
  indicator.set(0, 0, 1023);
  return writeStringToFile(IR_DATA_PATH(ch), data);
}

void IR_Station::restoreChName(void) {
  for (int ch = 0; ch < IR_CH_SIZE; ch++) {
    String json;
    if (getStringFromFile(IR_DATA_PATH(ch), json)) {
      DynamicJsonBuffer jsonBuffer;
      JsonObject& data = jsonBuffer.parseObject(json);
      chName[ch] = (const char*)data["name"];
    }
  }
}

String IR_Station::settingsCrcSerial(void) {
  return String(mode, DEC) + ssid + password + hostname + String(stealth, DEC);
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
  stealth = (bool)data["stealth"];
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
  JsonObject& root = jsonBuffer.createObject();
  root["mode"] = mode;
  root["ssid"] = ssid;
  root["password"] = password;
  root["hostname"] = hostname;
  root["stealth"] = stealth;
  String serial = settingsCrcSerial();
  root["crc"] = crc8((uint8_t*)serial.c_str(), serial.length(), CRC8INIT);
  String str = "";
  root.printTo(str);
  return writeStringToFile(SETTINGS_DATA_PATH, str);
}

