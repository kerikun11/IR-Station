#include "server_op.h"

const String html_head =
  "<!DOCTYPE HTML><html><head>\r\n"
  "<meta charset=\"utf-8\">\r\n"
  "<style type=\"text/css\">"
  "body{background-color:#BBDDFF;margin:5%;text-align:center;}"
  "#send button{font-size:1.5em;height:3em;font-weight:bold;width:19%;}"
  "</style>"
  "<link rel=\"shortcut icon\" href=\"http://kerikeri.top/esp8266.png\"/>\r\n"
  "<title>IR Station</title></head>\r\n"
  "<body><h1>IR Station</h1>\r\n";
const String html_tail =
  "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.0/jquery.min.js\"></script>\r\n"
  "<script>\r\n"
  "$('#send button').click(function(){var el = $(this);$.post('/send',{id: el.data('id')}).done(function(){$('#log').prepend('<p>'+Date().match(/.+(\\d\\d:\\d\\d:\\d\\d).+/)[1]+' => Sent '+el.text()+' Signal</p>');})});\r\n"
  "$('#recode button').click(function(){$.post('/recode',{ch: $('[name=ch]').val(), chName: $('[name=chName]').val()}).done(function(){location.reload();});});\r\n"
  "$('#reload').click(function(){location.reload();});"
  "$('#clearAllSignals').click(function(){if(confirm('Are you sure to delete all signals?')){$.post('/settings',{settings: $(this).attr('id')}).done(function(){location.reload();});}});\r\n"
  "$('#disconnectWifi').click(function(){if(confirm('Are you sure to disconnect this WiFi?')){$.post('/settings',{settings: $(this).attr('id')});}});\r\n"
  "</script>\r\n"
  "</body></html>";

// TCP server at port 80 will respond to HTTP requests
ESP8266WebServer server(80);
String mdns_address = DEFAULT_MDNS_ADDRESS;

void setupAPServer(void) {
  // Set up mDNS responder:
  print_dbg("mDNS address: ");
  println_dbg("http://" + mdns_address + ".local");
  if (!MDNS.begin(DEFAULT_MDNS_ADDRESS, WiFi.softAPIP())) {
    println_dbg("Error setting up MDNS responder!");
  } else {
    println_dbg("mDNS responder started");
  }

  server.on("/", handleAPRoot);

  // Start TCP (HTTP) server
  server.begin();
  println_dbg("TCP server started");
  println_dbg("Listening");
}

String generateAPHtml(String status) {
  String html_ssidpassform = "";
  html_ssidpassform +=  "<form method=\"get\"><p>SSID:<select name=\"ssid\"/>";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    html_ssidpassform += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
  }
  html_ssidpassform +=
    "</select></p>"
    "<p>Password:<input type=\"password\" name=\"pass\"/></p>"
    "<p>URL:<input type=\"text\" name=\"url\"/></p>"
    "<p><button type=\"submit\" name=\"End\">OK</button></p></form>";

  String html_status = "";
  html_status += status;

  return html_head + html_status + html_ssidpassform + html_tail;
}

String handleAPRequest(void) {
  String status = "<p>";
  // Match the request
  if (server.argName(0) == "ssid") {
    target_ssid = server.arg(0);
    target_pass = server.arg(1);
    mdns_address = server.arg(2);
    if (mdns_address == "") {
      mdns_address = DEFAULT_MDNS_ADDRESS;
    }
    println_dbg("Target SSID: " + target_ssid);
    println_dbg("Target Password: " + target_pass);
    println_dbg("mDNS Address: " + mdns_address);

    status += "Connecteding to " + target_ssid;
  } else {
    status += "Please fill up the blanks bellow.";
  }
  status += "</p>";
  return status;
}

