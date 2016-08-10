#ifndef FILE_H
#define FILE_H

#include <ESP8266WiFi.h>

bool writeStringToFile(String path, String dataString);
bool getStringFromFile(String path, String & dataString);
bool removeFile(String path);

#endif

