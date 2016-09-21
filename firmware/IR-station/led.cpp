#include "led.h"

#include "config.h" // for print_dbg()

void Indicator::set(int val_red, int val_green, int val_blue) {
  analogWrite(_pin_red, val_red);
  analogWrite(_pin_green, val_green);
  analogWrite(_pin_blue, val_blue);
}

void Indicator::red(int value) {
  analogWrite(_pin_red, value);
}

void Indicator::green(int value) {
  analogWrite(_pin_green, value);
}

void Indicator::blue(int value) {
  analogWrite(_pin_blue, value);
}

