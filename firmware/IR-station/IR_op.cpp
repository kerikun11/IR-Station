#include "IR_op.h"

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
    String dataString = ir[ch].getBackupString();
    SPIFFS.remove(IR_DATA_PATH(ch));
    File f = SPIFFS.open(IR_DATA_PATH(ch), "w");
    if (!f) {
      println_dbg("File open Indicate");
    } else {
      f.println(dataString);
      f.close();
      println_dbg("Backup Successful");
    }
    ret = 0;
  }
  digitalWrite(PIN_LED1, LOW);
  return ret;
}

void irDataBackupToFile(int ch) {
  String dataString = ir[ch].getBackupString();
  SPIFFS.remove(IR_DATA_PATH(ch));
  File f = SPIFFS.open(IR_DATA_PATH(ch), "w");
  if (!f) {
    println_dbg("File open Error: ch" + String(ch + 1));
  } else {
    f.println(dataString);
    f.close();
    println_dbg("Backup Successful: ch" + String(ch + 1));
  }
}

void irDataRestoreFromFile(void) {
  for (uint8_t ch = 0; ch < IR_CH_SIZE; ch++) {
    File f = SPIFFS.open(IR_DATA_PATH(ch), "r");
    if (!f) {
      println_dbg("File open Error: " + String(ch + 1));
    } else {
      String s = f.readStringUntil('\n');
      f.close();
      ir[ch].restoreFromString(s);
      println_dbg("Restore Successful: ch" + String(ch + 1));
    }
  }
}

void settingsRestoreFromFile(void) {
  File f = SPIFFS.open(SETTINGS_DATA_PATH, "r");
  if (!f) {
    println_dbg("Settings: file open Error");
  } else {
    String s = f.readStringUntil('\n');
    println_dbg("Settings data: " + s);
    mode = extract(s, "?mode=").toInt();
    String mdns = extract(s, "&mdns=");
    if (mdns != "") {
      mdns_address = mdns;
    } else {
      mdns_address = MDNS_ADDRESS_DEFAULT;
    }
    f.close();
    println_dbg("Restored Settings from File");
  }
}

void settingsBackupToFile(void) {
  SPIFFS.remove(SETTINGS_DATA_PATH);
  File f = SPIFFS.open(SETTINGS_DATA_PATH, "w");
  if (!f) {
    println_dbg("Settings: file open Error");
    return;
  }
  f.print("?mode=" + String(mode, DEC));
  f.print("&mdns=" + mdns_address);
  f.println("&End");
  f.close();
  println_dbg("Settings data backup successful");
}

String extract(String target, String head, String tail) {
  return target.substring(target.indexOf(head) + head.length(), target.indexOf(tail, target.indexOf(head) + head.length()));
}

