/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ir.h"

#include <ArduinoJson.h>
#include "config.h" // for print_dbg()

int IR::txPin, IR::rxPin;
volatile enum IR_RECEIVER_STATE IR::state;
volatile uint16_t IR::rawIndex;
volatile uint16_t IR::rawData[RAWDATA_BUFFER_SIZE];
volatile uint32_t IR::prev_us;

void IR::begin(int tx, int rx) {
  txPin = tx;
  rxPin = rx;
  pinMode(tx, OUTPUT);
  pinMode(rx, INPUT);
  attachInterrupt(rx, &IR::isr, CHANGE);
  state = IR_RECEIVER_READY;
}

bool IR::available() {
  return state == IR_RECEIVER_AVAILABLE;
}

String IR::read() {
  String data = "";
  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.createArray();
  for (int i = 0; i < rawIndex; i++) {
    root.add(rawData[i]);
  }
  root.printTo(data);
  return data;
}

void IR::resume() {
  state = IR_RECEIVER_READY;
}

void IR::send(const String& dataJson) {
  enum IR_RECEIVER_STATE state_cache = state;
  state = IR_RECEIVER_OFF;
  {
    DynamicJsonBuffer jsonBuffer;
    JsonArray& root = jsonBuffer.parseArray(dataJson);
    noInterrupts();
    {
      for (uint16_t count = 0; count < root.size(); count++) {
        wdt_reset();
        uint32_t us = micros();
        uint16_t time = (uint16_t)root[count];
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

void IR::isr() {
  uint32_t us = micros();
  uint32_t diff = us - prev_us;

  switch (state) {
    case IR_RECEIVER_OFF:
      break;
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
    case IR_RECEIVER_AVAILABLE:
      break;
  }

  prev_us = us;
}

void IR::handle() {
  noInterrupts();
  uint32_t diff = micros() - prev_us;
  interrupts();

  switch (state) {
    case IR_RECEIVER_OFF:
      break;
    case IR_RECEIVER_READY:
      break;
    case IR_RECEIVER_RECEIVING:
      if (diff > IR_RECEIVE_TIMEOUT_US) {
        state = IR_RECEIVER_READING;
        println_dbg("End Receiving");
      }
      break;
    case IR_RECEIVER_READING:
      if (rawIndex < REGARD_AS_NOISE_COUNT) {
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

      state = IR_RECEIVER_AVAILABLE;
      break;
    case IR_RECEIVER_AVAILABLE:
      break;
  }
}

