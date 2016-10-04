/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "station.h"

#include <ArduinoJson.h>
#include <FS.h>
#include "file.h"
#include "wifi.h"
#include "crc8.h"

void IR_Station::begin(void) {
  yield();
  wdt_reset();
  indicator.green(1023);

  if (restore() == false) reset();

#if USE_OTA_UPDATE == true
  ota.begin(hostname);
#endif

  switch (mode) {
    case IR_STATION_MODE_SETUP:
      println_dbg("Boot Mode: Setup");
      WiFi.mode(WIFI_AP_STA);
      setupAP(SOFTAP_SSID, SOFTAP_PASS);
      attachSetupApi();
      break;
    case IR_STATION_MODE_STATION:
      println_dbg("Boot Mode: Station");
      WiFi.mode(WIFI_STA);
      if (is_static_ip) WiFi.config(local_ip, gateway, subnetmask);
      connectWifi(ssid, password, is_stealth_ssid);
      if (WiFi.localIP() != local_ip) {
        is_static_ip = false;
        save();
      }
      attachStationApi();
      indicator.set(0, 0, 1023);
      break;
    case IR_STATION_MODE_AP:
      println_dbg("Boot Mode: Access Point");
      WiFi.mode(WIFI_AP_STA);
      setupAP(SOFTAP_SSID, SOFTAP_PASS);
      attachStationApi();
      indicator.set(0, 0, 1023);
      break;
  }

  if (!MDNS.begin(hostname.c_str())) println_dbg("Error setting up MDNS responder!");
  else println_dbg("mDNS: http://" + hostname + ".local");

#if USE_CAPITAL_PORTAL == true
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
#endif

  httpUpdater.setup(&server, "/firmware");
  server.on("/description.xml", HTTP_GET, [this]() {
    displayRequest();
    SSDP.schema(server.client());
  });

  server.begin();

  println_dbg("Starting SSDP...");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(hostname);
  SSDP.setSerialNumber(String(ESP.getChipId() , HEX));
  SSDP.setURL("index.htm");
  SSDP.setModelName("IR-Station");
  SSDP.setModelNumber("20160821");
  SSDP.setModelURL("https://github.com/kerikun11/IR-station");
  SSDP.setManufacturer("KERI's Lab");
  SSDP.setManufacturerURL("http://kerikeri.top");
  SSDP.begin();
}

void IR_Station::reset() {
  version = IR_STATION_VERSION;
  mode = IR_STATION_MODE_SETUP;
  hostname = HOSTNAME_DEFAULT;

  is_stealth_ssid = false;
  ssid = "";
  password = "";

  is_static_ip = false;
  local_ip = 0U;
  subnetmask = 0U;
  gateway = 0U;

  next_id = 0;

  for (int i = 0; i < signals.size(); i++) {
    removeFile(signals[i].path);
  }
  signals.resize(0);

  save();
  ESP.reset();
}

void IR_Station::disconnect() {
  version = IR_STATION_VERSION;
  mode = IR_STATION_MODE_SETUP;
  hostname = HOSTNAME_DEFAULT;

  is_stealth_ssid = false;
  ssid = "";
  password = "";

  is_static_ip = false;
  local_ip = 0U;
  subnetmask = 0U;
  gateway = 0U;

  //  next_id = 0;
  //  signals.resize(0);

  save();
  ESP.reset();
}

void IR_Station::handle() {
  server.handleClient();
  ir.handle();
  ota.handle();
  switch (mode) {
    case IR_STATION_MODE_SETUP:
      if ((WiFi.status() == WL_CONNECTED)) indicator.set(0, 1023, 1023);
      dnsServer.processNextRequest();
      break;
    case IR_STATION_MODE_STATION:
      static bool lost = false;
      if ((WiFi.status() != WL_CONNECTED)) {
        if (lost == false) {
          println_dbg("Lost WiFi: " + ssid);
          WiFi.mode(WIFI_AP_STA);
          delay(1000);
          setupAP(SOFTAP_SSID, SOFTAP_PASS);
          indicator.set(1023, 0, 0);
        }
        lost = true;
      } else {
        if (lost == true) {
          println_dbg("Found WiFi: " + ssid);
          WiFi.mode(WIFI_STA);
          indicator.set(0, 0, 1023);
        }
        lost = false;
      }
      break;
    case IR_STATION_MODE_AP:
      dnsServer.processNextRequest();
      break;
  }
}

int IR_Station::getNewId() {
  return next_id++;
}

bool IR_Station::save() {
  yield();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["version"] = version;
  root["mode"] = mode;
  root["hostname"] = hostname;

  root["is_stealth_ssid"] = is_stealth_ssid;
  root["ssid"] = ssid;
  root["password"] = password;

  root["is_static_ip"] = is_static_ip;
  root["local_ip"] = (const uint32_t)local_ip;
  root["subnetmask"] = (const uint32_t)subnetmask;
  root["gateway"] = (const uint32_t)gateway;

  root["next_id"] = next_id;

  JsonArray& _signals = root.createNestedArray("signals");
  for (int i = 0; i < signals.size(); i++) {
    JsonObject& _signal = jsonBuffer.createObject();
    _signal["id"] = signals[i].id;
    _signal["name"] = signals[i].name;
    _signal["path"] = signals[i].path;
    _signal["display"] = signals[i].display;
    JsonObject &_position = _signal.createNestedObject("position");
    _position["row"] = signals[i].position.row;
    _position["column"] = signals[i].position.column;
    _signals.add(_signal);
  }

  String str = "";
  root.printTo(str);
  return writeStringToFile(STATION_JSON_PATH, str);
}

