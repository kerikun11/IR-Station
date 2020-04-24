#ifndef MySSDP_H
#define MySSDP_H

#define NO_GLOBAL_SSDP
#include <ESP8266SSDP.h>

class MySSDPClass : public SSDPClass {
  public:
		uint16_t strlen(IPAddress ip);
    char* schema(IPAddress ip, char* buffer, uint16_t size) const;
};

extern MySSDPClass SSDP;

#endif
