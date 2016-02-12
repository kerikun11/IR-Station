#include "IR_op.h"

remocon ir[IR_CH_SIZE];

void irSendSignal(int ch) {
  digitalWrite(Indicate_LED, HIGH);
  ir[ch].sendSignal();
  digitalWrite(Indicate_LED, LOW);
}

int irRecodeSignal(int ch) {
  int ret = (-1);
  digitalWrite(Indicate_LED, HIGH);
  if (ir[ch].recodeSignal() == 0) {
    String dataString = ir[ch].getBackupString();
    SPIFFS.remove(IR_DATA_PATH(ch));
    File f = SPIFFS.open(IR_DATA_PATH(ch), "w");
    if (!f) {
      println_dbg("File open error");
    } else {
      f.println(dataString);
      f.close();
      println_dbg("Backup Successful");
    }
    ret = 0;
  }
  digitalWrite(Indicate_LED, LOW);
  return ret;
}

void irDataBackupToFile(int ch) {
  String dataString = ir[ch].getBackupString();
  SPIFFS.remove(IR_DATA_PATH(ch));
  File f = SPIFFS.open(IR_DATA_PATH(ch), "w");
  if (!f) {
    println_dbg("File open error: ch" + String(ch + 1));
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
      println_dbg("File open error: " + String(ch + 1));
    } else {
      String s = f.readStringUntil('\n');
      f.close();
      ir[ch].restoreFromString(s);
      println_dbg("Restore Successful: ch" + String(ch + 1));
    }
  }
}

