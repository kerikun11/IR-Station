#ifndef IR_OPERATION
#define IR_OPERATION

#include <ESP8266WiFi.h>
#include "IR-lib.h"

#define IR_STATION_MODE_NULL  0
#define IR_STATION_MODE_STA   1
#define IR_STATION_MODE_AP    2

extern remocon ir[IR_CH_SIZE];
extern uint8_t mode;
extern String ssid;
extern String password;
extern String mdns_address;

void modeSetup(void);
void setMode(uint8_t newMode);

void setupButtonInterrupt();

void irSendSignal(int ch);
int irRecodeSignal(int ch);

void irDataBackupToFile(int ch);
void irDataRestoreFromFile(void);

void settingsRestoreFromFile(void);
void settingsBackupToFile(void);

bool writeStringToFile(String path, String dataString);
bool getStringFromFile(String path, String& dataString);

#endif

