/*
   OTA operation

   void setup(){
     ...
     setupOTA();
     ...
   }
   void loop(){
     ...
     OTATask();
     ...
   }
*/
#ifndef OTA_OP_H
#define OTA_OP_H

#include <ESP8266WiFi.h>

class OTA {
  public:
    void begin(String hostname, String password = "", int port = 8266);
    void handle();
};

#endif

