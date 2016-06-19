#include "IR-lib.h"

#include <ArduinoJson.h>

void remocon::sendSignal(void) {
  for (uint16_t count = 0; irData[count]; count++) {
    uint32_t us = micros();
    uint16_t time = period * (irData[count] - '0');
    do {
      digitalWrite(PIN_IR_OUT, !(count & 1));
      delayMicroseconds(8);
      digitalWrite(PIN_IR_OUT, 0);
      delayMicroseconds(16);
    } while (int32_t(us + time - micros()) > 0);
  }
  dispData();
  println_dbg("Send OK");
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
    wdt_reset();
    now_value = digitalRead(PIN_IR_IN);
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
        println_dbg("No signal received");
        return (-1);
      }
    } else {
      if ((micros() - pre_us) > TIMEOUT_RECODE_NOSIGNAL) {
        dispData();
        break;
      }
    }
  }
  return 0;
}

void remocon::dispData(void) {
  print_dbg("Ch Name: ");
  println_dbg(chName);
  print_dbg("Period: ");
  println_dbg(period, DEC);
  print_dbg("IR Data: ");
  println_dbg(irData);
}

String remocon::getBackupString(void) {
  println_dbg("Generating IR data in Json...");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["period"] = period;
  data["irData"] = irData;
  data["chName"] = chName;
  String str;
  data.printTo(str);
  return str;
}

void remocon::restoreFromString(String dataString) {
  println_dbg("Psing IR data from Json...");
  println_dbg("data: " + dataString);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(dataString);
  period = (uint16_t)data["period"];
  irData = (const char*)data["irData"];
  chName = (const char*)data["chName"];
}

