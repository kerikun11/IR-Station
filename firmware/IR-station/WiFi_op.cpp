#include "WiFi_op.h"

const char softap_ssid[] = SOFTAP_SSID;
const char softap_pass[] = SOFTAP_PASS;
String target_ssid = "NULL";
String target_pass = "NULL";

void wifiSetup(void) {
  ESP.wdtFeed();
  setAccesspoint();
  while (1) {
    ESP.wdtFeed();
    if (getTargetWifi() != 0)continue;
    if (configureWifi() != 0)continue;
    break;
  }
  closeAccesspoint();
  ESP.wdtFeed();
  wifiBackupToFile();
}

void setAccesspoint(void) {
  ESP.wdtFeed();
  WiFi.mode(WIFI_AP);
  println_dbg("Configuring access point...");
  WiFi.softAP(softap_ssid, softap_pass);

  // display information
  print_dbg("AP SSID : ");
  println_dbg(softap_ssid);
  print_dbg("AP IP address: ");
  println_dbg(WiFi.softAPIP());

  // Set up mDNS responder:
  MDNS.addService("http", "tcp", 80);
  print_dbg("mDNS address: ");
  println_dbg(DEFAULT_MDNS_ADDRESS);
  if (!MDNS.begin(DEFAULT_MDNS_ADDRESS, WiFi.softAPIP())) {
    println_dbg("Error setting up MDNS responder!");
  } else {
    println_dbg("mDNS responder started");
  }

  server.begin();
  println_dbg("HTTP AP server started");
  println_dbg("Listening");
}

void closeAccesspoint(void) {
  ESP.wdtFeed();
  println_dbg("softAP closed and configuring Station");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  delay(100);
}

int configureWifi() {
  // Connect to WiFi network
  println_dbg("");
  print_dbg("Connecting to SSID: ");
  println_dbg(target_ssid);
  print_dbg("Password: ");
  println_dbg(target_pass);
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

  // Set up mDNS responder:
  MDNS.addService("http", "tcp", 80);
  print_dbg("mDNS address: ");
  println_dbg(mdns_address);
  if (!MDNS.begin(mdns_address.c_str(), WiFi.localIP())) {
    println_dbg("Error setting up MDNS responder!");
  } else {
    println_dbg("mDNS responder started");
  }

  // Start TCP (HTTP) server
  server.begin();
  println_dbg("TCP server started");
  println_dbg("Listening");
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


