/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "file.h"

#include <LittleFS.h>
#include "config.h"

bool writeStringToFile(const String& path, const String& dataString) {
  LittleFS.remove(path);
  File file = LittleFS.open(path, "w");
  if (!file) {
    print_dbg("File open Error: ");
    println_dbg(path);
    return false;
  }
  file.print(dataString);
  print_dbg("File Size: ");
  println_dbg(file.size(), DEC);
  file.close();
  print_dbg("Backup successful: ");
  println_dbg(path);
  print_dbg("data: ");
  println_dbg(dataString);
  return true;
}

bool getStringFromFile(const String& path, String& dataString) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    print_dbg("File open Error: ");
    println_dbg(path);
    return false;
  }
  print_dbg("File Size: ");
  println_dbg(file.size(), DEC);
  file.setTimeout(1);
  dataString = "";
  while (file.available()) {
    dataString += file.readString();
  }
  file.close();
  print_dbg("Restore successful: ");
  println_dbg(path);
  print_dbg("data: ");
  println_dbg(dataString);
  return true;
}

bool removeFile(const String& path) {
  println_dbg("Removed: " + path);
  return LittleFS.remove(path);
}

