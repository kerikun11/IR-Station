#include "IR_op.h"

#include <ArduinoJson.h>
#include <FS.h>
#include "config.h"
#include "server_op.h"
#include "WiFi_op.h"

remocon ir[IR_CH_SIZE];
uint8_t mode = IR_STATION_MODE_STA;

void modeSetup(void) {
  wdt_reset();
  switch (mode) {
    case IR_STATION_MODE_NULL:
      println_dbg("Boot Mode: NULL");
      mdns_address = MDNS_ADDRESS_DEFAULT;
      break;
    case IR_STATION_MODE_AP:
      println_dbg("Boot Mode: AP");
      return;
      break;
    case IR_STATION_MODE_STA:
      println_dbg("Boot Mode: Station");
      if (connectCachedWifi() == true) return;
      break;
  }
  setupAP();
  setupFormServer();
  while (1) {
    wdt_reset();
    serverTask();
  }
}

void irSendSignal(int ch) {
  digitalWrite(PIN_LED1, HIGH);
  ir[ch].sendSignal();
  digitalWrite(PIN_LED1, LOW);
}

int irRecodeSignal(int ch) {
  int ret = (-1);
  digitalWrite(PIN_LED1, HIGH);
  if (ir[ch].recodeSignal() == 0) {
    irDataBackupToFile(ch);
  }
  digitalWrite(PIN_LED1, LOW);
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
  getStringFromFile(SETTINGS_DATA_PATH, s);
  println_dbg("data: " + s);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(s);
  mode = data["mode"];
  mdns_address = (const char*)data["mdns_address"];
  if (mdns_address == "") {
    mdns_address = MDNS_ADDRESS_DEFAULT;
  }
}

void settingsBackupToFile(void) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["mode"] = mode;
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
  return true;
  println_dbg("Backup successful: " + path);
}

bool getStringFromFile(String path, String& dataString) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    println_dbg("File open Error: " + path);
    return false;
  }
  dataString = "";
  while (file.available()) {
    dataString += file.readStringUntil('\n');
  }
  file.close();
  println_dbg("Restored successful: " + path);
}

