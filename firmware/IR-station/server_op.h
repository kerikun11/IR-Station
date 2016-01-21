#ifndef SERVER_OPERATION
#define SERVER_OPERATION

#include <ESP8266WiFi.h>
#include "config.h"
#include "IR_op.h"
#include "WiFi_op.h"
#include "String_op.h"
#include "server_op.h"

// TCP server at port 80 will respond to HTTP requests
extern WiFiServer server;
extern String mdns_address;

int getTargetWifi(void);

void getClient(void) ;


#endif

