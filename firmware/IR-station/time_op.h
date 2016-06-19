#ifndef time_op_h
#define time_op_h

#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

#include "config.h"

extern const char weekdayCharJp[8][4];  /**< 日本語の曜日名 */
extern const char weekdayCharEn3[8][4]; /**< 英語の曜日名の上3文字 */

/** A fuction that sets up time operation. */
void setupTime();

/** A function that gets time by NTP
  @return epoch time
*/
time_t getNtpTime();

/** A function that sends a NTP packet
  @param target IP address (NTP server's IP address)
*/
void sendNTPpacket(IPAddress &address);

#endif

