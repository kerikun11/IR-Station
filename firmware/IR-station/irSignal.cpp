#include "irSignal.h"

#include <ArduinoJson.h>
#include "config.h"

IR_Signal::IR_Signal(uint8_t tx, uint8_t rx) {
  txPin = tx;
  rxPin = rx;
  pinMode(tx, OUTPUT);
  pinMode(rx, INPUT);
}

bool IR_Signal::available() {
  return state == IR_RECEIVER_AVAILABLE;
}

String IR_Signal::read() {
  return irJson;
}

void IR_Signal::resume() {
  state = IR_RECEIVER_READY;
}

void IR_Signal::send(String dataJson) {
  enum IR_RECEIVER_STATE state_cache = state;
  state = IR_RECEIVER_OFF;
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(dataJson);
    noInterrupts();
    {
      for (uint16_t count = 0; count < root["data"].size(); count++) {
        wdt_reset();
        uint32_t us = micros();
        uint16_t time = (uint16_t)root["data"][count];
        do {
          digitalWrite(txPin, !(count & 1));
          delayMicroseconds(8);
          digitalWrite(txPin, 0);
          delayMicroseconds(16);
        } while (int32_t(us + time - micros()) > 0);
      }
    }
    interrupts();
  }
  state = state_cache;
  println_dbg("Send OK");
}

void IR_Signal::isr() {
  uint32_t us = micros();
  uint32_t diff = us - prev_us;

  switch (state) {
    case IR_RECEIVER_READY:
      state = IR_RECEIVER_RECEIVING;
      rawIndex = 0;
      break;
    case IR_RECEIVER_RECEIVING:
      while (diff > 0xFFFF) {
        if (rawIndex > RAWDATA_BUFFER_SIZE - 2) {
          println_dbg("IR buffer overflow");
          break;
        }
        rawData[rawIndex++] = 0xFFFF;
        rawData[rawIndex++] = 0;
        diff -= 0xFFFF;
      }
      if (rawIndex > RAWDATA_BUFFER_SIZE - 1) {
        println_dbg("IR buffer overflow");
        break;
      }
      rawData[rawIndex++] = diff;
      break;
    case IR_RECEIVER_READING:
      break;
  }

  prev_us = us;
}

void IR_Signal::handle() {
  noInterrupts();
  uint32_t diff = micros() - prev_us;
  interrupts();

  switch (state) {
    case IR_RECEIVER_READY:
      break;
    case IR_RECEIVER_RECEIVING:
      if (diff > IR_RECEIVE_TIMEOUT_US) {
        state = IR_RECEIVER_READING;
        println_dbg("End Receiving");
      }
      break;
    case IR_RECEIVER_READING:
      if (rawIndex < 8) {
        println_dbg("noise");
        state = IR_RECEIVER_READY;
        break;
      }

      println_dbg("Raw Data: ");
      for (int i = 0; i < rawIndex; i++) {
        print_dbg(rawData[i], DEC);
        if (i != rawIndex - 1) print_dbg(",");
      }
      println_dbg("");

      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      JsonArray& data = root.createNestedArray("data");
      for (int i = 0; i < rawIndex; i++) {
        data.add(rawData[i]);
      }
      irJson = "";
      root.printTo(irJson);

      state = IR_RECEIVER_AVAILABLE;
      break;
  }
}

