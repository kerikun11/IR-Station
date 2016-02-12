#ifndef IR_OPERATION
#define IR_OPERATION

#include <ESP8266WiFi.h>
#include <FS.h>
#include "config.h"
#include "IR-lib.h"

// define variables
extern remocon ir[IR_CH_SIZE];

void irSendSignal(int ch);
int irRecodeSignal(int ch);
void irDataBackupToFile(int ch);
void irDataRestoreFromFile(void);

#endif

