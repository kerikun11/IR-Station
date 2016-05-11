/*
  ST7032.cpp - Arduino LiquidCrystal compatible library
  
  Original source is Arduino LiquidCrystal liblary
  
  Author:  tomozh@gmail.com
  License: MIT

  History:
    2014.10.13 コントラスト値のbit7がBONビットに影響する不具合を修正
    2014.08.23 コンストラクタでI2Cアドレスを設定可能にした
    2013.05.21 1st release
  
 ------------------------
  Arduino        ST7032i
 ------------------------
  3.3V    --+-- VDD
            +-- -RES
  A4(SDA) --*-- SDA
  A5(SCL) --*-- SCL
  GND     ----- GND
  
  *... 10Kohm pull-up
 ------------------------

*/


#include "ST7032.h"
#include "Arduino.h"
#include <Wire.h>
//#include <avr/pgmspace.h>

// private methods

void ST7032::setDisplayControl(uint8_t setBit) {
  _displaycontrol |= setBit;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void ST7032::resetDisplayControl(uint8_t resetBit) {
  _displaycontrol &= ~resetBit;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void ST7032::setEntryMode(uint8_t setBit) {
  _displaymode |= setBit;
  command(LCD_ENTRYMODESET | _displaymode);
}

void ST7032::resetEntryMode(uint8_t resetBit) {
  _displaymode &= ~resetBit;
  command(LCD_ENTRYMODESET | _displaymode);
}

void ST7032::normalFunctionSet() {
  command(LCD_FUNCTIONSET | _displayfunction);
}

void ST7032::extendFunctionSet() {
  command(LCD_FUNCTIONSET | _displayfunction | LCD_EX_INSTRUCTION);
}


// public methods


ST7032::ST7032(int i2c_addr)
: _displaycontrol(0x00)
, _displaymode(0x00)
, _i2c_addr((uint8_t)i2c_addr)
{
//  begin(16, 1);
}

void ST7032::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {

  _displayfunction  = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;
  _currline = 0;

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != 0) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }
  
  //Wire.begin();
  //delay(40);               // Wait time >40ms After VDD stable

  // finally, set # lines, font size, etc.
  normalFunctionSet();

  extendFunctionSet();
  command(LCD_EX_SETBIASOSC | LCD_BIAS_1_5 | LCD_OSC_183HZ);            // 1/5bias, OSC=183Hz@3.0V
  command(LCD_EX_FOLLOWERCONTROL | LCD_FOLLOWER_ON | LCD_RAB_2_00);     // internal follower circuit is turn on
  delay(200);                                       // Wait time >200ms (for power stable)
  normalFunctionSet();

  // turn the display on with no cursor or blinking default
//  display();
  _displaycontrol   = 0x00;//LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  setDisplayControl(LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
//  command(LCD_ENTRYMODESET | _displaymode);
  _displaymode      = 0x00;//LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  setEntryMode(LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
}

void ST7032::setContrast(uint8_t cont)
{
  extendFunctionSet();
  command(LCD_EX_CONTRASTSETL | (cont & 0x0f));                     // Contrast set
  command(LCD_EX_POWICONCONTRASTH | LCD_ICON_ON | LCD_BOOST_ON | ((cont >> 4) & 0x03)); // Power, ICON, Contrast control
  normalFunctionSet();
}

void ST7032::setIcon(uint8_t addr, uint8_t bit) {
  extendFunctionSet();
  command(LCD_EX_SETICONRAMADDR | (addr & 0x0f));                       // ICON address
  write(bit);
  normalFunctionSet();
}

/********** high level commands, for the user! */
void ST7032::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void ST7032::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void ST7032::setCursor(uint8_t col, uint8_t row)
{
  const int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

  if ( row > _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
  }
  
  command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void ST7032::noDisplay() {
  resetDisplayControl(LCD_DISPLAYON);
}
void ST7032::display() {
  setDisplayControl(LCD_DISPLAYON);
}

// Turns the underline cursor on/off
void ST7032::noCursor() {
  resetDisplayControl(LCD_CURSORON);
}
void ST7032::cursor() {
  setDisplayControl(LCD_CURSORON);
}

// Turn on and off the blinking cursor
void ST7032::noBlink() {
  resetDisplayControl(LCD_BLINKON);
}
void ST7032::blink() {
  setDisplayControl(LCD_BLINKON);
}

// These commands scroll the display without changing the RAM
void ST7032::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void ST7032::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void ST7032::leftToRight(void) {
  setEntryMode(LCD_ENTRYLEFT);
}

// This is for text that flows Right to Left
void ST7032::rightToLeft(void) {
  resetEntryMode(LCD_ENTRYLEFT);
}

// This will 'right justify' text from the cursor
void ST7032::autoscroll(void) {
  setEntryMode(LCD_ENTRYSHIFTINCREMENT);
}

// This will 'left justify' text from the cursor
void ST7032::noAutoscroll(void) {
  resetEntryMode(LCD_ENTRYSHIFTINCREMENT);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void ST7032::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

void ST7032::command(uint8_t value) {
  Wire.beginTransmission(_i2c_addr);
  Wire.write((uint8_t)0x00);
  Wire.write(value);
  Wire.endTransmission();
  delayMicroseconds(27);    // >26.3us
}

size_t ST7032::write(uint8_t value) {
  Wire.beginTransmission(_i2c_addr);
  Wire.write((uint8_t)0x40);
  Wire.write(value);
  Wire.endTransmission();
  delayMicroseconds(27);    // >26.3us

  return 1;
}

