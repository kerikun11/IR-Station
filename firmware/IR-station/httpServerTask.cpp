#include "httpServerTask.h"

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <FS.h>
#include "config.h"
#include "ir-stationTask.h"
#include "wifiTask.h"
#include "ledTask.h"

// TCP server at port 80 will respond to HTTP requests
ESP8266WebServer server(80);

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

void serverTask() {
  server.handleClient();
  if (station.mode == IR_STATION_MODE_NULL) dnsServer.processNextRequest();
}

void dispRequest() {
  println_dbg("");
  println_dbg("New Request");
  println_dbg("URI: " + server.uri());
  println_dbg(String("Method: ") + ((server.method() == HTTP_GET) ? "GET" : "POST"));
  println_dbg("Arguments: " + String(server.args()));
  for (uint8_t i = 0; i < server.args(); i++) {
    println_dbg("  " + server.argName(i) + " = " + server.arg(i));
  }
}

void setupFormServer(void) {
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/wifiList", []() {
    dispRequest();
    int n = WiFi.scanNetworks();
    String res = "[";
    for (int i = 0; i < n; ++i) {
      res += "\"" + WiFi.SSID(i) + "\"";
      if (i != n - 1) res += ",";
    }
    res += "]";
    server.send(200, "text/json", res);
    println_dbg("End");
  });
  server.on("/confirm", []() {
    dispRequest();
    station.ssid = server.arg("ssid");
    station.password = server.arg("password");
    station.mdns_hostname = server.arg("url");
    if (station.mdns_hostname == "") {
      station.mdns_hostname = MDNS_HOSTNAME_DEFAULT;
    }
    println_dbg("Target SSID: " + station.ssid);
    println_dbg("Target Password: " + station.password);
    println_dbg("mDNS Address: " + station.mdns_hostname);
    indicator.set(0, 1023, 0);
    server.send(200);
    WiFi.begin(station.ssid.c_str(), station.password.c_str());
  });
  server.on("/isConnected", []() {
    dispRequest();
    if (WiFi.status() == WL_CONNECTED) {
      String res = (String)WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
      server.send(200, "text/palin", res);
      station.setMode(IR_STATION_MODE_STA);
      indicator.set(0, 0, 1023);
      delay(5000);
      ESP.reset();
    } else {
      println_dbg("Not connected yet.");
      server.send(200, "text/plain", "false");
      println_dbg("End");
    }
  });
  server.on("/accessPointMode", []() {
    dispRequest();
    station.mdns_hostname = server.arg("url");
    if (station.mdns_hostname == "") {
      station.mdns_hostname = MDNS_HOSTNAME_DEFAULT;
    }
    server.send(200, "text/plain", "Setting up Access Point Successful");
    station.setMode(IR_STATION_MODE_AP);
    ESP.reset();
  });
  server.onNotFound([]() {
    // Request detail
    dispRequest();
    println_dbg("Redirect");
    String res = "<script>location.href = \"http://" + (String)WiFi.softAPIP()[0] + "." + WiFi.softAPIP()[1] + "." + WiFi.softAPIP()[2] + "." + WiFi.softAPIP()[3] + "/\";</script>";
    server.send(200, "text/html", res);
    println_dbg("End");
  });

  server.serveStatic("/", SPIFFS, "/form/");

  // Start TCP (HTTP) server
  server.begin();
  println_dbg("Form Server Listening");
}

void setupServer(void) {
  // Set up mDNS responder:
  if (station.mdns_hostname == "") station.mdns_hostname = MDNS_HOSTNAME_DEFAULT;
  print_dbg("mDNS address: ");
  println_dbg("http://" + station.mdns_hostname + ".local");
  if (!MDNS.begin(station.mdns_hostname.c_str())) {
    println_dbg("Error setting up MDNS responder!");
  } else {
    println_dbg("mDNS responder started");
  }

  server.on("/send", []() {
    // Request detail
    dispRequest();
    String res;
    uint8_t ch = server.arg("ch").toInt();
    ch -= 1; // display: 1 ch ~ IR_CH_SIZE ch but data: 0 ch ~ (IR_CH_NAME - 1) ch so 1 decriment
    if (0 <= ch  && ch < IR_CH_SIZE) {
      res = "Sent a signal of ch " + String(ch + 1, DEC) + ": " + ir[ch].chName;
      station.irSendSignal(ch);
    } else {
      res = "Invalid channel selected. Sending failed";
      println_dbg("Invalid channel selected. Sending failed.");
    }
    // Send the response
    server.send(200, "text/plain", res);
    println_dbg("End");
  });
  server.on("/recode", []() {
    // Request detail
    dispRequest();
    String status;
    // Match the request
    uint8_t ch = server.arg("ch").toInt();
    ch -= 1; // display: 1 ch ~ IR_CH_SIZE ch but data: 0 ch ~ (IR_CH_NAME - 1) ch so 1 decriment
    if (0 <= ch  && ch < IR_CH_SIZE) {
      String chName = server.arg("chName");
      if (chName == "") chName = "ch " + String(ch + 1, DEC);
      ir[ch].chName = chName;
      if (station.irRecodeSignal(ch) == 0) {
        status = "Recoding Successful: ch " + String(ch + 1);
      } else {
        status = "No Signal Recieved";
      }
    } else {
      status = "Invalid Request";
    }
    // Send the response
    server.send(200, "text/plain", status);
    println_dbg("End");
  });
  server.on("/chName", []() {
    dispRequest();
    String res = "";
    res += "[";
    for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
      res += "\"" + ir[i].chName + "\"";
      if (i != IR_CH_SIZE - 1) res += ",";
    }
    res += "]";
    server.send(200, "text/json", res);
    println_dbg("End");
  });
  server.on("/clearAllSignals", []() {
    dispRequest();
    for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
      ir[i].period = 0;
      ir[i].chName = "ch " + String(i + 1, DEC);
      ir[i].irData = "";
      station.irDataBackupToFile(i);
    }
    println_dbg("Cleared All Signals");
    server.send(200, "text/plain", "Cleared All Signals");
    println_dbg("End");
  });
  server.on("/disconnectWifi", []() {
    dispRequest();
    server.send(200, "text/json", "Disconnected this WiFi, Please connect again");
    println_dbg("Change WiFi SSID");
    station.reset();
    ESP.reset();
  });
  server.on("/info", []() {
    dispRequest();
    String res = "";
    res += "[\"";
    res += "Listening...";
    res += "\",\"";
    if (station.mode == IR_STATION_MODE_STA)res += WiFi.SSID();
    else res += SOFTAP_SSID;
    res += "\",\"";
    if (station.mode == IR_STATION_MODE_STA) res += (String)WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
    else res += (String)WiFi.softAPIP()[0] + "." + WiFi.softAPIP()[1] + "." + WiFi.softAPIP()[2] + "." + WiFi.softAPIP()[3];
    res += "\",\"";
    res += "http://" + station.mdns_hostname + ".local";
    res += "\"]";
    server.send(200, "text/json", res);
    println_dbg("End");
  });
  server.onNotFound([]() {
    // Request detail
    dispRequest();
    println_dbg("File not found");
    server.send(404, "text/plain", "FileNotFound");
    println_dbg("End");
  });

  server.serveStatic("/", SPIFFS, "/general/", "public");

  // Start TCP (HTTP) server
  server.begin();
  println_dbg("IR Station Server Listening");
}

