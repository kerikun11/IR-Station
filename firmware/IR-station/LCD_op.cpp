#include "LCD_op.h"

#include <Wire.h>
#include "ST7032.h"
#include "IR_op.h"

ST7032 lcd;

void setupLcd() {
  Wire.begin(PIN_SDA, PIN_SCL);
  lcd.begin(8, 2);
  lcd.setContrast(30);
  lcd.home();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IR-");
  lcd.setCursor(0, 1);
  lcd.print("Station");
}

void lcdPutIP(IPAddress IP) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print((String)IP[0] + "." + IP[1] + ".");
  lcd.setCursor(0, 1);
  lcd.print((String)IP[2] + "." + IP[3]);
}

void lcdTask() {
  static IPAddress cachedIP;
  IPAddress IP;
  if (mode == IR_STATION_MODE_STA)IP = WiFi.localIP();
  else IP = WiFi.softAPIP();
  if (IP != cachedIP) {
    cachedIP = IP;
    lcdPutIP(IP);
  }
}

