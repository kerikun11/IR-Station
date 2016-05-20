#include "WiFi_op.h"

#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "config.h"
#include "IR_op.h"
#include "server_op.h"

String target_ssid = "NULL";
String target_pass = "NULL";

void setupAP(void) {
  wdt_reset();
  WiFi.mode(WIFI_AP_STA);
  println_dbg("Configuring Access Point...");
  WiFi.softAP(SOFTAP_SSID, SOFTAP_PASS);

  // display information
  print_dbg("AP SSID : ");
  println_dbg(SOFTAP_SSID);
  print_dbg("AP IP address: ");
  println_dbg(WiFi.softAPIP());
}

bool connectCachedWifi() {
  wdt_reset();
  // set WiFi Mode
  WiFi.mode(WIFI_STA);
  // Connect to WiFi network
  target_ssid = WiFi.SSID();
  //  int n = WiFi.scanNetworks();
  //  for (int i = 0; i < n; ++i) {
  //    println_dbg("SSID: " + String(WiFi.SSID(i)));
  //    if (target_ssid == String(WiFi.SSID(i))) {
  //      break;
  //    }
  //    if (i == n - 1) {
  //      println_dbg("");
  //      print_dbg("Couldn't find cached SSID: ");
  //      println_dbg(target_ssid);
  //      return false;
  //    }
  //  }
  println_dbg("");
  print_dbg("Connecting to cached SSID: ");
  println_dbg(target_ssid);
  WiFi.begin();

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
  println_dbg(target_ssid);
  print_dbg("IP address: ");
  println_dbg(WiFi.localIP());

  return true;
}

bool connectWifi() {
  wdt_reset();
  // set WiFi Mode
  WiFi.mode(WIFI_AP_STA);
  // Connect to WiFi network
  println_dbg("");
  print_dbg("Connecting to SSID: ");
  println_dbg(target_ssid);
  print_dbg("Password: ");
  println_dbg(target_pass);
  WiFi.begin(target_ssid.c_str(), target_pass.c_str());

  // Wait for connection
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    wdt_reset();
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
  println_dbg(target_ssid);
  print_dbg("IP address: ");
  println_dbg(WiFi.localIP());

  return true;
}

