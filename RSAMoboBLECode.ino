#include "SparkFun_SinglePairEthernet.h"
#include <SoftwareSerial.h>

/*
    This code is for Team 42s RSA Motherboard Artemis MCU

      The purpsose of this code is to recieve data via
      Single Pair Ethernet (SPE) from attached sensors.
      This data is then organized into packets following
      our define packet structure.  From there, the 
      packets are then sent via UART to the HC-05 Bluetooth
      module to the Handheld device.  

    Code Overview:
      Setup:
        Within the setup function, a couple of things happen.
        First, the UART communication for the HC-05 Bluetooth
        module is initialized.  Next, the ADIN1110 SPE
        Transcievers are initalized.  These are the chips that
        facilitate the SPE communications.  They are assigned
        their own recieve callback and link change functions.
        These functions are interrupt driven and will call
        automatically.  Each ADIN1110 or port, has its own
        recieved data buffer which is used to store the most
        recently recieved data.
      Main Loop:

*/

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
//  Bluetooth Var/Def
///////////////////////////
#define txPin G10
#define rxPin G11
SoftwareSerial Bluetooth = SoftwareSerial(rxPin, txPin);
///////////////////////////
//  END Bluetooth Var/Def
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
///////////////////////////
//  END Packaging Variables
///////////////////////////

///////////////////////////
//  SPE Port1 Var/Def
///////////////////////////
#define RST CS  // All ports use same RST line
#define port1CS G0
#define port1INT G1
SinglePairEthernet port1;
int port1MessageSize;
byte port1RecBuffer[1000];
byte port1MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port1RecMAC[6];
bool port1LinkStatus = false;
bool port1FirstPacket = true;
bool port1SendDiscon = false;
///////////////////////////
//  SPE Port2 Var/Def
///////////////////////////
#define port2CS G2
#define port2INT G3
SinglePairEthernet port2;
int port2MessageSize;
byte port2RecBuffer[1000];
byte port2MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port2RecMAC[6];
bool port2LinkStatus = false;
bool port2FirstPacket = true;
bool port2SendDiscon = false;
////////////////////////////
//  END All SPE Port Var/Def
////////////////////////////

unsigned long lastBlink = 0;  // Heartbeat LED Timer Counter

///////////////////////////
// SPE Per Port rxCallback
///////////////////////////
static void port1RxCallback(byte * data, int dataLen, byte * senderMac)
{
  if (!port1DataReady)
  {
    noInterrupts();
    port1MessageSize = dataLen;
    for(int i = 0; i < 6; i++)
    {
      port1RecMAC[i] = senderMac[i];
    }
    for(int i = 0; i < dataLen; i++)
    {
      port1RecBuffer[i] = data[i];
    }
    ConvertToPacketStruct();
    port1DataReady = true;
    interrupts();
  }
}

static void port2RxCallback(byte * data, int dataLen, byte * senderMac)
{
  if(!port2DataReady)
  {
    noInterrupts();
    port2MessageSize = dataLen;
    for(int i = 0; i < 6; i++)
    {
      port2RecMAC[i] = senderMac[i];
    }
    for(int i = 0; i < dataLen; i++)
    {
      port2RecBuffer[i] = data[i];
    }
    ConvertToPacketStruct();
    port2DataReady = true;
    interrupts();
  }
}
///////////////////////////
//  END SPE rxCallbacks
///////////////////////////

///////////////////////////
// SPE Per Port Link
//  Status Callback
///////////////////////////
void port1LinkCallback(bool linkStatus)
{
    port1LinkStatus = linkStatus;
    if (port1LinkStatus) {
      port1FirstPacket = true;
    }
    else {
      port1DataReady = false;
      port1SendDiscon = true;
    }
}
void port2LinkCallback(bool linkStatus)
{
    port2LinkStatus = linkStatus;
    if (port2LinkStatus) {
      port2FirstPacket = true;
    }
    else {
      port2DataReady = false;
      port2SendDiscon = true;
    }
}
///////////////////////////
// END SPE Link callback
///////////////////////////

void setup() {
  /////////////////////////
  // Bluetooth UART Init
  /////////////////////////
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Bluetooth.begin(57600);
  /////////////////////////
  // END Bluetooth UART
  //       Init
  /////////////////////////
  /////////////////////////
  // SPE Port 1 Init
  /////////////////////////
  if (!port1.begin(port1MAC, LED_BUILTIN, port1INT, RST, port1CS))
  {
    while(1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }
  port1.setRxCallback(port1RxCallback);
  port1.setLinkCallback(port1LinkCallback);
  /////////////////////////
  // END SPE Port 1 Init
  /////////////////////////
  /////////////////////////
  // SPE Port 2 Init
  /////////////////////////
  if (!port2.begin(port2MAC, LED_BUILTIN, port2INT, RST, port2CS))
  {
    while(1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }
  port2.setRxCallback(port2RxCallback);
  port2.setLinkCallback(port2LinkCallback);
  //////////////////////////
  // END SPE Port 2 Init
  //////////////////////////
}

void loop() {

  if (port1DataReady && port1LinkStatus)
  {
    if(port1FirstPacket)
    {
      for (int i = 0; i < 5; i++)
    {
      Bluetooth.write(PORT1 | CONNWITHNODATA | port1RecMAC[5]);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)255);
      port1SendDiscon = false;
      delay(50);
    }
    }
    for(int i = 0; i < port1FormattedSize; i++)
    {
      Bluetooth.write(port1Formatted[i]);
      delay(1);
    }
    port1DataReady = false;
  }
  if (port1SendDiscon)
  {
    for (int i = 0; i < 5; i++)
    {
      Bluetooth.write(PORT1 | DISCONNECTED | port1RecMAC[5]);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)255);
      port1SendDiscon = false;
      delay(50);
    }
  }

  if (port2DataReady && port2LinkStatus)
  {
    if(port2FirstPacket)
    {
      for (int i = 0; i < 5; i++)
    {
      Bluetooth.write(PORT2 | CONNWITHNODATA | port2RecMAC[5]);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)255);
      port2SendDiscon = false;
      delay(50);
    }
    }
    for(int i = 0; i < port2FormattedSize; i++)
    {
      Bluetooth.write(port2Formatted[i]);
      delay(1);
    }
    port2DataReady = false;
  }
  if (port2SendDiscon)
  {
    for (int i = 0; i < 5; i++)
    {
      Bluetooth.write(PORT2 | DISCONNECTED | port2RecMAC[5]);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)255);
      port2SendDiscon = false;
      delay(50);
    }
  }
  
  unsigned long now = millis();
  if(now - lastBlink >= 1000)
  {;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); 
    lastBlink = now;
  }
}

