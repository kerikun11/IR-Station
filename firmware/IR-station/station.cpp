/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "station.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include "file.h"
#include "wifi.h"
#include "ntp.h"
#include "wpa.h"

void IR_Station::begin() {
  wdt_reset();

  // begin setup
  indicator.green(1023);

  // prepare internal filesystem
  LittleFS.begin();

  // restore settings
  if (restore() == false) reset();

  switch (mode) {
    case IR_STATION_MODE_SETUP:
      println_dbg("Boot Mode: Setup");
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin();
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
        local_ip = WiFi.localIP();
        subnetmask = WiFi.subnetMask();
        gateway = WiFi.gatewayIP();
        save();
      }
      attachStationApi();
      ntp_begin();
      indicator.set(0, 0, 1023);
      break;
    case IR_STATION_MODE_AP:
      println_dbg("Boot Mode: Access Point");
      WiFi.mode(WIFI_AP_STA);
      setupAP(SOFTAP_SSID, SOFTAP_PASS);
      attachStationApi();
      schedules.resize(0);
      save();
      indicator.set(0, 0, 1023);
      break;
  }

  if (!MDNS.begin(hostname.c_str())) println_dbg("Error setting up MDNS responder!");
  else println_dbg("mDNS: http://" + hostname + ".local");

#if USE_CAPITAL_PORTAL == true
  println_dbg("Starting Capital Portal...");
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
#endif

#if USE_OTA_UPDATE == true
  ota.begin(hostname);
#endif

  println_dbg("Starting HTTP Updater...");
  httpUpdater.setup(&server, "/firmware");
  server.on("/description.xml", HTTP_GET, [this]() {
    displayRequest();
    SSDP.schema(server.client());
  });

  println_dbg("Starting HTTP Server...");
  server.begin();

  println_dbg("Starting SSDP...");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(hostname);
  SSDP.setSerialNumber(String(ESP.getChipId(), HEX));
  SSDP.setURL("index.htm");
  SSDP.setModelName("IR-Station");
  SSDP.setModelNumber("20160821");
  SSDP.setModelURL("https://github.com/kerikun11/IR-Station");
  SSDP.setManufacturer("KERI's Lab");
  SSDP.setManufacturerURL("https://www.kerislab.jp");
  SSDP.begin();
}

void IR_Station::reset(bool clean) {
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

  if (clean) clearAllSignals();

  save();
  ESP.reset();
}

void IR_Station::handle() {
  server.handleClient();
  ir.handle();
  ota.handle();
  /* person sensor */
  static long last_detected_ms = millis();
  static long light_state = 0;
  const long light_off_threshold_ms = 10'000;
  if (digitalRead(16) == HIGH) {
    last_detected_ms = millis();
    if (light_state == 0) {
      light_state = 1;
      sendSignal(1);
    }
  }
  if (light_state == 1 && (long)millis() > last_detected_ms + light_off_threshold_ms) {
    light_state = 0;
    sendSignal(2);
  }
  switch (mode) {
    case IR_STATION_MODE_SETUP:
      if ((WiFi.status() == WL_CONNECTED)) indicator.set(0, 1023, 1023);
#if USE_CAPITAL_PORTAL == true
      dnsServer.processNextRequest();
#endif
      break;
    case IR_STATION_MODE_STATION:
      handleSchedule();
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
#if USE_CAPITAL_PORTAL == true
      dnsServer.processNextRequest();
#endif
      break;
  }
}

void IR_Station::handleSchedule() {
  static time_t prev_time;
  if (now() != prev_time) {
    for (std::size_t i = 0; i < schedules.size(); i++) {
      wdt_reset();
      if (now() > schedules[i].time) {
        sendSignal(schedules[i].id);
        schedules.erase(schedules.begin() + i);
        save();
      }
    }
    prev_time = now();
  }
}

int IR_Station::getNewId() {
  return next_id++;
}

int IR_Station::getNewScheduleId() {
  return next_schedule_id++;
}

Signal* IR_Station::getSignalById(int id) {
  for (std::size_t i = 0; i < signals.size(); i++) {
    if (signals[i].id == id) {
      return &(signals[i]);
    }
  }
  return NULL;
}

bool IR_Station::sendSignal(int id) {
  Signal* signal = getSignalById(id);
  if (signal == NULL) return false;
  String json;
  if (!getStringFromFile(signal->path, json)) return false;
  indicator.set(0, 1023, 0);
  ir.send(json);
  indicator.set(0, 0, 1023);
  return true;
}

bool IR_Station::clearAllSignals() {
  next_id = 1;
  for (std::size_t i = 0; i < signals.size(); i++) {
    removeFile(signals[i].path);
  }
  signals.resize(0);

  next_schedule_id = 1;
  schedules.resize(0);
  return true;
}

