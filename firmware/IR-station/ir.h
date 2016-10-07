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

#define IR_RECEIVE_TIMEOUT_US   200000
#define RAWDATA_BUFFER_SIZE     800
#define REGARD_AS_NOISE_COUNT   8

enum IR_RECEIVER_STATE {
  IR_RECEIVER_OFF,
  IR_RECEIVER_READY,
  IR_RECEIVER_RECEIVING,
  IR_RECEIVER_READING,
  IR_RECEIVER_AVAILABLE,
};

class IR {
  public:
    static void begin(int tx, int rx);
    static void handle();
    static bool available();
    static String read();
    static void resume();

    static void send(String dataJson);

  private:
    static int txPin, rxPin;
    static volatile enum IR_RECEIVER_STATE state;
    static volatile uint16_t rawIndex;
    static volatile uint16_t rawData[RAWDATA_BUFFER_SIZE];
    static volatile uint32_t prev_us;

    static void isr();
};

#endif

