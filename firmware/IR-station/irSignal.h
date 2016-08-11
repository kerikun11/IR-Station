#ifndef IR_Signal_H
#define IR_Signal_H

#include <ESP8266WiFi.h>

#define IR_RECEIVE_TIMEOUT_US   200000

#define RAWDATA_BUFFER_SIZE     800

enum IR_RECEIVER_STATE {
  IR_RECEIVER_OFF,
  IR_RECEIVER_READY,
  IR_RECEIVER_RECEIVING,
  IR_RECEIVER_READING,
  IR_RECEIVER_AVAILABLE,
};

class IR_Signal {
  public:
    IR_Signal(uint8_t tx, uint8_t rx);
    uint8_t txPin, rxPin;

    bool available();
    String read();
    void resume();
    void send(String dataJson);

    void isr();
    void handle();

  private:
    volatile enum IR_RECEIVER_STATE state;
    volatile uint16_t rawIndex;
    volatile uint16_t rawData[RAWDATA_BUFFER_SIZE];
    volatile uint32_t prev_us = 0;

    String irJson;
};

#endif

