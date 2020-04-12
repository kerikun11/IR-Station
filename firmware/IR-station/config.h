/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* Version */
#define IR_STATION_VERSION      "v1.6.0"

/* Hardware Mapping */
#define PIN_BUTTON              (0)
#define PIN_IR_IN               (5)
#define PIN_IR_OUT              (14)

#define PIN_LED_R               (12)
#define PIN_LED_G               (15)
#define PIN_LED_B               (13)

/* Software */
// for Remocon
#define SIGNAL_COUNT_DEFAULT    (25)
#define SIGNAL_COUNT_MAX        (100)

// SSID & Password of ESP8266 Access Point Mode
#define SOFTAP_SSID             "IR-Station"
#define SOFTAP_PASS             "IR-Station"

// WiFi connection Timeout
#define WIFI_CONNECT_TIMEOUT    (10) // seconds

// http://HOSTNAME_DEFAULT.local
#define HOSTNAME_DEFAULT        "ir"

// OTA update
#define USE_OTA_UPDATE          true

// Capital Portal
#define USE_CAPITAL_PORTAL      true

// Alexa
#define USE_ALEXA               true

/* for Debug */
#define SERIAL_DEBUG            true

#if SERIAL_DEBUG == true
#define DEBUG_SERIAL_STREAM     Serial
#define print_dbg               DEBUG_SERIAL_STREAM.print
#define printf_dbg              DEBUG_SERIAL_STREAM.printf
#define println_dbg             DEBUG_SERIAL_STREAM.println
#else
#define DEBUG_SERIAL_STREAM     NULL
#define print_dbg               // No Operation
#define printf_dbg              // No Operation
#define println_dbg             // No Operation
#endif

#endif
