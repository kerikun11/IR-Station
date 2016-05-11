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
#include "config.h"

#ifndef OTA_HOSTNAME
#Indicate "Please define OTA_HOSTNAME"
#endif

#ifndef OTA_PASSWORD
#Indicate "Please define OTA_PASSWORD"
#endif

void setupOTA();
void OTATask();

#endif

