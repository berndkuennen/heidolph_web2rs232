/*
 * ## heidolph-http-adapter ##
 * - v1.2: 2020-01-30/Bernd Künnen
 * - adds a http interface to Heidolph stirrer with RS232 connector
 * - runs on ESP32
 * - some commands available via REST API
 * - define basic settings (wifi etc.) in config.h
 * - inspired by https://github.com/bbx10/WebServer_tng/tree/master/examples
 * 
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "HardwareSerial.h"	//-- works with ESP32 built-in hardware serial
#include "config.h"		//-- define serial and wifi settings in separate config file

/*
 *-- wifi password in own git-ignored file
 * set the following in wifi.h:
 * const char *ssid     = "Wifi-name";
 * const char *password = "wifi-pass";
 *
 */
#include "wifi.h"

//-- global objects --
WebServer server ( 80 );
HardwareSerial   mySerial(2);

//-- raw read buffer definition --
#define bufsz 512
char    myBuffer[bufsz];
int     bufix = 0;
String  hx = "";


//-- handlers, functions etc.
#include "serialstuff.h"
#include "http-handler.h"


//== setup ========================
void setup ( void ) {
  Serial.begin ( SER_BAUD );

  //-- setup wifi --
  WiFi.begin ( ssid, password );
  Serial.println ( "" );
  //-- Wait for connection --
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println ();
  Serial.print( "Connected to " );  Serial.println ( ssid );
  Serial.print( "IP address: " );   Serial.println ( WiFi.localIP() );

  //-- setup mDNS --
  if ( MDNS.begin ( "esp32" ) ) {
    Serial.println ( "MDNS responder started" );
  }
  else
  {
    Serial.println ( "MDNS responder FAILED" );
  }

  //-- setup web server --
  server.on( "/", handleRoot );
  server.onNotFound ( handleNotFound );

  //-- demo pages --
  server.on( "/heidolph/html", heidolph_rs232_demo );
  server.on( "/heidolph/text", heidolph_rs232_text );

  // REST API
  server.on( "/v1/VAL/ALL", read_all_values );
  server.addHandler(&myCMDhandler);  // uri starts with /v1/CMD/
  
  server.begin();
  Serial.println ( "HTTP server started" );

  //-- setup hardware serial interface --
  mySerial.begin(HWS_BAUD, HWS_PROTOCOL, RXD2, TXD2 );
  delay(1000);
}


//== main loop ====================
void loop ( void ) {
  // just take care of the web requests
  server.handleClient();
}


