# IR Station

Infrared Remote Controller with ESP8266 WiFi-module  
You can control your home appliances with your smartphone or laptop.

## IR data JSON format

### Format

array of microseconds (type: uint16_t)

~~~json
[
	 "microseconds_high", "microseconds_low (uint16_t)", "microseconds_high (uint16_t)", "microseconds_low (uint16_t)", ...
]
~~~

### Example

~~~json
[
	3300,1786,363,1305,390,473,364,476,317,548,289,577,288,501,364,474,390,475,290,550,289,1403,290,551,314,1377,290,552,313,502,362,1379,289,1378,289,1378,314,1377,389,1304,364,476,312,552,290,553,312,474,391,1305,362,501,364,474,361,504,289,553,312,477,363,500,364,476,287,578,289,1377,314,551,290,502,363,476,388,474,391,451,313,551,313,553,288,500,363,1304,385,1362,310,475,389,475,364,477,313,552,288,1380,317,50215,3305,3489,314
]
~~~

## HTTP API v1.5.1

### Setup Form

|Path				|Method	|Parameter(s)	|Return	|Remarks	|
|:------------------|:------|:--------------|:------|:----------|
|/					|GET	|				|index.html	|setup form page	|
|/wifi/list			|GET	|				|a list of (string)	|a list of existing WiFi SSID 	|
|/wifi/confirm		|POST	|				|IP Address or "false"		|confirm if WiFi connection is established and reboot the device	|
|/mode/station		|POST	|ssid, password, stealth, hostname	|message	|set the device as Station Mode	|
|/mode/accesspoint	|POST	|hostname	|message	|set the device as AP Mode	|
|/dbg				|GET	|ssid, password	|local_ip or "false"	|

### Main Page

|Path				|Method	|Parameter(s)			|Return		|Remarks	|
|:------------------|:------|:----------------------|:----------|:----------|
|/					|GET	|						|index.html	|main page of IR-Station	|
|/info				|GET	|						|station.json	|a json includes the device information	|
|/signals/send		|POST	|id						|message	|			|
|/signals/record	|POST	|row, column, name		|message	|			|
|/signals/rename	|POST	|id, name				|message	|			|
|/signals/move		|POST	|id, row, column		|message	|			|
|/signals/upload	|POST	|irJson, row, column	|message	|			|
|/signals/clear		|POST	|id						|message	|			|
|/signals/clear-all	|POST	|						|message	|			|
|/schedule/new		|POST	|id, time				|message	|			|
|/schedule/delete	|POST	|schedule_id			|message	|			|
|/wifi/disconnect	|POST	|						|none		|			|
|/wifi/change-ip	|POST	|local_ip, subnetmask, gateway	|message|automatically redirect to a new IP Address		|
