
//==== serial section ================

//-- transmit->do()
void serial_tx_bytes(String msg){
  int s = msg.length();
  Serial.println("msg = " + msg);
  Serial.println("size of msg = " + String(s) + " bytes - sending...");
  if (s>0){
    for ( int n=0; n<s; n++) {
      mySerial.write(msg.charAt(n));
      //Serial.print(msg[n]);
    }
  }
}

//-- receive->do()
int serial_rx_bytes(){
  int n = 0;
  while ( mySerial.available() && n<bufsz) {   
    char c = mySerial.read();
    // Serial.print(c);
    myBuffer[n] = c;
    n++;
  }
  myBuffer[n] = '\0';
  return n;
}

//-- send data via rs232 and get some answer
void do_rs232(String msg){
    serial_tx_bytes( msg );
    delay( HWS_DELAY );
    int bytes_read = serial_rx_bytes();
    if (bytes_read >0) {
      myBuffer[bytes_read] = '\0';
    }
    Serial.println("Read " + String(bytes_read) + " bytes from serial: " + String(myBuffer) );
}
