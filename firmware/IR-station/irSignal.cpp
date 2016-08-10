#include "irSignal.h"

#include <ArduinoJson.h>
#include "config.h"

void IR_Signal::dispData(void) {
  //  print_dbg("Ch Name: ");
  //  println_dbg(chName);
  //  print_dbg("Period: ");
  //  println_dbg(period, DEC);
  //  print_dbg("IR Data: ");
  //  println_dbg(irData);
}

String IR_Signal::getBackupString(void) {
  println_dbg("Generating IR data in Json...");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["chName"] = chName;
  data["rawDataLength"] = rawDataLength;
  JsonArray& rawDataArray = data.createNestedArray("rawData");
  for (int i = 0; i < rawDataLength; i++) {
    rawDataArray.add(rawData[i]);
  }
  String str;
  data.printTo(str);
  return str;
}

void IR_Signal::restoreFromString(String dataString) {
  println_dbg("Psing IR data from Json...");
  println_dbg("data: " + dataString);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(dataString);
  chName = (const char*)data["chName"];
  rawDataLength = (uint16_t)data["rawDataLength"];
  for (int i = 0; i < rawDataLength; i++) {
    rawData[i] = (uint16_t)data["rawData"][i];
  }
}
