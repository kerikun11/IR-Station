/*
   config.h
*/
#ifndef CONFIG
#define CONFIG


/* Hardware Mapping */
#define IR_IN                 (12)
#define IR_OUT                (15)
#define SW0                   (0)
#define LED0                  (4)
#define LED1                  (16)


/* Software */
// for Remocon
#define IR_CH_SIZE            (24)

// SPIFFS saving path
#define IR_DATA_PATH(i)       ("/data/"+String(i,DEC)+".txt")
#define WIFI_DATA_PATH        ("/wifi.txt")

// SSID & Password of ESP8266 Access Point Mode
#define SOFTAP_SSID           "ESP8266-Remocon"
#define SOFTAP_PASS           ""

#define WIFI_CONNECT_TIMEOUT  (10) // seconds

// http://DEFAULT_MDNS_ADDRESS.local
#define DEFAULT_MDNS_ADDRESS  "esp8266"


/* for Debug */
#define print_dbg             Serial.print
#define println_dbg           Serial.println

#endif

