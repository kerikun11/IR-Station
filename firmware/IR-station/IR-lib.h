#include <arduino.h>
#include "config.h"

#ifndef IR_OPERATION_H
#define IR_OPERATION_H

// IR recode timeouts
#define TIMEOUT_RECODE_NOSIGNAL (40000)   // [us]
#define TIMEOUT_RECODE          (5000000) // [us]

// for period
#define MIN_PERIOD              (200)     // [us]
#define MAX_PERIOD              (580)     // [us]
#define EXTRA_PERIOD            (100)     // [us]

class remocon {
  public:
    uint16_t period;
    String irData;
    String chName;

    void sendSignal(void);
    int recodeSignal(void);

    String getBackupString(void);
    void restoreFromString(String dataString);

    void dispData(void);

  private:
    // extracts a string between "head" and "tail"
    String extract(String target, String head, String tail = "&");
};

#endif

