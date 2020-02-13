
/* 
 * ==== simple function for custom CMDhandler/REST API: send msg with trailing \r\n ====
 */
void send_cmd_rn( String cmd )
{
  String rs232_answer;
  do_rs232(cmd + "\r\n");
  rs232_answer = String(myBuffer);
  server.send ( 200, "application/json", "{ \"result\": \"" + rs232_answer.substring(0,rs232_answer.indexOf("\r")) + "\" }" );
}



/*
 * add own handler for set-value commands
 * - inspired by https://forum.arduino.cc/index.php?topic=453310.0
 * 
 */
class CMDhandler : public RequestHandler {
  String APIstartseq = "/v1/CMD/";
  
  bool canHandle(HTTPMethod method, String uri) {
    return uri != NULL && uri.startsWith( APIstartseq );
  }

  bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {   
    String cmd;
    String val;
    String payload = requestUri.substring( APIstartseq.length() );
    Serial.println(payload);
    int posSlash = payload.indexOf("/");  // if any, else -1
    Serial.println(posSlash);
    
    if  ( posSlash < 0 )
    {
      cmd = payload;
      send_cmd_rn( payload );   // send simple command like PA_NEW
    } 
    else
    {
      cmd = payload.substring(0,posSlash);
      val = payload.substring(posSlash+1);
      Serial.println(cmd);
      Serial.println(val);
      send_cmd_rn(cmd + " " + val);   // send command with parameter like OUT_PV_5 120
    }
  
  }
} myCMDhandler;



//-- basic interface: text in, text out; no html
//   * just get payload, write to rs232 and return answer as plain text
void heidolph_rs232_text(){
  
  String payload = server.arg("payload");
  Serial.println("Payload sent is: " + payload);
  
  String rs232_answer = "(none)";
  if (payload != "" ){
    do_rs232(payload + "\r\n");
    rs232_answer = String(myBuffer);
  }
  server.send ( 200,"text/plain", rs232_answer );
}


/*
 * == root web page ==
 */
void handleRoot() {
  server.send ( 200, "text/html", "<html><body style='font-family: Sans-Serif;'><h2>Heidolph Stirrer MR Hei-Tec</h2><p>Find a html frontend <a href='/heidolph/html'>here</a></p></body></html>" );
}


/*
 * == http 404 handler ==
 */
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";          message +=   server.uri();
  message += "\nMethod: ";     message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";  message += server.args();
  message + "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );
}


/* 
 *  Function presents a html form which posts a stirrer command in 'payload' to the handler.
 *  This payload then is sent via RS232 to an attached device.
 */

//-- main handler for http to rs232
void heidolph_rs232_demo(){
  String payload = server.arg("payload");
  Serial.println("Payload sent is: " + payload);
  
  String rs232_answer = "(none)";
  if (payload != "" ){
    do_rs232(payload + "\r\n");
    rs232_answer = String(myBuffer);
  }

  String html = "<html><body style='font-family: Sans-Serif;'>\n<h2>WebToRS232 gateway with Heidolph Stirrer</h2>";
  html += "<form action='/heidolph/html' method='POST'>Choose command:<br/> \
    <fieldset border=0>\
      <input type='radio' name='payload' value='PA_NEW'> init() stirrer<br>\
      <input type='radio' name='payload' value='START_2'> start rotation<br/>\
      <input type='radio' name='payload' value='STOP_2'> stop rotation<br/>\
      <input type='radio' name='payload' value='OUT_SP_3 120'> rotation -> 120 rpm<br/>\
      <input type='radio' name='payload' value='OUT_SP_3 240'> rotation -> 240 rpm<br/>\
      <input type='radio' name='payload' value='IN_PV_5' checked> show current rpm<br/>\
      <input type='radio' name='payload' value='STATUS'> show status (0=manually; 1=remote,started; 2=remote,stopped)<br/>\
      <input type='radio' name='payload' value='SW_VERS'> show software version<br/>\
    </fieldset>\
    <input type='submit' value='Submit'>  </form><br/>\n\
    <table border=1 cellspacing=0 cellpadding=7> \
  ";
  html += "<tr><td>payload sent: </td><td>"   + payload      + "</td></tr>\n";
  html += "<tr><td>read from rs232:</td><td>" + rs232_answer + "</td></tr>\n";
  html += "</table></body></html>";
  server.send ( 200,"text/html", html );
}


/*
 * -- send all values as json -------
 */
void read_all_values()
{
  String rs232_answer;

  String inner_json = "";
  #define num_commands 4
  String commands[num_commands] = {"STATUS", "IN_PV_1", "IN_PV_3", "IN_PV_5"};
  for ( int i=0; i<num_commands; i++ ) 
  {
    String CMD = commands[i];
    do_rs232( CMD + "\r\n");
    rs232_answer = String(myBuffer);
    int result_i = -99;
    if ( rs232_answer.startsWith( CMD ) )
    {
      String result_raw = rs232_answer.substring(0,rs232_answer.indexOf("\r"));
      int    result_i   = rs232_answer.substring(CMD.length()+1).toFloat();
      String s_result   = "  \""+ CMD +"\": { \"raw\": \""+ result_raw +"\", \"value\": "+ result_i +"},\n";
      inner_json += s_result;
    }
    else
    {
      inner_json += "  \""+ CMD +"\": null,\n";
    }
    // IN_PV_1
  }
  String full_json = "{\n" + inner_json + "  \"version\": 1\n}";
  
  server.send ( 200, "application/json", full_json);
}


