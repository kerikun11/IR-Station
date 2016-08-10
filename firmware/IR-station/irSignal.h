#ifndef IR_SIGNAL_H
#define IR_SIGNAL_H

#include <ESP8266WiFi.h>

#define RAW_DATA_MAX_SIZE 256

class IR_Signal {
  public:
    String chName;
    unsigned int rawData[RAW_DATA_MAX_SIZE];
    uint16_t rawDataLength;

    void set(String name, uint16_t* data = NULL, uint16_t length = NULL);
    String getBackupString(void);
    void restoreFromString(String dataString);
    void dispData(void);
};

#endif

