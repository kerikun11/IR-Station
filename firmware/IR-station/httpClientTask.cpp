#include "httpClientTask.h"

#include <ArduinoJson.h>
#include "config.h"
#include "timeTask.h"
#include "ir-stationTask.h"

#define SERVER_HOSTNAME   "ir-station.link"
#define SERVER_PORT       80

bool notifyIP(bool isUpdate) {
  WiFiClient client;
  if (!client.connect(SERVER_HOSTNAME, SERVER_PORT)) {
    println_dbg("Error: Connection Failed");
    return false;
  }
  String local_ip = String(WiFi.localIP()[0], DEC) + "." + String(WiFi.localIP()[1], DEC) + "." + String(WiFi.localIP()[2], DEC) + "." + String(WiFi.localIP()[3], DEC);
  client.println((String)"GET /app.rb?notify=" + (isUpdate ? "update" : "remove") + "&name=" + station.hostname + "&local_ip=" + local_ip + " HTTP/1.1");
  client.println("Host: "SERVER_HOSTNAME);
  client.println("");

  uint32_t timestamp = millis();
  uint32_t timeout = 2000;
  /* サーバーからの返答を待つ */
  while (true) {
    wdt_reset();

    /* サーバーから返答が来たら離脱 */
    if (client.available()) break;

    /* タイムアウト処理 */
    if (millis() - timestamp > timeout) {
      println_dbg("Timeout: Read Server");
      return false;
    }
  }
  client.setTimeout(10);
  String resp = client.readString();
  resp.replace("\r", "");
  resp.replace("\n", " ");
  print_dbg("Response: ");
  println_dbg(resp);
  client.stop();

  return true;
}

void notifyTask() {
  static uint32_t prev_sec;
  static IPAddress prev_local_ip;
  uint32_t now_sec = now();
  IPAddress now_local_ip = WiFi.localIP();
  if ((now_local_ip != prev_local_ip) || (now_sec > prev_sec + 300)) {
    if (!notifyIP()) {
      delay(1000);
      notifyIP();
    }
    prev_local_ip = now_local_ip;
    prev_sec = now_sec;
  }
}
