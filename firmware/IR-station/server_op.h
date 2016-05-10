#ifndef SERVER_OPERATION
#define SERVER_OPERATION

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

extern String mdns_address;

void dispRequest();
void serverTask();

void setupFormServer(void);
void setupServer(void);

// replace a number to the symbol
void charEncode(String &s);

#endif

