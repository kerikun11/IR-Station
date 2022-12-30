/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __IR_SIGNAL_H__
#define __IR_SIGNAL_H__

#include <ESP8266WiFi.h>

#define IR_RECEIVE_TIMEOUT_US 200000
#define RAW_DATA_BUFFER_SIZE 800
#define REGARD_AS_NOISE_COUNT 8

enum IR_RECEIVER_STATE {
  IR_RECEIVER_OFF,
  IR_RECEIVER_READY,
  IR_RECEIVER_RECEIVING,
  IR_RECEIVER_READING,
  IR_RECEIVER_AVAILABLE,
};

class IR {
public:
  void begin(int tx, int rx);
  void handle();
  bool available();
  String read();
  void resume();
  void send(const String& dataJson);

private:
  int txPin, rxPin;
  volatile enum IR_RECEIVER_STATE state;
  volatile uint16_t rawIndex;
  volatile uint16_t rawData[RAW_DATA_BUFFER_SIZE];
  volatile uint32_t prev_us;

  void isr();
  static void IRAM_ATTR isrEntryPoint(void* this_ptr);
};

#endif
