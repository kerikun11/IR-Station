#ifndef SERVER_OPERATION
#define SERVER_OPERATION

#include <ESP8266WiFi.h>
#include "config.h"
#include "IR_op.h"
#include "WiFi_op.h"
#include "server_op.h"

// TCP server at port 80 will respond to HTTP requests
extern ESP8266WebServer server;
extern String mdns_address;

void setupAPServer(void);
String generateAPHtml(String status);
String handleAPRequest(void);
void handleAPRoot(void);

void setupServer(void);
String generateHtml(String status);
String handleRequest(void);
void handleRoot(void);

// replace a number to the symbol
void charEncode(String &s);

#endif

