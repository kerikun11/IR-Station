#ifndef STRING_OPERATION
#define STRING_OPERATION

#include <arduino.h>

// extracts a string between "head" and "tail"
String extract(String target, String head, String tail = "&");

// replace a number to the symbol
void charEncode(String &s);

#endif
