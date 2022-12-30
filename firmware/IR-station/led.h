/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __LED_H__
#define __LED_H__

#include <WiFi.h>

class Indicator {
public:
  Indicator(int pin_red, int pin_green, int pin_blue)
    : _pin_red(pin_red), _pin_green(pin_green), _pin_blue(pin_blue) {
    for (auto pin : { pin_red, pin_green, pin_blue })
      pinMode(pin, OUTPUT);
    set(0, 0, 0);
  }
  void set(int val_red, int val_green, int val_blue);
  void red(int value);
  void green(int value);
  void blue(int value);

private:
  int _pin_red;
  int _pin_green;
  int _pin_blue;
};

#endif