bool IR_Station::restore() {
  wdt_reset();
  String s;
  if (getStringFromFile(STATION_JSON_PATH, s) == false) return false;
  print_dbg("data: ");
  println_dbg(s);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(s);
  if (!root.success()) return false;

  version = (const char*)root["version"];
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
  for (std::size_t i = 0; i < root["signals"].size(); i++) {
    Signal signal;
    signal.id = (int)root["signals"][i]["id"];
    signal.name = (const char*)root["signals"][i]["name"];
    signal.path = (const char*)root["signals"][i]["path"];
    signal.display = (bool)root["signals"][i]["display"];
    signal.row = (int)root["signals"][i]["row"];
    signal.column = (int)root["signals"][i]["column"];
    signals.push_back(signal);
  }

  schedules.clear();
  next_schedule_id = (int)root["next_schedule_id"];
  for (std::size_t i = 0; i < root["schedules"].size(); i++) {
    Schedule schedule;
    schedule.schedule_id = (int)root["schedules"][i]["schedule_id"];
    schedule.id = (int)root["schedules"][i]["id"];
    schedule.time = (uint32_t)root["schedules"][i]["time"];
    schedules.push_back(schedule);
  }

  if (version != IR_STATION_VERSION) {
    print_dbg("version does not match! version:");
    println_dbg(version);
    version = IR_STATION_VERSION;
    save();
  }
  println_dbg("Restored IR-Station Settings");
  return true;
}

bool IR_Station::save() {
  wdt_reset();
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
  for (std::size_t i = 0; i < signals.size(); i++) {
    JsonObject& _signal = jsonBuffer.createObject();
    _signal["id"] = signals[i].id;
    _signal["name"] = signals[i].name;
    _signal["path"] = signals[i].path;
    _signal["display"] = signals[i].display;
    _signal["row"] = signals[i].row;
    _signal["column"] = signals[i].column;
    _signals.add(_signal);
  }

  root["next_schedule_id"] = next_schedule_id;
  JsonArray& _schedules = root.createNestedArray("schedules");
  for (std::size_t i = 0; i < schedules.size(); i++) {
    JsonObject& _schedule = jsonBuffer.createObject();
    _schedule["schedule_id"] = schedules[i].schedule_id;
    _schedule["id"] = schedules[i].id;
    _schedule["time"] = (uint32_t)schedules[i].time;
    _schedules.add(_schedule);
  }

  String path = STATION_JSON_PATH;
  LittleFS.remove(path);
  File file = LittleFS.open(path, "w");
  if (!file) {
    print_dbg("File open Error: ");
    println_dbg(path);
    return false;
  }
  root.printTo(file);
  print_dbg("File Size: ");
  println_dbg(file.size(), DEC);
  file.close();
  print_dbg("Backup successful: ");
  println_dbg(path);
  print_dbg("data: ");
  root.printTo(DEBUG_SERIAL_STREAM);
  return true;
}

void IR_Station::displayRequest() {
  wdt_reset();
  println_dbg("");
  println_dbg("New Request");
  print_dbg("URI: ");
  println_dbg(server.uri());
  print_dbg("Method: ");
  println_dbg((server.method() == HTTP_GET) ? "GET" : "POST");
  print_dbg("Arguments count: ");
  println_dbg(server.args(), DEC);
  for (uint8_t i = 0; i < server.args(); i++) {
    printf_dbg("\t%s = %s\n", server.argName(i).c_str(), server.arg(i).c_str());
  }
}

