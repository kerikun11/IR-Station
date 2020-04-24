#include "myssdp.h"
const char* _ssdp_schema_template PROGMEM =
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/xml\r\n"
  "Connection: close\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "\r\n"
  "<?xml version=\"1.0\"?>"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
  "<specVersion>"
  "<major>1</major>"
  "<minor>0</minor>"
  "</specVersion>"
  "<URLBase>http://%s:%u/</URLBase>" // WiFi.localIP(), _port
  "<device>"
  "<deviceType>%s</deviceType>"
  "<friendlyName>%s</friendlyName>"
  "<presentationURL>%s</presentationURL>"
  "<serialNumber>%s</serialNumber>"
  "<modelName>%s</modelName>"
  "<modelNumber>%s</modelNumber>"
  "<modelURL>%s</modelURL>"
  "<manufacturer>%s</manufacturer>"
  "<manufacturerURL>%s</manufacturerURL>"
  "<UDN>%s</UDN>"
  "</device>"
  "</root>\r\n"
  "\r\n";

uint16_t MySSDPClass::strlen(IPAddress ip) {
  return strlen_P(ip.toString().c_str())+
				 2+
         strlen_P(_deviceType)+
         strlen_P(_friendlyName)+
         strlen_P(_presentationURL)+
         strlen_P(_serialNumber)+
         strlen_P(_modelName)+
         strlen_P(_modelNumber)+
				 strlen_P(_modelURL)+
         strlen_P(_manufacturer)+
         strlen_P(_manufacturerURL)+
         strlen_P(_uuid);
}

char* MySSDPClass::schema(IPAddress ip, char* buffer, uint16_t size) const {
  strcpy_P(buffer, _ssdp_schema_template);
  snprintf_P(buffer, size, _ssdp_schema_template,
             ip.toString().c_str(),
						 _port,
             _deviceType,
             _friendlyName,
             _presentationURL,
             _serialNumber,
             _modelName,
             _modelNumber,
             _modelURL,
             _manufacturer,
             _manufacturerURL,
             _uuid
            );
}