void ConvertToPacketStruct()
{ 
  port1Header = 0;
  port1FormattedSize = 0;
  memset(port1Formatted, 0, sizeof(port1Formatted));
  if(port1LinkStatus)
  {
    port1Header = port1Header | PORT1;
    if ((port1MessageSize > 0) && (port1MessageSize <= 512))
    {
      port1Header = port1Header | CONNWITHDATA;
    }
    else
    {
      port1Header = port1Header | CONNWITHNODATA;
    }
    switch (port1RecMAC[5])
    {
      case 0:
        break;
      case 1:
        break;
      case 2:
        port1Header = port1Header | VIBSENS;
        break;
      case 3:
        port1Header = port1Header | SOUNDSENS;
        break;
      default:
        port1Header = port1Header | OTHERERR; 
    }
    if ((port1RecMAC[5] == 2) || (port1RecMAC[5] == 3))
    {
      for (int i = 0; i < (port1MessageSize / 2) - 1; i++)
      {
        port1Formatted[i * 5] = port1Header;
        port1Formatted[(i * 5) + 1] = i;
        port1Formatted[(i * 5) + 2] = port1RecBuffer[i * 2];
        port1Formatted[(i * 5) + 3] = port1RecBuffer[(i * 2) + 1];
        port1Formatted[(i * 5) + 4] = 255;
      }
      port1FormattedSize = sizeof(port1Formatted);
    }
    if (port1RecMAC[5] == 1)
    {
      port1Formatted[0] = port1Header | HUMSENS;
      port1Formatted[1] = 0;
      port1Formatted[2] = port1RecBuffer[0];
      port1Formatted[3] = 0;
      port1Formatted[4] = 255;
      port1Formatted[5] = port1Header | TEMPSENS;
      port1Formatted[6] = 0;
      port1Formatted[7] = port1RecBuffer[1];
      port1Formatted[8] = 0;
      port1Formatted[9] = 255;
      port1FormattedSize = 10;
    }
  }
  else
  {
    port1Header = port1Header | PORT1;
    port1Header = port1Header | DISCONNECTED;
    port1Formatted[0] = port1Header;
    port1Formatted[1] = 0;
    port1Formatted[2] = 0;
    port1Formatted[3] = 0;
    port1Formatted[4] = 255;
    port1FormattedSize = 5;
  }


  port2Header = 0;
  port2FormattedSize = 0;
  memset(port2Formatted, 0, sizeof(port2Formatted));
  if(port2LinkStatus)
  {
    port2Header = port2Header | PORT2;
    if ((port2MessageSize > 0) && (port2MessageSize <= 512))
    {
      port2Header = port2Header | CONNWITHDATA;
    }
    else
    {
      port2Header = port2Header | CONNWITHNODATA;
    }
    switch (port2RecMAC[5])
    {
      case 0:
        break;
      case 1:
        break;
      case 2:
        port2Header = port2Header | VIBSENS;
        break;
      case 3:
        port2Header = port2Header | SOUNDSENS;
        break;
      default:
        port2Header = port2Header | OTHERERR; 
    }
    if ((port2RecMAC[5] == 2) || (port2RecMAC[5] == 3))
    {
      for (int i = 0; i < (port2MessageSize / 2) - 1; i++)
      {
        port2Formatted[i * 5] = port2Header;
        port2Formatted[(i * 5) + 1] = i;
        port2Formatted[(i * 5) + 2] = port2RecBuffer[i * 2];
        port2Formatted[(i * 5) + 3] = port2RecBuffer[(i * 2) + 1];
        port2Formatted[(i * 5) + 4] = 255;
      }
      port2FormattedSize = sizeof(port2Formatted);
    }
    if (port2RecMAC[5] == 1)
    {
      port2Formatted[0] = port2Header | HUMSENS;
      port2Formatted[1] = 0;
      port2Formatted[2] = port2RecBuffer[0];
      port2Formatted[3] = 0;
      port2Formatted[4] = 255;
      port2Formatted[5] = port2Header | TEMPSENS;
      port2Formatted[6] = 0;
      port2Formatted[7] = port2RecBuffer[1];
      port2Formatted[8] = 0;
      port2Formatted[9] = 255;
      port2FormattedSize = 10;
    }
  }
  else
  {
    port2Header = port2Header | PORT2;
    port2Header = port2Header | DISCONNECTED;
    port2Formatted[0] = port2Header;
    port2Formatted[1] = 0;
    port2Formatted[2] = 0;
    port2Formatted[3] = 0;
    port2Formatted[4] = 255;
    port2FormattedSize = 5;
  }
}
