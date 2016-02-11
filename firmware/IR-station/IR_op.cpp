#include "IR_op.h"

remocon ir[IR_CH_SIZE];

int remocon::sendSignal(void) {
  for (uint16_t count = 0; irData[count]; count++) {
    uint32_t us = micros();
    uint16_t time = period * (irData[count] - '0');
    ESP.wdtFeed();
    do {
      digitalWrite(IR_OUT, !(count & 1));
      delayMicroseconds(8);
      digitalWrite(IR_OUT, 0);
      delayMicroseconds(16);
    } while (int32_t(us + time - micros()) > 0);
  }
  println_dbg("Send OK");
  return 0;
}

int remocon::recodeSignal(void) {
  uint16_t time;
  bool firstBit = true;
  uint8_t pre_value = HIGH;
  uint8_t now_value = HIGH;
  uint8_t wait_flag = HIGH;
  uint32_t pre_us = micros();
  uint32_t now_us = 0;

  while (1) {
    //ESP.wdtFeed();
    now_value = digitalRead(IR_IN);
    if (pre_value != now_value) {
      now_us = micros();
      if (!wait_flag) {
        time = now_us - pre_us;
        if (firstBit) {
          firstBit = false;
          irData = "";
          period = time;
          if (period < MIN_PERIOD) {
            break;
          }
          while (period > MAX_PERIOD) {
            period /= 2;
          }
        }
        irData += (char)('0' + (time + EXTRA_PERIOD) / period);
      }
      wait_flag = LOW;
      pre_value = now_value;
      pre_us = now_us;
    }

    if (wait_flag) {
      if ((micros() - pre_us) > TIMEOUT_RECODE) {
      }
    } else {
      if ((micros() - pre_us) > TIMEOUT_RECODE_NOSIGNAL) {
        dispData();
        return 0;
      }
    }
  }
  println_dbg("No signal received");
  return (-1);
}

void remocon::dispData(void) {
  print_dbg("Period: ");
  println_dbg(period, DEC);
  print_dbg("IR Data: ");
  println_dbg(irData);
}

void remocon::dataBackupToFile(String path) {
  println_dbg("IR data backup");
  // irFormat,chName,period,binDataSize,binData
  SPIFFS.remove(path);
  File f = SPIFFS.open(path, "w");
  if (!f) {
    println_dbg("File open error");
    return;
  }
  f.print("?period=");
  f.print(period, DEC);
  f.print("&irData=" + irData);
  f.print("&chName=" + chName);
  f.println("&End");
  f.close();
  println_dbg("Successful");
}

void remocon::dataRestoreFromFile(String path) {
  print_dbg("path: " + path + " ");
  File f = SPIFFS.open(path, "r");
  if (!f) {
    println_dbg("File open error");
    return;
  }
  String s = f.readStringUntil('\n');
  f.close();
  println_dbg("data: " + s);
  period = extract(s, "?period=").toInt();
  irData = extract(s, "&irData=");
  chName = extract(s, "&chName=");
}

