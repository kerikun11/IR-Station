#include "led.h"

#include "config.h"

Indicator indicator(PIN_LED_R, PIN_LED_G, PIN_LED_B);

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

