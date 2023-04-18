/////////////////////////
//  Packaging Defines
/////////////////////////
/*
  Defined Packet Structure:
    5 Total Bytes:
      -1 Metadata byte
      -3 Data bytes
      -1 Stop byte
    Meta Data Byte Structure:
      (PPPSSTTT)
        -3 Bits for Port #
        -2 Bits for Port Status
        -3 Bits for Sensor Type
    Data Bytes:
      Temperature/Humidity Sensor:
        -First Data Byte = 0
        -Second Data Byte = Data
        -Third Data Byte = 0;
        -X2 Packets but from same
        port since they are on same
        sensor board
      Vibration and Sound Sensors:
        -First Data Byte = FFT Index (0-254)
        -Second Data Byte = FFT Bin
        -Third Data Byte = FFT Bin
    Stop Byte:
      This byte = 255 = 0b11111111, closes
      the packet transmission for MSP432
*/
////// Port Number //////
#define PORT1 0  // 0b00000000
#define PORT2 32 // 0b00100000
#define PORT3 64 // 0b01000000
#define PORT4 96 // 0b01100000
////// Port Status //////
#define DISCONNECTED   0  // 0b00000000
#define CONNWITHDATA   8  // 0b00001000
#define CONNWITHNODATA 16 // 0b00010000
#define MESSAGE        24 // 0b00011000
////// Sensor Type ///////
#define TEMPSENS  0  // 0b00000000
#define HUMSENS   1  // 0b00000001
#define VIBSENS   2  // 0b00000010
#define SOUNDSENS 3  // 0b00000011
#define LOWBATT   4  // 0b00000100
#define OTHERERR  5  // 0b00000101
#define COLLSTOP  6  // 0b00000110
#define COLLSTART 7  // 0b00000111
///////////////////////////
//  END Packaging Defines
///////////////////////////

///////////////////////////
//  Packaging Variables
///////////////////////////
// Port 1
byte port1Formatted[1275];
byte port1Header = 0;
int port1FormattedSize;
bool port1DataReady = false;
// Port 2
byte port2Formatted[1275];
byte port2Header = 0;
int port2FormattedSize;
bool port2DataReady = false;
// Port 3
byte port3Formatted[1275];
byte port3Header = 0;
int port3FormattedSize;
bool port3DataReady = false;
// Port 4
byte port4Formatted[1275];
byte port4Header = 0;
int port4FormattedSize;
bool port4DataReady = false;
///////////////////////////
//  END Packaging Variables
///////////////////////////