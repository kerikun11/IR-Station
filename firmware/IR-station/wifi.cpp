#include "wifi.h"

#include "config.h"

void setupAP(String ssid, String password) {
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

bool connectWifi(String ssid, String password) {
  wdt_reset();
  if (WiFi.status() == WL_CONNECTED) {
    if ((ssid == (String)WiFi.SSID()) && (password == (String)WiFi.psk())) {
      println_dbg("Already connected!");
      return true;
    }
  }

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

