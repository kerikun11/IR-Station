#include "IR_op.h"

remocon ir[IR_CH_SIZE];

int remocon::sendSignal(void) {
  uint16_t rawData[RAW_DATA_SIZE];
  bin2raw(rawData);
  for (uint16_t count = 0; rawData[count]; count++) {
    uint32_t us = micros();
    uint16_t time = rawData[count];
    ESP.wdtFeed();
    do {
      digitalWrite(IR_OUT, !(count & 1));
      delayMicroseconds(8);
      digitalWrite(IR_OUT, 0);
      delayMicroseconds(16);
    } while (int32_t(us + time - micros()) > 0);
  }
  Serial.println("Send OK");
  return 0;
}

int remocon::recodeSignal(void) {
  uint16_t rawData[RAW_DATA_SIZE];
  uint16_t rawDataSize;
  uint8_t pre_value = HIGH;
  uint8_t now_value = HIGH;
  uint8_t wait_flag = HIGH;
  uint32_t pre_us = micros();
  uint32_t now_us = 0;

  rawDataSize = 0;
  while (1) {
    ESP.wdtFeed();
    now_value = digitalRead(IR_IN);
    if (pre_value != now_value) {
      now_us = micros();
      if (!wait_flag) {
        rawData[rawDataSize++] = now_us - pre_us;
      }
      wait_flag = LOW;
      pre_value = now_value;
      pre_us = now_us;
    }

    if (wait_flag) {
      if ((micros() - pre_us) > TIMEOUT_RECODE) {
        Serial.println("No signal received");
        return (-1);
      }
    } else {
      if ((micros() - pre_us) > TIMEOUT_RECODE_NOSIGNAL) {
        rawData[rawDataSize++] = 0;
        raw2bin(rawData);
        return 0;
      }
    }
  }
}

void remocon::raw2bin(uint16_t *rawData) {
  dispRawData(rawData);

  irData = "";
  period = rawData[0];
  while (period > MAX_PERIOD) {
    period /= 2;
  }
  for (int i = 0; rawData[i]; i++) {
    irData += (char)('0' + (rawData[i] + EXTRA_PERIOD) / period);
  }

  dispData();
}

void remocon::bin2raw(uint16_t *rawData) {
  dispData();

  uint16_t rawDataSize = 0;
  for (int i = 0; irData[i]; i++) {
    rawData[i] = period * (irData[i] - '0');
  }

  dispRawData(rawData);
}

void remocon::dispData(void) {
  Serial.print("Period: ");
  Serial.println(period, DEC);
  Serial.print("IR Data: ");
  Serial.println(irData);
}

void remocon::dispRawData(uint16_t* rawData) {
  Serial.print("Raw Data: ");
  for (uint16_t count = 0; rawData[count]; count++) {
    Serial.print(rawData[count], DEC);
    Serial.print(",");
  }
  Serial.println("End");
}

void remocon::dataBackupToFile(String path) {
  Serial.println("IR data backup");
  // irFormat,chName,period,binDataSize,binData
  SPIFFS.remove(path);
  File f = SPIFFS.open(path, "w");
  if (!f) {
    Serial.println("file open error");
    return;
  }
  f.print("?period=");
  f.print(period, DEC);
  f.print("&irData=" + irData);
  f.print("&chName=" + chName);
  f.println("&End");
  f.close();
  Serial.println("Successful");
}

void remocon::dataRestoreFromFile(String path) {
  Serial.println("path: " + path);
  File f = SPIFFS.open(path, "r");
  if (!f) {
    Serial.println("file open error");
    return;
  }
  String s = f.readStringUntil('\n');
  f.close();
  Serial.println("data: " + s);
  period = extract(s, "?period=").toInt();
  irData = extract(s, "&irData=");
  chName = extract(s, "&chName=");
}