void handleAPRoot(void) {
  // Request detail
  println_dbg("");
  println_dbg("New Request");
  println_dbg("URI: " + server.uri());
  println_dbg("Method: " + (server.method() == HTTP_GET) ? "GET" : "POST");
  println_dbg("Arguments: " + String(server.args()));
  for (uint8_t i = 0; i < server.args(); i++) {
    println_dbg("  " + server.argName(i) + " = " + server.arg(i));
  }

  // Prepare the response
  String status = handleAPRequest();

  // Prepare the response
  String response = generateAPHtml(status);

  // Send the response
  server.send(200, "text/html", response);
  delay(100);

  // if WiFi is configured, reboot
  if (server.argName(0) == "ssid") {
    wifiBackupToFile();
    closeAP();
    RESET();
  }
}

void setupServer(void) {
  // Set up mDNS responder:
  print_dbg("mDNS address: ");
  println_dbg("http://" + mdns_address + ".local");
  if (!MDNS.begin(mdns_address.c_str(), WiFi.localIP())) {
    println_dbg("Error setting up MDNS responder!");
  } else {
    println_dbg("mDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.on("/recode", handleRecode);
  server.on("/settings", handleSettings);

  // Start TCP (HTTP) server
  server.begin();
  println_dbg("TCP server started");
  println_dbg("Listening");
}

String generateHtml(String status) {
  String html_send_buttons = "";
  html_send_buttons += "<div id=\"send\">";
  for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
    if (i % 5 == 0) html_send_buttons += "<p>";
    html_send_buttons += "<button data-id=\"" + String(i + 1, DEC) + "\">";
    if (ir[i].chName == "") html_send_buttons += "ch " + String(i + 1, DEC) ;
    else html_send_buttons += String(ir[i].chName);
    html_send_buttons += "</button>";
    if (i % 5 == 5 - 1) html_send_buttons += "</p>\r\n";
  }
  html_send_buttons += "</div>\r\n";

  String html_recode_buttons = "";
  html_recode_buttons += "<div id=\"recode\">";
  html_recode_buttons += "<p><select name=\"ch\">\r\n";
  html_recode_buttons += "<option value=\"" + String(-1, DEC) + "\">-- channel select --</option>";
  for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
    html_recode_buttons += "<option value=\"" + String(i + 1, DEC) + "\">";
    if (ir[i].chName == "") html_recode_buttons += "Ch." + String(i + 1, DEC);
    else html_recode_buttons += "Ch." + String(i + 1, DEC) + " : " + ir[i].chName;
    html_recode_buttons += "</option>\r\n";
  }
  html_recode_buttons += "</select><input type=\"text\" name=\"chName\"/><button>Recode</button>";
  html_recode_buttons += "</p></div>\r\n";

  String html_status = "";
  html_status += status;
  html_status += "<p>Connecting SSID : " + target_ssid + "</p>\r\n";
  html_status += "<p>IP address : ";
  html_status += String((WiFi.localIP() >> 0) & 0xFF, DEC) + ".";
  html_status += String((WiFi.localIP() >> 8) & 0xFF, DEC) + ".";
  html_status += String((WiFi.localIP() >> 16) & 0xFF, DEC) + ".";
  html_status += String((WiFi.localIP() >> 24) & 0xFF, DEC);
  html_status += "</p>\r\n";
  html_status += "<p>URL : http://" + mdns_address + ".local</p>\r\n";

  String html_log = "";
  html_log += "<div id=\"log\"></div>";

  String html_menu_buttons =
    "<div id=\"settings\">"
    "<button id=\"reload\">Reload</button>"
    "<button id=\"clearAllSignals\">Clear All Signals</button>"
    "<button id=\"disconnectWifi\">Disconnect WiFi</button>"
    "</div>\r\n";

  return html_head + html_send_buttons + html_recode_buttons + html_status + html_menu_buttons + html_log + html_tail;
}

