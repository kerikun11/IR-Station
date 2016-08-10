#ifndef IR_OPERATION
#define IR_OPERATION

#include <ESP8266WiFi.h>
#include "irSignal.h"
#include "config.h"

#define IR_STATION_MODE_NULL  0
#define IR_STATION_MODE_STA   1
#define IR_STATION_MODE_AP    2

class IR_Station {
  public:
    uint8_t mode;
    String ssid;
    String password;
    String hostname;

    String chName[IR_CH_SIZE];

    void begin(void);
    void setMode(uint8_t newMode);
    void reset();

    void setupButtonInterrupt();

    void clearSignals();
    void irSendSignal(int ch);
    bool irRecodeSignal(int ch, String name, uint32_t timeout_ms = 3000);

    String settingsCrcSerial(void);
    bool settingsRestoreFromFile(void);
    bool settingsBackupToFile(void);
    void restoreChName(void);

  private:

};

extern IR_Station station;
extern IR_Signal ir;

bool writeStringToFile(String path, String dataString);
bool getStringFromFile(String path, String& dataString);

#endif

