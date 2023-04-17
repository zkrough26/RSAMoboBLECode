#include "SparkFun_SinglePairEthernet.h"
#include <SoftwareSerial.h>

#define RST CS
#define port1CS G0
#define port1INT G1
#define txPin G10
#define rxPin G11

#define PORT1 0
#define PORT2 32
#define PORT3 64
#define PORT4 96

#define DISCONNECTED   0
#define CONNWITHDATA   8
#define CONNWITHNODATA 16
#define MESSAGE        24

#define TEMPSENS  0
#define HUMSENS   1
#define VIBSENS   2
#define SOUNDSENS 3
#define LOWBATT   4
#define OTHERERR  5
#define COLLSTOP  6
#define COLLSTART 7

SoftwareSerial Bluetooth = SoftwareSerial(rxPin, txPin);

///////////////////////////
//  Packaging Variables
///////////////////////////
byte port1Formatted[1275];
byte port1Header = 0;
int port1FormattedSize;
bool port1DataReady = false;
///////////////////////////
//  End Packaging Variables
///////////////////////////

///////////////////////////
//  SPE Port1 Var/Def
///////////////////////////
SinglePairEthernet port1;
int port1MessageSize;
byte port1RecBuffer[1000];
byte port1MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port1RecMAC[6];
bool port1LinkStatus = false;
///////////////////////////
//  SPE Port2 Var/Def
///////////////////////////
SinglePairEthernet port2;
int port2MessageSize;
byte port2RecBuffer[1000];
byte port2MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port2RecMAC[6];
bool port2LinkStatus = false;

unsigned long lastBlink = 0;

///////////////////////////
// SPE Per Port rxCallback
///////////////////////////
static void port1RxCallback(byte * data, int dataLen, byte * senderMac)
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

static void port2RxCallback(byte * data, int dataLen, byte * senderMac)
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

///////////////////////////
// SPE Per Port Link
//  Status Callback
///////////////////////////
void port1LinkCallback(bool linkStatus)
{
    port1LinkStatus = linkStatus;
}
void port2LinkCallback(bool linkStatus)
{
    port2LinkStatus = linkStatus;
}

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
  port1.setRxCallback(rxCallback);
  port1.setLinkCallback(port1LinkCallback);
  /////////////////////////
  // END SPE Port 1 Init
  /////////////////////////
}

void loop() {

  if (port1DataReady)
  {
    for(int i = 0; i < port1FormattedSize; i++)
    {
      Bluetooth.write(port1Formatted[i]);
      delay(1);
    }
    port1DataReady = false;
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
  if(port1.getLinkStatus())
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
}
