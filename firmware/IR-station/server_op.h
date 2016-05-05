#ifndef SERVER_OPERATION
#define SERVER_OPERATION

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// TCP server at port 80 will respond to HTTP requests
extern ESP8266WebServer server;
extern String mdns_address;

void setupAPServer(void);
String generateAPHtml(String status);
String handleAPRequest(void);
void handleAPRoot(void);

void setupServer(void);
String generateHtml(String status = "");
void handleRoot(void);
void handleSend(void);
void handleRecode(void);
void handleSettings(void);

// replace a number to the symbol
void charEncode(String &s);

#endif