void IR_Station::attachSetupApi() {
  server.on("/wifi/list", [this]() {
    displayRequest();
    DynamicJsonBuffer jsonBuffer;
    JsonArray& root = jsonBuffer.createArray();
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
      String s = WiFi.SSID(i);
      if (s.length() < 28) root.add(s);
    }
    String res;
    root.printTo(res);
    println_dbg(res);
    server.send(200, "application/json", res);
    println_dbg("End");
  });
  server.on("/wifi/confirm", [this]() {
    displayRequest();
    if (WiFi.status() == WL_CONNECTED) {
      String res = (String)WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
      server.send(200, "text/plain", res);
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
    print_dbg("Hostname: ");
    println_dbg(hostname);
    print_dbg("SSID: ");
    println_dbg(ssid);
    print_dbg("Password: ");
    println_dbg(password);
    print_dbg("Stealth: ");
    println_dbg(is_stealth_ssid ? "true" : "false");
    password = calcWPAPassPhrase(ssid, password);
    print_dbg("WPA Passphrase: ");
    println_dbg(password);
    indicator.set(0, 1023, 0);
    server.send(200);
    WiFi.disconnect();
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
  server.serveStatic("/", LittleFS, "/setup/");
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
    Signal* signal = getSignalById(id);
    if (signal == NULL) return server.send(400, "text/plain", "No signal assigned");
    if (!sendSignal(id)) return server.send(500, "text/plain", "Failed to send IR signal");
    return server.send(200, "text/plain", "Sending Successful: " + signal->name);
  });
  server.on("/signals/record", [this]() {
    displayRequest();
    String name;
    {
      const uint32_t timeout_ms = 5000;
      indicator.set(0, 1023, 0);
      ir.resume();
      int timeStamp = millis();
      while (!ir.available()) {
        wdt_reset();
        ir.handle();
        if (millis() - timeStamp > timeout_ms) {
          indicator.set(1023, 0, 0);
          return server.send(500, "text/plain", "No Signal Received");
        }
      }
      indicator.set(0, 0, 1023);
      String data = ir.read();
      name = server.arg("name");
      Signal signal;
      signal.id = getNewId();
      signal.name = name;
      signal.path = IR_DATA_PATH(signal.id);
      signal.display = (server.arg("display") == "true");
      signal.row = server.arg("row").toInt();
      signal.column = server.arg("column").toInt();

      if (!writeStringToFile(signal.path, data)) return server.send(500, "text/plain", "Failed to write File");
      signals.push_back(signal);
    }
    save();
    return server.send(200, "text/plain", "Recording Successful: " + name);
  });
  server.on("/signals/rename", [this]() {
    displayRequest();
    int id = server.arg("id").toInt();
    Signal* signal = getSignalById(id);
    if (signal == NULL) return server.send(400, "text/plain", "No signal assigned");
    String prev_name = signal->name;
    signal->name = server.arg("name");
    save();
    return server.send(200, "text/plain", "Renamed " + prev_name + " to " + signal->name);
  });
  server.on("/signals/move", [this]() {
    displayRequest();
    int id = server.arg("id").toInt();
    Signal* signal = getSignalById(id);
    if (signal == NULL) return server.send(400, "text/plain", "No signal assigned");
    signal->row = server.arg("row").toInt();
    signal->column = server.arg("column").toInt();
    save();
    return server.send(200, "text/plain", "Moved position: " + signal->name);
  });
  server.on("/signals/upload", [this]() {
    displayRequest();
    Signal signal;
    signal.id = getNewId();
    signal.name = server.arg("name");
    signal.path = IR_DATA_PATH(signal.id);
    signal.display = (server.arg("display") == "true");
    signal.row = server.arg("row").toInt();
    signal.column = server.arg("column").toInt();

    String irJson = server.arg("irJson");
    DynamicJsonBuffer jsonBuffer;
    JsonArray& data = jsonBuffer.parseArray(irJson);
    if (!data.success()) return server.send(400, "text/plain", "Invalid Signal Format");
    if (!writeStringToFile(signal.path, irJson)) return server.send(500, "text/plain", "Failed to write File");
    signals.push_back(signal);
    save();
    return server.send(200, "text/plain", "Uploading Successful: " + signal.name);
  });
  server.on("/signals/clear", [this]() {
    displayRequest();
    int id = server.arg("id").toInt();
    for (std::size_t i = 0; i < signals.size(); i++) {
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
    clearAllSignals();
    save();
    return server.send(200, "text/plain", "Cleared All Signals");
  });
  server.on("/schedule/new", [this]() {
    if (mode == IR_STATION_MODE_AP) return server.send(400, "text/plain", "Schedule is unavailable in AP mode :(");
    int id = server.arg("id").toInt();
    Schedule schedule;
    schedule.schedule_id = getNewScheduleId();
    schedule.id = id;
    schedule.time = server.arg("time").toInt();
    schedules.push_back(schedule);
    save();
    return server.send(200, "text/plain", "Added a Schedule");
  });
  server.on("/schedule/delete", [this]() {
    int schedule_id = server.arg("schedule_id").toInt();
    for (std::size_t i = 0; i < signals.size(); i++) {
      if (schedules[i].schedule_id == schedule_id) {
        schedules.erase(schedules.begin() + i);
        save();
        return server.send(200, "text/plain", "Deleted a Schedule");
      }
    }
    return server.send(400, "text/plain", "No such a schedule");
  });
  server.on("/wifi/disconnect", [this]() {
    displayRequest();
    server.send(200);
    delay(100);
    reset(false);
  });
  server.on("/wifi/change-ip", [this]() {
    displayRequest();
    IPAddress _local_ip, _subnetmask, _gateway;
    if (!_local_ip.fromString(server.arg("local_ip")) || !_subnetmask.fromString(server.arg("subnetmask")) || !_gateway.fromString(server.arg("gateway"))) {
      println_dbg("400");
      return server.send(400, "text/plain", "Bad Request!");
    }
    WiFi.config(_local_ip, _gateway, _subnetmask);
    if (WiFi.localIP() != _local_ip) {
      println_dbg("500");
      return server.send(500, "text/plain", "Couldn't change IP Address :(");
    }
    is_static_ip = true;
    local_ip = _local_ip;
    subnetmask = _subnetmask;
    gateway = _gateway;
    save();
    return server.send(200, "text/plain", "Changed IP Address to " + WiFi.localIP().toString());
  });
  server.onNotFound([this]() {
    displayRequest();
    String res = "<script>location.href = \"http://" + WiFi.localIP().toString() + "/\";</script>";
    server.send(200, "text/html", res);
    println_dbg("End");
  });
  server.serveStatic("/", LittleFS, "/main/", "public");
}
