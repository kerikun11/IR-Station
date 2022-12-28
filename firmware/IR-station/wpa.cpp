/**
  The MIT License (MIT)
  Copyright (c)  2016  Ryotaro Onuki

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "wpa.h"

/* defined in wpa_supplicant library */
extern "C" int pbkdf2_sha1(
  const char * passphrase,
  const char * ssid,
  size_t ssid_len,
  int iterations,
  u8 * buf,
  size_t buflen
);

String calcWPAPassPhrase(const String &ssid, const String &password) {
  const int s = 32; // 256 bit
  u8 buf[s];
  pbkdf2_sha1(password.c_str(), ssid.c_str(), ssid.length(), 4096, buf, s);
  String hash;
  for(int i=0; i<s; ++i)
    hash += String(buf[i] >> 4, HEX) + String(buf[i] & 0x0f, HEX);
  hash.toLowerCase();
  return hash;
}
