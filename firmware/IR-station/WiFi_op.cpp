#include "WiFi_op.h"

const char softap_ssid[] = SOFTAP_SSID;
const char softap_pass[] = SOFTAP_PASS;
String target_ssid = "NULL";
String target_pass = "NULL";

void wifiSetup(void) {
  ESP.wdtFeed();
  if (connectWifi() != 0) {
    setAccesspoint();
    while (1) {
      ESP.wdtFeed();
      if (getTargetWifi() != 0)continue;
      if (connectWifi() != 0)continue;
      break;
    }
    closeAccesspoint();
    ESP.wdtFeed();
    wifiBackupToFile();
  }
}

void setAccesspoint(void) {
  ESP.wdtFeed();
  WiFi.mode(WIFI_AP);
  Serial.println("Configuring access point...");
  WiFi.softAP(softap_ssid, softap_pass);

  // display information
  Serial.print("AP SSID : ");
  Serial.println(softap_ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Set up mDNS responder:
  MDNS.addService("http", "tcp", 80);
  Serial.print("mDNS address: ");
  Serial.println(DEFAULT_MDNS_ADDRESS);
  if (!MDNS.begin(DEFAULT_MDNS_ADDRESS, WiFi.softAPIP())) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }

  server.begin();
  Serial.println("HTTP AP server started");
  Serial.println("Listening");
}

void closeAccesspoint(void) {
  ESP.wdtFeed();
  Serial.println("softAP closed and configuring Station");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  delay(100);
}

int connectWifi() {
  // Connect to WiFi network
  Serial.println("");
  Serial.print("Connecting to SSID: ");
  Serial.println(target_ssid);
  Serial.print("Password: ");
  Serial.println(target_pass);
  WiFi.begin(target_ssid.c_str(), target_pass.c_str());
  Serial.println("");

  // Wait for connection
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    ESP.wdtFeed();
    delay(500);
    Serial.print(".");
    timeout++;
    if (timeout >= 2 * WIFI_CONNECT_TIMEOUT) {
      Serial.println("");
      Serial.println("Invalid SSID or Password");
      Serial.println("WiFi Connection Failed");
      return (-1);
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(target_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set up mDNS responder:
  MDNS.addService("http", "tcp", 80);
  Serial.print("mDNS address: ");
  Serial.println(mdns_address);
  if (!MDNS.begin(mdns_address.c_str(), WiFi.localIP())) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }

  // Start TCP (HTTP) server
  server.begin();
  Serial.println("TCP server started");
  Serial.println("Listening");
  return 0;
}

void wifiRestoreFromFile(void) {
  File f = SPIFFS.open(WIFI_DATA_PATH, "r");
  if (!f) {
    Serial.println("file open error");
  } else {
    String s = f.readStringUntil('\n');
    target_ssid = extract(s, "?ssid=");
    target_pass = extract(s, "&pass=");
    String mdns = extract(s, "&mdns=");
    if (mdns != "") {
      mdns_address = mdns;
    }
    f.close();
    Serial.println("got WiFi SSID and Password");
  }
}

void wifiBackupToFile(void) {
  SPIFFS.remove(WIFI_DATA_PATH);
  File f = SPIFFS.open(WIFI_DATA_PATH, "w");
  if (!f) {
    Serial.println("file open error");
    return;
  }
  f.print("?ssid=" + target_ssid);
  f.print("&pass=" + target_pass);
  f.print("&mdns=" + mdns_address);
  f.println("&End");
  f.close();
  Serial.println("WiFi data backup successful");
}


