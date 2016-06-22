#ifndef SERVER_OPERATION
#define SERVER_OPERATION

#include <ESP8266WiFi.h>

extern String mdns_address;

void dispRequest();
void serverTask();

void setupFormServer(void);
void setupServer(void);

#endif

