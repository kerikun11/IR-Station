#include "server_op.h"

const String html_head =
  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
  "<!DOCTYPE HTML><html><head>"
  "<style type=\"text/css\">"
  "body{background-color:#BBDDFF;margin:5%;text-align:center;}"
  "button.send{font-size:2em;font-weight:bold;width:30%;}"
  "</style>"
  "<link rel=\"shortcut icon\" href=\"http://kerikeri.top/esp8266.png\"/>"
  "<title>ESP8266-Remocon</title></head>\r\n"
  "<body><h1>Welcome to ESP8266!</h1>";
const String html_tail = "</body></html>\r\n";
const String html_menu_buttons =
  "<form method=\"get\"><p>"
  "<button type=\"submit\" name=\"clear\">Clear All Signals</button>"
  "<button type=\"submit\" name=\"chwifi\">Change WiFi-SSID</button>"
  "</p></form>";

// TCP server at port 80 will respond to HTTP requests
WiFiServer server(80);
String mdns_address;

int getTargetWifi() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    ESP.wdtFeed();
    return (-1);
  }
  println_dbg("");
  println_dbg("New client");

  // Wait for data from client to become available
  while (client.connected() && !client.available()) {
    ESP.wdtFeed();
    delay(10);
  }

  // Read the first line of HTTP request
  String req = client.readStringUntil('\n');
  print_dbg("Request: ");
  println_dbg(req);

  // GET / HTTP/1.1
  // GET /?ssid=ABCDEFG&pass=PASSWORD HTTP/1.1
  String ssidpassform;
  ssidpassform =  "<form method=\"get\"><p>SSID:<select name=\"ssid\"/>";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    ssidpassform += "<option value=\"";
    ssidpassform += WiFi.SSID(i);
    ssidpassform += "\">";
    ssidpassform += WiFi.SSID(i);
    ssidpassform += "</option>";
  }
  ssidpassform += "</select></p>"
                  "<p>Password:<input type=\"password\" name=\"pass\"/></p>"
                  "<p>URL:<input type=\"text\" name=\"url\"/></p>"
                  "<p><button type=\"submit\" name=\"End\">OK</button></p></form>";

  String s;
  int error = 0;
  if (req.startsWith("GET /?ssid=")) {
    target_ssid = extract(req, "?ssid=");
    target_pass = extract(req, "&pass=");
    String url = extract(req, "&url=");
    if (url.length() > 0) {
      mdns_address = url;
    } else {
      mdns_address = DEFAULT_MDNS_ADDRESS;
    }
    print_dbg("Target SSID: ");
    println_dbg(target_ssid);
    print_dbg("Target Password: ");
    println_dbg(target_pass);
    print_dbg("mDNS Address: ");
    println_dbg(mdns_address);

    s = html_head;
    s += "<p>Connecting to SSID:" + target_ssid + "</p>";
    s += ssidpassform;
    s += html_tail;
    client.print(s);
    delay(1);
    println_dbg("Client disonnected");
    client.stop();
    return 0;
  } else {
    s = html_head;
    s += ssidpassform;
    s += html_tail;
    client.print(s);
    delay(1);
    client.stop();
    println_dbg("Client disonnected");
    return (-1);
  }
}

void getClient(void) {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    ESP.wdtFeed();
    return;
  }
  println_dbg("");
  println_dbg("New client");

  // Wait for data from client to become available
  while (client.connected() && !client.available()) {
    ESP.wdtFeed();
    delay(10);
  }

  // Read the first line of HTTP request
  String req = client.readStringUntil('\r');
  print_dbg("Request: ");
  println_dbg(req);
  if (req == "") {
    client.stop();
    return;
  }

  // Match the request
  if (req.startsWith("GET /?send", 0)) {
    uint8_t ch = extract(req, "?send", "ch=").toInt();
    ch -= 1;
    if (0 <= ch  && ch < IR_CH_SIZE) {
      digitalWrite(LED1, HIGH);
      ir[ch].sendSignal();
      digitalWrite(LED1, LOW);
    }
  } else if (req.startsWith("GET /?recode=", 0)) {
    uint8_t ch = extract(req, "?recode=").toInt();
    ch -= 1;
    if (0 <= ch  && ch < IR_CH_SIZE) {
      ir[ch].chName = extract(req, "&chName=", " HTTP/");
      charEncode(ir[ch].chName);
      digitalWrite(LED1, HIGH);
      if (ir[ch].recodeSignal() == 0) {
        ir[ch].dataBackupToFile(IR_DATA_PATH(ch));
      }
      digitalWrite(LED1, LOW);
    } else {
      println_dbg("No ch selected");
    }
  } else if (req.startsWith("GET /?clear=", 0)) {
    for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
      ir[i].chName = "";
      ir[i].irData = "";
      ir[i].dataBackupToFile(IR_DATA_PATH(i));
    }
    println_dbg("Cleared All Ch Signals");
  } else if (req.startsWith("GET /?chwifi=", 0)) {
    println_dbg("Change WiFi SSID");
    target_ssid = "NULL";
    target_pass = "NULL";
    wifiBackupToFile();
    wifiSetup();
  }

  ESP.wdtFeed();
  // Prepare the response
  String html_send_buttons;
  html_send_buttons = "<form method=\"get\">";
  for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
    if (i % 3 == 0) html_send_buttons += "<p>";
    html_send_buttons += "<button class=\"send\" type = \"submit\" name=\"send" + String(i+1, DEC) + "ch\" >";
    if (ir[i].chName == "") html_send_buttons += "ch " + String(i + 1, DEC) ;
    else html_send_buttons += String(ir[i].chName);
    html_send_buttons += "</button>";
    if (i % 3 == 3 - 1) html_send_buttons += "</p>";
  }
  html_send_buttons += "</form>";

  String html_recode_buttons;
  html_recode_buttons += "<form method=\"get\"><p><select name=\"recode\">";
  html_recode_buttons += "<option value=\"" + String(-1, DEC) + "\">-- channel select --</option>";
  for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
    html_recode_buttons += "<option value=\"" + String(i + 1, DEC) + "\">";
    if (ir[i].chName == "") html_recode_buttons += "Ch." + String(i + 1, DEC);
    else html_recode_buttons += ir[i].chName;
    html_recode_buttons += "</option>";
  }
  html_recode_buttons += "</select>:<input type=\"text\" name=\"chName\"/><button type=\"submit\">Recode</button>";
  html_recode_buttons += "</p></form>";

  String html_status;
  html_status = "<p>Connecting SSID : " + target_ssid + "</p>";
  html_status += "<p>IP address : ";
  html_status += String((WiFi.localIP() >> 0) & 0xFF, DEC) + ".";
  html_status += String((WiFi.localIP() >> 8) & 0xFF, DEC) + ".";
  html_status += String((WiFi.localIP() >> 16) & 0xFF, DEC) + ".";
  html_status += String((WiFi.localIP() >> 24) & 0xFF, DEC);
  html_status += "</p>";
  html_status += "<p>URL : http://" + mdns_address + ".local</p>";

  // Send the response to the client
  client.println(html_head);
  client.println(html_send_buttons);
  client.println(html_recode_buttons);
  client.println(html_status);
  client.println(html_menu_buttons);
  client.println(html_tail);
  delay(1);
  client.stop();
  println_dbg("Client disonnected");
}

