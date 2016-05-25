#ifndef LCD_op_h
#define LCD_op_h

#include <ESP8266WiFi.h>

void setupLcd();
void lcdPutIP(IPAddress IP);
void lcdTask();

#endif