void handleRoot(void) {
  // Request detail
  println_dbg("");
  println_dbg("New Request");
  println_dbg("URI: " + server.uri());
  println_dbg("Method: " + (server.method() == HTTP_GET) ? "GET" : "POST");
  println_dbg("Arguments: " + String(server.args()));
  for (uint8_t i = 0; i < server.args(); i++) {
    println_dbg("  " + server.argName(i) + " = " + server.arg(i));
  }

  // Prepare the response
  String response = generateHtml("<p>Status: Listening</p>");

  // Send the response
  server.send(200, "text/html", response);
}

void handleSend(void) {
  // Request detail
  println_dbg("");
  println_dbg("New Request");
  println_dbg("URI: " + server.uri());
  println_dbg("Method: " + (server.method() == HTTP_GET) ? "GET" : "POST");
  println_dbg("Arguments: " + String(server.args()));
  for (uint8_t i = 0; i < server.args(); i++) {
    println_dbg("  " + server.argName(i) + " = " + server.arg(i));
  }

  uint8_t ch = server.arg(0).toInt();
  ch -= 1; // display: 1 ch ~ IR_CH_SIZE ch but data: 0 ch ~ (IR_CH_NAME - 1) ch so 1 decriment
  if (0 <= ch  && ch < IR_CH_SIZE) {
    irSendSignal(ch);
  } else {
    println_dbg("Invalid channel selected. Sending failed.");
  }

  // Send the response
  server.send(200);
}

void handleRecode(void) {
  // Request detail
  println_dbg("");
  println_dbg("New Request");
  println_dbg("URI: " + server.uri());
  println_dbg("Method: " + (server.method() == HTTP_GET) ? "GET" : "POST");
  println_dbg("Arguments: " + String(server.args()));
  for (uint8_t i = 0; i < server.args(); i++) {
    println_dbg("  " + server.argName(i) + " = " + server.arg(i));
  }

  String status = "<p>Status: ";
  // Match the request
  uint8_t ch = server.arg(0).toInt();
  ch -= 1; // display: 1 ch ~ IR_CH_SIZE ch but data: 0 ch ~ (IR_CH_NAME - 1) ch so 1 decriment
  if (0 <= ch  && ch < IR_CH_SIZE) {
    ir[ch].chName = server.arg(1);
    charEncode(ir[ch].chName);
    if (irRecodeSignal(ch) == 0) {
      status += "Recoding Successful: ch " + String(ch + 1);
    } else {
      status += "Nosignal Recieved";
    }
  } else {
    println_dbg("No ch selected");
    status += "Please select a channel.";
  }

  // Send the response
  server.send(200);
}

void handleSettings(void) {
  // Request detail
  println_dbg("");
  println_dbg("New Request");
  println_dbg("URI: " + server.uri());
  println_dbg("Method: " + (server.method() == HTTP_GET) ? "GET" : "POST");
  println_dbg("Arguments: " + String(server.args()));
  for (uint8_t i = 0; i < server.args(); i++) {
    println_dbg("  " + server.argName(i) + " = " + server.arg(i));
  }

  if (server.arg(0) == "clearAllSignals") {
    for (uint8_t i = 0; i < IR_CH_SIZE; i++) {
      ir[i].period = 0;
      ir[i].chName = "";
      ir[i].irData = "";
      irDataBackupToFile(i);
    }
    println_dbg("Cleared All Ch Signals");
  } else if (server.arg(0) == "disconnectWifi") {
    println_dbg("Change WiFi SSID");
    target_ssid = "NULL";
    target_pass = "NULL";
    wifiBackupToFile();
    RESET();
  }

  // Send the response
  server.send(200);
}

void charEncode(String & s) {
  s.replace("+", " ");
  s.replace("%20", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%2A", "*");
  s.replace("%2B", "+");
  s.replace("%2C", ",");
  s.replace("%2D", "-");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%3A", ":");
  s.replace("%3B", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  s.replace("%7B", "{");
  s.replace("%7C", "|");
  s.replace("%7D", "}");
  s.replace("%7E", "~");
}

