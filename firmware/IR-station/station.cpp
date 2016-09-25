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

  if (settingsRestoreFromFile() == false) reset();

  println_dbg("attached button interrupt");

  // setup OTA
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
      restoreSignalName();
      WiFi.mode(WIFI_STA);
      if (is_static_ip) WiFi.config(local_ip, gateway, subnetmask);
      connectWifi(ssid, password, is_stealth_ssid);
      if (WiFi.localIP() != local_ip) {
        is_static_ip = false;
        settingsBackupToFile();
      }
      attachStationApi();
      indicator.green(0);
      indicator.blue(1023);
      break;
    case IR_STATION_MODE_AP:
      println_dbg("Boot Mode: AP");
      restoreSignalName();
      WiFi.mode(WIFI_AP_STA);
      setupAP(SOFTAP_SSID, SOFTAP_PASS);
      attachStationApi();
      indicator.green(0);
      indicator.blue(1023);
      break;
  }

  if (!MDNS.begin(hostname.c_str())) println_dbg("Error setting up MDNS responder!");
  else println_dbg("mDNS: http://" + hostname + ".local");

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

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
  yield();
  mode = IR_STATION_MODE_SETUP;
  hostname = HOSTNAME_DEFAULT;
  is_stealth_ssid = false;
  ssid = "";
  password = "";
  is_static_ip = false;
  local_ip = 0U;
  subnetmask = 0U;
  gateway = 0U;
  signalCount = SIGNAL_COUNT_DEFAULT;
  settingsBackupToFile();
  ESP.reset();
}

void IR_Station::disconnect() {
  mode = IR_STATION_MODE_SETUP;
  is_stealth_ssid = false;
  ssid = "";
  password = "";
  is_static_ip = false;
  signalCount = SIGNAL_COUNT_DEFAULT;
  settingsBackupToFile();
  ESP.reset();
}

