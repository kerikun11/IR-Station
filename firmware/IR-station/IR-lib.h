#include <arduino.h>
#include "config.h"
#include "String_op.h"

#ifndef IR_OPERATION_H
#define IR_OPERATION_H

// IR recode timeouts
#define TIMEOUT_RECODE_NOSIGNAL (40000)
#define TIMEOUT_RECODE          (5000000)

// for period
#define MIN_PERIOD              (200)
#define MAX_PERIOD              (580)
#define EXTRA_PERIOD            (100)

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
};

#endif

