#include "server_op.h"

#include <ESP8266mDNS.h>
#include <FS.h>
#include "config.h"
#include "IR_op.h"
#include "WiFi_op.h"
#include "server_op.h"

// TCP server at port 80 will respond to HTTP requests
ESP8266WebServer server(80);
String mdns_address = MDNS_ADDRESS_DEFAULT;

void serverTask() {
  server.handleClient();
}

void dispRequest() {
  println_dbg("");
  println_dbg("New Request");
  println_dbg("URI: " + server.uri());
  println_dbg("Method: " + (server.method() == HTTP_GET) ? "GET" : "POST");
  println_dbg("Arguments: " + String(server.args()));
  for (uint8_t i = 0; i < server.args(); i++) {
    println_dbg("  " + server.argName(i) + " = " + server.arg(i));
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void setupFormServer(void) {
  // Set up mDNS responder:
  print_dbg("mDNS address: ");
  println_dbg("http://" + String(MDNS_ADDRESS_SETUP) + ".local");
  if (!MDNS.begin(MDNS_ADDRESS_SETUP, WiFi.softAPIP())) {
    println_dbg("Error setting up MDNS responder!");
  } else {
    println_dbg("mDNS responder started");
  }

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
  });
  server.on("/confirm", []() {
    dispRequest();
    target_ssid = server.arg("ssid");
    target_pass = server.arg("password");
    mdns_address = server.arg("url");
    if (mdns_address == "") {
      mdns_address = MDNS_ADDRESS_DEFAULT;
    }
    println_dbg("Target SSID: " + target_ssid);
    println_dbg("Target Password: " + target_pass);
    println_dbg("mDNS Address: " + mdns_address);
    if (connectWifi()) {
      server.send(200, "text/palin", "true");
      mode = IR_STATION_MODE_STA;
      settingsBackupToFile();
      ESP.reset();
    } else {
      server.send(200, "text/plain", "false");
    }
  });
  server.on("/accessPointMode", []() {
    dispRequest();
    mdns_address = server.arg("url");
    if (mdns_address == "") {
      mdns_address = MDNS_ADDRESS_DEFAULT;
    }
    mode = IR_STATION_MODE_AP;
    settingsBackupToFile();
    server.send(200, "text/plain", "Setting up Access Point Successful");
    ESP.reset();
  });
  server.onNotFound([]() {
    // Request detail
    dispRequest();
    String path = server.uri();
    if (path.endsWith("/")) path += "form.html";
    if (SPIFFS.exists(path)) {
      println_dbg("file exists");
      File file = SPIFFS.open(path, "r");
      size_t sent = server.streamFile(file, getContentType(path));
      file.close();
      return;
    }
    println_dbg("file not found");
    server.send(404, "text/plain", "FileNotFound");
  });

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
      charEncode(ir[ch].chName);
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
  });
  server.on("/chName", []() {
    String res = "";
    res += "[";
    for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
      res += "\"" + ir[i].chName + "\"";
      if (i != IR_CH_SIZE - 1) res += ",";
    }
    res += "]";
    server.send(200, "text/json", res);
  });
  server.on("/clearAllSignals", []() {
    for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
      ir[i].period = 0;
      ir[i].chName = "ch " + String(i + 1, DEC);
      ir[i].irData = "";
      irDataBackupToFile(i);
    }
    server.send(200, "text/plain", "Cleared All Signals");
  });
  server.on("/disconnectWifi", []() {
    server.send(200, "text/json", "Disconnected this WiFi, Please connect again");
    println_dbg("Change WiFi SSID");
    mode = IR_STATION_MODE_NULL;
    settingsBackupToFile();
    ESP.reset();
  });
  server.on("/info", []() {
    String res = "";
    res += "[\"";
    res += "Listening...";
    res += "\",\"";
    if (mode == IR_STATION_MODE_STA)res += target_ssid;
    else res += SOFTAP_SSID;
    res += "\",\"";
    if (mode == IR_STATION_MODE_STA)res += (String)WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
    else res += (String)WiFi.softAPIP()[0] + "." + WiFi.softAPIP()[1] + "." + WiFi.softAPIP()[2] + "." + WiFi.softAPIP()[3];
    res += "\",\"";
    res += "http://" + mdns_address + ".local";
    res += "\"]";
    server.send(200, "text/json", res);
  });
  server.onNotFound([]() {
    // Request detail
    dispRequest();
    String path = server.uri();
    if (path.endsWith("/")) path += "index.html";
    if (SPIFFS.exists(path)) {
      println_dbg("file exists");
      File file = SPIFFS.open(path, "r");
      size_t sent = server.streamFile(file, getContentType(path));
      file.close();
      return;
    }
    println_dbg("file not found");
    server.send(404, "text/plain", "FileNotFound");
  });
  // Start TCP (HTTP) server
  server.begin();
  println_dbg("IR Station Server Listening");
}

void charEncode(String & s) {
  s.replace("+", " ");
  s.replace("%20", " ");  s.replace("%21", "!");  s.replace("%22", "\""); s.replace("%23", "#");
  s.replace("%24", "$");  s.replace("%25", "%");  s.replace("%26", "&");  s.replace("%27", "\'");
  s.replace("%28", "(");  s.replace("%29", ")");  s.replace("%2A", "*");  s.replace("%2B", "+");
  s.replace("%2C", ",");  s.replace("%2D", "-");  s.replace("%2E", ".");  s.replace("%2F", "/");
  s.replace("%3A", ":");  s.replace("%3B", ";");  s.replace("%3C", "<");  s.replace("%3D", "=");
  s.replace("%3E", ">");  s.replace("%3F", "?");  s.replace("%40", "@");  s.replace("%5B", "[");
  s.replace("%5C", "\\"); s.replace("%5D", "]");  s.replace("%5E", "^");  s.replace("%5F", "-");
  s.replace("%60", "`");  s.replace("%7B", "{");  s.replace("%7C", "|");  s.replace("%7D", "}");
  s.replace("%7E", "~");
}