void IR_Station::displayRequest() {
  yield();
  println_dbg("");
  println_dbg("New Request");
  println_dbg("URI: " + server.uri());
  println_dbg(String("Method: ") + ((server.method() == HTTP_GET) ? "GET" : "POST"));
  println_dbg("Arguments count: " + String(server.args()));
  for (uint8_t i = 0; i < server.args(); i++) {
    println_dbg("\t" + server.argName(i) + " = " + server.arg(i));
  }
}

void IR_Station::attachSetupApi() {
  server.on("/wifi/list", [this]() {
    displayRequest();
    int n = WiFi.scanNetworks();
    String res = "[";
    for (int i = 0; i < n; ++i) {
      res += "\"" + WiFi.SSID(i) + "\"";
      if (i != n - 1) res += ",";
    }
    res += "]";
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.on("/wifi/confirm", [this]() {
    displayRequest();
    if (WiFi.status() == WL_CONNECTED) {
      String res = (String)WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
      server.send(200, "text/palin", res);
      mode = IR_STATION_MODE_STATION;
      local_ip = WiFi.localIP();
      subnetmask = WiFi.subnetMask();
      gateway = WiFi.gatewayIP();
      save();
      indicator.set(0, 0, 1023);
      delay(1000);
      ESP.reset();
    } else {
      println_dbg("Not connected yet.");
      server.send(200, "text/plain", "false");
      println_dbg("End");
    }
  });
  server.on("/mode/station", [this]() {
    displayRequest();
    ssid = server.arg("ssid");
    password = server.arg("password");
    is_stealth_ssid = server.arg("stealth") == "true";
    hostname = server.arg("hostname");
    if (hostname == "") hostname = HOSTNAME_DEFAULT;
    println_dbg("Hostname: " + hostname);
    println_dbg("SSID: " + ssid + " Password: " + password + " Stealth: " + (is_stealth_ssid ? "true" : "false"));
    indicator.set(0, 1023, 0);
    server.send(200);
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(ssid.c_str(), password.c_str());
  });
  server.on("/mode/accesspoint", [this]() {
    displayRequest();
    server.send(200, "text / plain", "Setting up Access Point Successful");
    hostname = server.arg("hostname");
    if (hostname == "") hostname = HOSTNAME_DEFAULT;
    mode = IR_STATION_MODE_AP;
    save();
    ESP.reset();
  });
  server.on("/dbg", [this]() {
    displayRequest();
    ssid = server.arg("ssid");
    password = server.arg("password");
    println_dbg("Target SSID : " + ssid);
    println_dbg("Target Password : " + password);
    indicator.set(0, 1023, 0);
    if (connectWifi(ssid.c_str(), password.c_str())) {
      String res = (String)WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
      server.send(200, "text/plain", res);
    } else {
      server.send(200, "text/plain", "Connection Failed");
    }
  });
  server.onNotFound([this]() {
    displayRequest();
    println_dbg("Redirect");
    String res = "<script>location.href = \"http://" + (String)WiFi.softAPIP()[0] + "." + WiFi.softAPIP()[1] + "." + WiFi.softAPIP()[2] + "." + WiFi.softAPIP()[3] + "/\";</script>";
    server.send(200, "text/html", res);
    println_dbg("End");
  });
  server.serveStatic("/", SPIFFS, "/setup/");
}

void IR_Station::attachStationApi() {
  server.on("/info", [this]() {
    displayRequest();
    String res;
    if (getStringFromFile(STATION_JSON_PATH, res)) {
      return server.send(200, "application/json", res);
    } else {
      return server.send(500, "text/plain", "Failed to open File");
    }
  });
  server.on("/signals/send", [this]() {
    displayRequest();
    int id = server.arg("id").toInt();
    for (int i = 0; i < signals.size(); i++) {
      if (signals[i].id == id) {
        String json;
        if (!getStringFromFile(signals[i].path, json)) return server.send(500, "text/plain", "Failed to open File");
        indicator.set(0, 1023, 0);
        ir.send(json);
        indicator.set(0, 0, 1023);
        return server.send(200, "text/plain", "Sending Successful: " + signals[i].name);
      }
    }
    return server.send(400, "text/plain", "No signal assigned");
  });
  server.on("/signals/record", [this]() {
    displayRequest();
    const uint32_t timeout_ms = 5000;
    indicator.set(0, 1023, 0);
    ir.resume();
    int timeStamp = millis();
    while (!ir.available()) {
      wdt_reset();
      ir.handle();
      if (millis() - timeStamp > timeout_ms) {
        indicator.set(1023, 0, 0);
        return server.send(500, "text/plain", "No Signal Recieved");
      }
    }
    indicator.set(0, 0, 1023);
    String data = ir.read();
    Signal signal;
    signal.id = getNewId();
    signal.name = server.arg("name");
    signal.path = IR_DATA_PATH(signal.id);
    signal.display = (server.arg("display") == "true");
    signal.position.row = server.arg("row").toInt();
    signal.position.column = server.arg("column").toInt();

    if (!writeStringToFile(signal.path, data)) return server.send(500, "text/plain", "Failed to write File");
    signals.push_back(signal);
    save();
    return server.send(200, "text/plain", "Recording Successful: " + signal.name);
  });
  server.on("/signals/rename", [this]() {
    displayRequest();
    int id = server.arg("id").toInt();
    for (int i = 0; i < signals.size(); i++) {
      if (signals[i].id == id) {
        String prev_name = signals[i].name;
        signals[i].name = server.arg("name");
        save();
        return server.send(200, "text/plain", "Renamed " + prev_name + " to " + signals[i].name);
      }
    }
    return server.send(400, "text/plain", "No signal assigned");
  });
  server.on("/signals/upload", [this]() {
    displayRequest();
    Signal signal;
    signal.id = getNewId();
    signal.name = server.arg("name");
    signal.path = IR_DATA_PATH(signal.id);
    signal.display = (server.arg("display") == "true");
    signal.position.row = server.arg("row").toInt();
    signal.position.column = server.arg("column").toInt();

    if (!writeStringToFile(signal.path, server.arg("irJson"))) return server.send(500, "text/plain", "Failed to write File");
    signals.push_back(signal);
    save();
    return server.send(200, "text/plain", "Recording Successful: " + signal.name);
  });
  server.on("/signals/clear", [this]() {
    displayRequest();
    int id = server.arg("id").toInt();
    for (int i = 0; i < signals.size(); i++) {
      if (signals[i].id == id) {
        if (!removeFile(signals[i].path)) return server.send(500, "text/plain", "Failed to Delete File");
        signals.erase(signals.begin() + i);
        save();
        return server.send(200, "text/plain", "Deleted");
      }
    }
    return server.send(400, "text/plain", "No signal assigned");
  });
  server.on("/signals/clear-all", [this]() {
    displayRequest();
    for (int i = 0; i < signals.size(); i++) {
      removeFile(signals[i].path);
    }
    signals.resize(0);
    save();
    return server.send(200, "text/plain", "Cleared All Signals");
  });
  server.on("/wifi/disconnect", [this]() {
    displayRequest();
    server.send(200);
    delay(100);
    disconnect();
  });
  server.on("/wifi/change-ip", [this]() {
    displayRequest();
    IPAddress _local_ip, _subnetmask, _gateway;
    if (!_local_ip.fromString(server.arg("local_ip")) || !_subnetmask.fromString(server.arg("netmask")) || !_gateway.fromString(server.arg("gateway"))) {
      return server.send(400, "text/palin", "Bad Request!");
    }
    WiFi.config(_local_ip, _gateway, _subnetmask);
    if (WiFi.localIP() != local_ip) {
      return server.send(500, "text/palin", "Couldn't change IP Address :(");
    }
    is_static_ip = true;
    local_ip = _local_ip;
    subnetmask = _subnetmask;
    gateway = _gateway;
    save();
    return server.send(200, "text/palin", "Changed IP Address to " + WiFi.localIP().toString());
  });
  server.onNotFound([this]() {
    displayRequest();
    String res = "<script>location.href = \"http://" + WiFi.localIP().toString() + "/\";</script>";
    server.send(200, "text/html", res);
    println_dbg("End");
  });
  server.serveStatic("/", SPIFFS, "/main/", "public");
}

bool IR_Station::restore() {
  yield();
  String s;
  if (getStringFromFile(STATION_JSON_PATH, s) == false) return false;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(s);

  version = (const char *)root["version"];
  mode = (int)root["mode"];
  hostname = (const char*)root["hostname"];

  is_stealth_ssid = (bool)root["is_stealth_ssid"];
  ssid = (const char*)root["ssid"];
  password = (const char*)root["password"];

  is_static_ip = (bool)root["is_static_ip"];
  local_ip = (const uint32_t)root["local_ip"];
  subnetmask = (const uint32_t)root["subnetmask"];
  gateway = (const uint32_t)root["gateway"];

  next_id = (int)root["next_id"];

  for (int i = 0; i < root["signals"].size(); i++) {
    Signal signal;
    signal.id = (int)root["signals"][i]["id"];
    signal.name = (const char *)root["signals"][i]["name"];
    signal.path = (const char *)root["signals"][i]["path"];
    signal.display = (bool)root["signals"][i]["display"];
    signal.position.row = (int)root["signals"][i]["position"]["row"];
    signal.position.column = (int)root["signals"][i]["position"]["column"];
    signals.push_back(signal);
  }

  if (version != IR_STATION_VERSION) {
    println_dbg("version difference");
    return false;
  }
  return true;
}

