/*
 * config.h for heidolph http interface -
 * - define serial and wifi settings here
 * 
 */


//-- default serial settings ---------------------
#define SER_BAUD 115200  // for USB debugging

//-- harware serial settings ---------------------
#define HWS_BAUD     9600
#define HWS_PROTOCOL SERIAL_7E1   // most used otherwise is SERIAL_8N1
#define HWS_DELAY    100          // ms between RX to and TX from decice
#define RXD2 16                   // default for 3rd UART is 16
#define TXD2 17                   // default for 3rd UART is 17
