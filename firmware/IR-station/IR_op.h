#include <arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include "config.h"
#include "String_op.h"

#ifndef IR_OPERATION_H
#define IR_OPERATION_H

// IR recode timeouts
#define TIMEOUT_RECODE_NOSIGNAL (40000)
#define TIMEOUT_RECODE          (3000000)

// IR recode state
#define STATE_NONE              (-1)
#define STATE_OK                (0)
#define STATE_TIMEOUT           (1)
#define STATE_OVERFLOW          (2)
#define STATE_FORMAT_ERROR      (3)

// for rawDataSize
#define RAW_DATA_SIZE           (400)

// for state
#define REMOCON_IDLE            (0)
#define REMOCON_RECODE          (1)
#define REMOCON_SEND            (2)

#define MAX_PERIOD              (580)
#define EXTRA_PERIOD            (100)

class remocon {
  public:
    uint8_t state = REMOCON_IDLE;
    uint16_t period;
    String irData;
    String chName;

    int sendSignal(void);
    int recodeSignal(void);

    void dataBackupToFile(String path);
    void dataRestoreFromFile(String path);

    void dispRawData(uint16_t* rawData);
    void dispData(void);
  private:
    void raw2bin(uint16_t *rawData);
    void bin2raw(uint16_t *rawData);
};

extern remocon ir[IR_CH_SIZE];

#endif

