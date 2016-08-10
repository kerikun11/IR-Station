#include "file.h"

#include <FS.h>
#include "config.h"

bool writeStringToFile(String path, String dataString) {
  SPIFFS.remove(path);
  File file = SPIFFS.open(path, "w");
  if (!file) {
    println_dbg("File open Error: " + path);
    return false;
  }
  file.print(dataString);
  file.close();
  println_dbg("Backup successful: " + path);
  println_dbg("data: " + dataString);
  return true;
}

bool getStringFromFile(String path, String& dataString) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    println_dbg("File open Error: " + path);
    return false;
  }
  file.setTimeout(10);
  dataString = file.readString();
  file.close();
  println_dbg("Restore successful: " + path + " data:" + dataString);
  return true;
}

bool removeFile(String path) {
  println_dbg("Removed: " + path);
  return SPIFFS.remove(path);
}

