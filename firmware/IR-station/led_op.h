#ifndef __LED_H__
#define __LED_H__

#include <ESP8266WiFi.h>

class Indicator {
  public:
    Indicator(int pin_red, int pin_green, int pin_blue)
      : _pin_red(pin_red), _pin_green(pin_green), _pin_blue(pin_blue) {
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

extern Indicator indicator;

#endif

