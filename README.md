# heidolph_web2rs232

## TL;DR
(en) This is an example or poc how to use an ESP32 to control a Heidolph MR Hei-Connect
stirrer by sending http requests via the microcontroller to the stirrers
RS232 interface.

It will enable you to control the stirrer (or many stirrers) not only from a
PC right beside the device but from anywhere. 

(de) Dies ist ein Beispiel oder auch Proof-Of-Concept für die Verwendung
eines ESP32 zur Steuerung eines
Heidolph MR Hei-Connect Rührers, indem Kommandos per http an den Mikrocontroller
und über diesen an die RS232-Schnittstelle des Rührers gesendet werden.

Das Projekt ermöglicht es, den Rührer nicht nur von einem dedizierten PC neben dem Gerät
zu steuern, sondern prinzipiell von überall und ggf. auch mehrere Rührer.


## Speak via http to a Heidolph MR Hei-Connect stirrer

The Heidolph Hei-Connect stirrer comes with a RS232 interface over which
commands may be submitted, e.g. turn on/off the heating, and values can be
read, such as current temperature and rpm. With this code an ESP32 is
connected to the serial line and it's possible to communicate with the
stirrer via http, with simple commands.

The code exposes these interfaces:
* a simple text interface, to be found at /heidolph/text, which accepts a command and returns the answer as plain text
* a small  html interface, to be found at /heidolph/html, with some example commands for demo purposes
* a REST API which returns json documents

### Configuration
* The ESP32 connects to Wifi. Place the credentials for your Wifi network in config.h.
* The Heidolph stirrer uses 9600 baud and 7E1 as communication parameters. Change this
  in config.h if you want to connect to a device with different needs. Set HWS_DELAY
  to ms for the delay between sending to and reading from the stirrer.

Wifi is configured in an own, git-ignored header file. Set the following in
wifi.h:
```
const char *ssid     = "Wifi-name";
const char *password = "wifi-pass";
```

### Wiring
On the RS232 side I used a TTL-RS232 module with D-Sub9 connector and a three wire null modem to connect to the
stirrer. The wiring of the cable is as follows:

Null modem cable:
* Pin2/TTL module (RX)  to Pin3/stirrer (TX)
* Pin3/TTL module (TX)  to Pin2/stirrer (RX)
* Pin5/TTL module (GND) to Pin5/stirrer (GND)

Wiring to the TTL module, pins at ESP32:
* VIN -> +5V
* GND -> ground
* RX2 -> RX
* TX2 -> TX


## The commands
The Heidolph Hei-Connect stirrers support a couple of commands which are
sent as plain text. The web interfaces use these commands, too, add the
CRLF and send it via serial line to the device.

Here's a small list of the most important commands:
* PA_NEW - set stirrer to new communication protocol (uses \r\n instead of \n\r as EOL)
* STATUS - get status code (numeric, see stirrer manual)
* SW_VERS - read software version
* RESET - switch off al functions and use old protocol again
* START_1 / STOP_1 - start/stop heating
* START_2 / STOP_2 - start/stop stirring
* OUT_PV_1 - set target temperature of solvent/heating plate
* OUT_PV_5 - set target rpm value
* IN_PV_1 - read temperature of solvent sensor
* IN_PV_3 - read temperature of heating plate
* IN_PV_5 - read rpm value 

You find a list of available commands in the
manual of your Heidolph stirrer.


## The simple http interface
The http interface at /heidolph/text accepts one parameter _payload_
which contains the text command (see above) to be sent via RS232.

The OUT* commands are followed by a space (which needs to be escaped as
*%20* when given as GET parameter) and a numeric value.

**Example:** A command line http request to set the rotation speed to 120 rpm would look like this:

```
$ curl http://ip.of.your.esp/heidolph/text?payload=OUT_PV_5%20120
OUT_PV_5 120
$
```

## The REST API
Under /v1 you'll find a RESTful API which allows you to send commands and
read values from the json response.

### /v1/CMD/$command/$parameter
Send the above mentioned stirrer commands as $command (in uppercase) and
some value to set as $parameter, if needed. The reponse is a simple json with the
key "result" while the value contains the answer that was sent by the stirrer.

**Examples:**
```
$ curl http://ip.of.your.esp/v1/CMD/PA_NEW # init stirrer to new protocol
{ "result": "PA_NEW" }

$ curl http://ip.of.your.esp/v1/CMD/OUT_SP_3/120 # set rotation to 120 rpm
{ "result": "OUT_SP_3 120.0" }

$ curl http://ip.of.your.esp/v1/CMD/START_2 # start rotation
{ "result": "START_2" }
```

### /v1/VAL/ALL
This uri returns the values that can be read from the stirrer. In version 1
these are:
* status
* speed of rotation (key=IN_PV5, rpm)
* temperatur of heating plate (key=IN_PV_X, °C) and sensor (key=IN_PV_Y°C)

Currently all values are given as int.

**Example:**
```
$ curl "http://ip.of.your.esp/v1/VAL/ALL"
{
  "STATUS":  { "raw": "STATUS 2",     "value":  2},
  "IN_PV_1": { "raw": "IN_PV_1 25.2", "value": 25},
  "IN_PV_3": { "raw": "IN_PV_3 26.7", "value": 26},
  "IN_PV_5": { "raw": "IN_PV_5 0.0",  "value":  0},
  "version": 1
}
```


## Python example
The file coffee.py contains some python code which shows an example how to
communicate via the simple http interface  and thus control the
stirrer remotely. In the example the stirrer is used to heat some water
to 97°C before heating and subsequently stirring is switched off again.