void IR_Station::handle() {
  server.handleClient();
  ir.handle();
  ota.handle();
  switch (mode) {
    case IR_STATION_MODE_SETUP:
      if ((WiFi.status() == WL_CONNECTED)) indicator.set(0, 0, 1023);
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

bool IR_Station::changeIp(String local_ip_s, String gateway_s, String subnetmask_s) {
  if (local_ip.fromString(local_ip_s) == false) {
    return false;
  }
  gateway.fromString(gateway_s);
  subnetmask.fromString(subnetmask_s);
  WiFi.config(local_ip, gateway, subnetmask);
  if (WiFi.localIP() != local_ip) {
    return false;
  }
  is_static_ip = true;
  settingsBackupToFile();
  return true;
}

void IR_Station::restoreSignalName(void) {
  for (int ch = 1; ch <= SIGNAL_COUNT_MAX; ch++) {
    String json;
    if (getStringFromFile(IR_DATA_PATH(ch), json)) {
      DynamicJsonBuffer jsonBuffer;
      JsonObject& data = jsonBuffer.parseObject(json);
      signalName[ch] = (const char*)data["name"];
    }
  }
}

bool IR_Station::irSendSignal(int ch) {
  String json;
  if (getStringFromFile(IR_DATA_PATH(ch), json)) {
    indicator.set(0, 1023, 0);
    ir.send(json);
    indicator.set(0, 0, 1023);
    return true;
  }
  return false;
}

bool IR_Station::irRecordSignal(int id, String name, uint32_t timeout_ms) {
  indicator.set(0, 1023, 0);
  ir.resume();
  int timeStamp = millis();
  while (!ir.available()) {
    wdt_reset();
    ir.handle();
    if (millis() - timeStamp > timeout_ms) {
      indicator.set(1023, 0, 0);
      return false;
    }
  }
  String data = ir.read();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data);
  root["name"] = name;
  data = "";
  root.printTo(data);
  indicator.set(0, 0, 1023);
  return writeStringToFile(IR_DATA_PATH(id), data);
}

bool IR_Station::clearSignal(int ch) {
  return removeFile(IR_DATA_PATH(ch));
}

bool IR_Station::renameSignal(int ch, String name) {
  String data;
  if (!getStringFromFile(IR_DATA_PATH(ch), data)) {
    return false;
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data);
  root["name"] = name;
  data = "";
  root.printTo(data);
  return writeStringToFile(IR_DATA_PATH(ch), data);
}

bool IR_Station::uploadSignal(int ch, String data) {
  return writeStringToFile(IR_DATA_PATH(ch), data);
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

String IR_Station::resultJson(int code, String message) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["message"] = message;
  root["code"] = code;
  String str;
  root.printTo(str);
  return str;
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
      settingsBackupToFile();
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
    settingsBackupToFile();
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
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["message"] = "Listening...";
    root["ssid"] = (mode == IR_STATION_MODE_STATION) ? WiFi.SSID() : SOFTAP_SSID;
    IPAddress ip = (mode == IR_STATION_MODE_STATION) ? WiFi.localIP() : WiFi.softAPIP();
    root["ipaddress"] = ip.toString();
    root["hostname"] = hostname;
    String res;
    root.printTo(res);
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.on("/signals/list", [this]() {
    displayRequest();
    DynamicJsonBuffer jsonBuffer;
    JsonArray& data = jsonBuffer.createArray();
    for (int i = 1; i <= signalCount; i++) {
      data.add(signalName[i]);
    }
    String res;
    data.printTo(res);
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.on("/signals/send", [this]() {
    displayRequest();
    String res;
    int ch = server.arg("ch").toInt();
    if (ch > 0 && irSendSignal(ch)) {
      res = resultJson(0, "Sending Successful");
    } else {
      res = resultJson(-1, "No signal was sent");
    }
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.on("/signals/record", [this]() {
    displayRequest();
    String res;
    int ch = server.arg("ch").toInt();
    if (ch > 0 && irRecordSignal(ch, server.arg("name"))) {
      res = resultJson(0, "Recording Successful");
      signalName[ch] = server.arg("name");
    } else {
      res = resultJson(-1, "No Signal Recieved");
    }
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.on("/signals/rename", [this]() {
    displayRequest();
    String res;
    int ch = server.arg("ch").toInt();
    if (ch > 0 && renameSignal(ch, server.arg("name"))) {
      signalName[ch] = server.arg("name");
      res = resultJson(0, "Rename Successful");
    } else {
      res = resultJson(-1, "Rename Failed");
    }
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.on("/signals/upload", [this]() {
    displayRequest();
    String res;
    int ch = server.arg("ch").toInt();
    if (ch > 0 && uploadSignal(ch, server.arg("irJson"))) {
      res = resultJson(0, "Upload Successful");
    } else {
      res = resultJson(-1, "Upload Failed");
    }
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.on("/signals/clear", [this]() {
    displayRequest();
    String res;
    int ch = server.arg("ch").toInt();
    if (ch > 0 && clearSignal(ch)) {
      res = resultJson(0, "Cleared a Singal");
    } else {
      res = resultJson(-1, "Failed to clear");
    }
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.on("/signals/clear-all", [this]() {
    displayRequest();
    for (int ch = 1; ch <= SIGNAL_COUNT_MAX; ch++) {
      clearSignal(ch);
    }
    server.send(200, "application/json", resultJson(0, "Cleared All Signals"));
    println_dbg("End");
  });
  server.on("/signals/number", [this]() {
    displayRequest();
    signalCount = server.arg("number").toInt();
    settingsBackupToFile();
    server.send(200, "application/json", resultJson(0, "Updated a number of channels"));
    println_dbg("End");
  });
  server.on("/wifi/disconnect", [this]() {
    displayRequest();
    server.send(200);
    delay(100);
    disconnect();
  });
  server.on("/wifi/change-ip", [this]() {
    displayRequest();
    String res;
    if (changeIp(server.arg("ipaddress"), server.arg("gateway"), server.arg("netmask"))) {
      res = resultJson(0, "Changed IP to " + WiFi.localIP().toString());
    } else {
      res = resultJson(-1, "Failed to Change IP Address");
    }
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.onNotFound([this]() {
    displayRequest();
    String res = "<script>location.href = \"http://" + WiFi.localIP().toString() + "/\";</script>";
    server.send(200, "text/html", res);
    println_dbg("End");
  });
  server.serveStatic("/", SPIFFS, "/main/", "public");
}

String IR_Station::settingsCrcSerial() {
  return String(mode, DEC) + hostname + String(is_stealth_ssid, DEC) + ssid + password + String(is_static_ip, DEC) + local_ip + subnetmask + gateway + String(signalCount, DEC);
}

bool IR_Station::settingsRestoreFromFile() {
  yield();
  String s;
  if (getStringFromFile(SETTINGS_DATA_PATH, s) == false) return false;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(s);
  mode = (int)root["mode"];
  hostname = (const char*)root["hostname"];
  is_stealth_ssid = (bool)root["is_stealth_ssid"];
  ssid = (const char*)root["ssid"];
  password = (const char*)root["password"];
  is_static_ip = (bool)root["is_static_ip"];
  local_ip = (const uint32_t)root["local_ip"];
  subnetmask = (const uint32_t)root["subnetmask"];
  gateway = (const uint32_t)root["gateway"];
  signalCount = (int)root["signalsCount"];
  uint8_t crc = (uint8_t)root["crc"];
  String serial = settingsCrcSerial();
  if (crc != crc8((uint8_t*)serial.c_str(), serial.length(), CRC8INIT)) {
    println_dbg("CRC8 difference");
    return false;
  }
  println_dbg("CRC8 OK");
  return true;
}

bool IR_Station::settingsBackupToFile() {
  yield();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["mode"] = mode;
  root["hostname"] = hostname;
  root["is_stealth_ssid"] = is_stealth_ssid;
  root["ssid"] = ssid;
  root["password"] = password;
  root["is_static_ip"] = is_static_ip;
  root["local_ip"] = (const uint32_t)local_ip;
  root["subnetmask"] = (const uint32_t)subnetmask;
  root["gateway"] = (const uint32_t)gateway;
  root["signalsCount"] = signalCount;
  String serial = settingsCrcSerial();
  root["crc"] = crc8((uint8_t*)serial.c_str(), serial.length(), CRC8INIT);
  String str = "";
  root.printTo(str);
  return writeStringToFile(SETTINGS_DATA_PATH, str);
}

