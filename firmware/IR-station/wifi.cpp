/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "wifi.h"

#include "config.h" // for print_dbg()

void setupAP(const String& ssid, const String& password) {
  wdt_reset();
  println_dbg("Configuring Access Point...");
  WiFi.softAP(ssid.c_str(), password.c_str());

  // display information
  print_dbg("AP SSID : ");
  println_dbg(ssid);
  print_dbg("AP Password : ");
  println_dbg(password);
  print_dbg("AP IP address: ");
  println_dbg(WiFi.softAPIP());
}

bool connectWifi(const String& ssid, const String& password, bool stealth) {
  wdt_reset();
  if (WiFi.status() == WL_CONNECTED) {
    if ((ssid == (String)WiFi.SSID()) && (password == (String)WiFi.psk())) {
      println_dbg("Already connected:" + ssid);
      print_dbg("IP address: ");
      println_dbg(WiFi.localIP());
      return true;
    }
  }

  if (!stealth) {
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
      println_dbg("SSID: " + String(WiFi.SSID(i)));
      if (ssid == String(WiFi.SSID(i))) {
        break;
      }
      if (i == n - 1) {
        println_dbg("");
        print_dbg("Couldn't find SSID: ");
        println_dbg(ssid);
        return false;
      }
    }
  }
  println_dbg("");
  print_dbg("Connecting to SSID: ");
  println_dbg(ssid);
  WiFi.begin(ssid.c_str(), password.c_str());

  // Wait for connection
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    ESP.wdtFeed();
    delay(500);
    print_dbg(".");
    timeout++;
    if (timeout >= 2 * WIFI_CONNECT_TIMEOUT) {
      println_dbg("");
      println_dbg("Invalid SSID or Password");
      println_dbg("WiFi Connection Failed");
      return false;
    }
  }
  println_dbg("");
  print_dbg("Connected to ");
  println_dbg(ssid);
  print_dbg("IP address: ");
  println_dbg(WiFi.localIP());

  return true;
}

