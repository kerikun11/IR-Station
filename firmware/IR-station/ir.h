#ifndef IR_SIGNAL_H
#define IR_SIGNAL_H

#include <ESP8266WiFi.h>

#define IR_RECEIVE_TIMEOUT_US   200000
#define RAWDATA_BUFFER_SIZE     800
#define REGARD_AS_NOISE_COUNT   8

enum IR_RECEIVER_STATE {
  IR_RECEIVER_OFF,
  IR_RECEIVER_READY,
  IR_RECEIVER_RECEIVING,
  IR_RECEIVER_READING,
  IR_RECEIVER_AVAILABLE,
};

class IR {
  public:
    static void begin(int tx, int rx);
    static void handle();
    static bool available();
    static String read();
    static void resume();

    static void send(String dataJson);

  private:
    static int txPin, rxPin;
    static volatile enum IR_RECEIVER_STATE state;
    static volatile uint16_t rawIndex;
    static volatile uint16_t rawData[RAWDATA_BUFFER_SIZE];
    static volatile uint32_t prev_us;
    static String irJson;

    static void isr();
};

#endif

