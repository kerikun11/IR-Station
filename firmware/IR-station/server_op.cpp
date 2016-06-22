#include "server_op.h"

#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <FS.h>
#include "config.h"
#include "IR_op.h"
#include "WiFi_op.h"
#include "server_op.h"

// TCP server at port 80 will respond to HTTP requests
ESP8266WebServer server(80);
String mdns_address = MDNS_ADDRESS_DEFAULT;

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

void serverTask() {
  server.handleClient();
  if (mode = IR_STATION_MODE_NULL) dnsServer.processNextRequest();
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
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    mdns_address = server.arg("url");
    if (mdns_address == "") {
      mdns_address = MDNS_ADDRESS_DEFAULT;
    }
    println_dbg("Target SSID: " + ssid);
    println_dbg("Target Password: " + password);
    println_dbg("mDNS Address: " + mdns_address);
    if (connectWifi(ssid, password)) {
      server.send(200, "text/palin", "true");
      setMode(IR_STATION_MODE_STA);
      ESP.reset();
    } else {
      server.send(200, "text/plain", "false");
      println_dbg("End");
    }
  });
  server.on("/accessPointMode", []() {
    dispRequest();
    mdns_address = server.arg("url");
    if (mdns_address == "") {
      mdns_address = MDNS_ADDRESS_DEFAULT;
    }
    server.send(200, "text/plain", "Setting up Access Point Successful");
    setMode(IR_STATION_MODE_AP);
    ESP.reset();
  });
  server.onNotFound([]() {
    // Request detail
    dispRequest();
    File file = SPIFFS.open("/form/index.html", "r");
    size_t sent = server.streamFile(file, "text/html");
    file.close();
    //    println_dbg("File not found");
    //    server.send(404, "text/plain", "FileNotFound");
    println_dbg("End");
  });

  server.serveStatic("/apple-touch-icon.png", SPIFFS, "/common/apple-touch-icon.png");
  server.serveStatic("/esp8266.png", SPIFFS, "/common/esp8266.png");
  server.serveStatic("/jquery-2.2.3.min.js", SPIFFS, "/common/jquery-2.2.3.min.js");

  server.serveStatic("/", SPIFFS, "/form/index.html");
  server.serveStatic("/hotspot-detect.html", SPIFFS, "/form/index.html");
  server.serveStatic("/main.js", SPIFFS, "/form/main.js");

  // Start TCP (HTTP) server
  server.begin();
  println_dbg("Setup Form Listening");
}

void setupServer(void) {
  // Set up mDNS responder:
  print_dbg("mDNS address: ");
  println_dbg("http://" + mdns_address + ".local");
  if (!MDNS.begin(mdns_address.c_str())) {
    println_dbg("Error setting up MDNS responder!");
  } else {
    println_dbg("mDNS responder started");
  }
  MDNS.addService("http", "tcp", 80);

  server.on("/send", []() {
    // Request detail
    dispRequest();
    String res;
    uint8_t ch = server.arg("ch").toInt();
    ch -= 1; // display: 1 ch ~ IR_CH_SIZE ch but data: 0 ch ~ (IR_CH_NAME - 1) ch so 1 decriment
    if (0 <= ch  && ch < IR_CH_SIZE) {
      res = "Sent a signal of ch " + String(ch + 1, DEC) + ": " + ir[ch].chName;
      irSendSignal(ch);
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
      if (irRecodeSignal(ch) == 0) {
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
      irDataBackupToFile(i);
    }
    println_dbg("Cleared All Signals");
    server.send(200, "text/plain", "Cleared All Signals");
    println_dbg("End");
  });
  server.on("/disconnectWifi", []() {
    dispRequest();
    server.send(200, "text/json", "Disconnected this WiFi, Please connect again");
    println_dbg("Change WiFi SSID");
    setMode(IR_STATION_MODE_NULL);
    ESP.reset();
  });
  server.on("/info", []() {
    dispRequest();
    String res = "";
    res += "[\"";
    res += "Listening...";
    res += "\",\"";
    if (mode == IR_STATION_MODE_STA)res += WiFi.SSID();
    else res += SOFTAP_SSID;
    res += "\",\"";
    if (mode == IR_STATION_MODE_STA) res += (String)WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
    else res += (String)WiFi.softAPIP()[0] + "." + WiFi.softAPIP()[1] + "." + WiFi.softAPIP()[2] + "." + WiFi.softAPIP()[3];
    res += "\",\"";
    res += "http://" + mdns_address + ".local";
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

  server.serveStatic("/apple-touch-icon.png", SPIFFS, "/common/apple-touch-icon.png");
  server.serveStatic("/esp8266.png", SPIFFS, "/common/esp8266.png");
  server.serveStatic("/jquery-2.2.3.min.js", SPIFFS, "/common/jquery-2.2.3.min.js");

  server.serveStatic("/", SPIFFS, "/general/index.html");
  server.serveStatic("/main.js", SPIFFS, "/general/main.js");

  // Start TCP (HTTP) server
  server.begin();
  println_dbg("IR Station Server Listening");
}

