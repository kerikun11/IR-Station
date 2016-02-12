#include "WiFi_op.h"

const char softap_ssid[] = SOFTAP_SSID;
const char softap_pass[] = SOFTAP_PASS;
String target_ssid = "NULL";
String target_pass = "NULL";

void wifiSetup(void) {
  ESP.wdtFeed();
  if (connectWifi() == 0) return;

  // if couldn't connect the cached WiFi
  setupAP();
  setupAPServer();
  while (1) {
    ESP.wdtFeed();
    server.handleClient();
  }
}

void setupAP(void) {
  ESP.wdtFeed();
  WiFi.mode(WIFI_AP);
  println_dbg("Configuring Access Point...");
  WiFi.softAP(softap_ssid, softap_pass);

  // display information
  print_dbg("AP SSID : ");
  println_dbg(softap_ssid);
  print_dbg("AP IP address: ");
  println_dbg(WiFi.softAPIP());
}

void closeAP(void) {
  ESP.wdtFeed();
  println_dbg("softAP closed and configuring Station");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  delay(100);
}

int connectWifi() {
  // set WiFi Mode
  WiFi.mode(WIFI_STA);
  // Connect to WiFi network
  println_dbg("");
  print_dbg("Connecting to SSID: ");
  println_dbg(target_ssid);
  print_dbg("Password: ");
  println_dbg(target_pass);
  if (target_ssid == "NULL")return (-1);
  WiFi.begin(target_ssid.c_str(), target_pass.c_str());
  println_dbg("");

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
      return (-1);
    }
  }
  println_dbg("");
  print_dbg("Connected to ");
  println_dbg(target_ssid);
  print_dbg("IP address: ");
  println_dbg(WiFi.localIP());

  return 0;
}

void wifiRestoreFromFile(void) {
  File f = SPIFFS.open(WIFI_DATA_PATH, "r");
  if (!f) {
    println_dbg("file open error");
  } else {
    String s = f.readStringUntil('\n');
    target_ssid = extract(s, "?ssid=");
    target_pass = extract(s, "&pass=");
    String mdns = extract(s, "&mdns=");
    if (mdns != "") {
      mdns_address = mdns;
    }
    f.close();
    println_dbg("got WiFi SSID and Password");
  }
}

void wifiBackupToFile(void) {
  SPIFFS.remove(WIFI_DATA_PATH);
  File f = SPIFFS.open(WIFI_DATA_PATH, "w");
  if (!f) {
    println_dbg("file open error");
    return;
  }
  f.print("?ssid=" + target_ssid);
  f.print("&pass=" + target_pass);
  f.print("&mdns=" + mdns_address);
  f.println("&End");
  f.close();
  println_dbg("WiFi data backup successful");
}

String extract(String target, String head, String tail) {
  return target.substring(target.indexOf(head) + head.length(), target.indexOf(tail, target.indexOf(head) + head.length()));
}

