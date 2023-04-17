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

byte port1Formatted[1275];
byte port1Header = 0;
int port1FormattedSize;
bool port1DataReady = false;

SinglePairEthernet port1;
SoftwareSerial Bluetooth = SoftwareSerial(rxPin, txPin);

int port1MessageSize;
byte port1RecBuffer[1000];
byte port1MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port1RecMAC[6];

unsigned long lastBlink = 0;

static void rxCallback(byte * data, int dataLen, byte * senderMac)
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

void setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Bluetooth.begin(57600);
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
  while (port1.getLinkStatus() != true);
}

void loop() {

  if (!port1.getLinkStatus())
  {
    port1Header = port1Header | PORT1;
    port1Header = port1Header | DISCONNECTED;
    port1Formatted[0] = port1Header;
    port1Formatted[1] = 0;
    port1Formatted[2] = 0;
    port1Formatted[3] = 0;
    port1Formatted[4] = 255;
    port1FormattedSize = 5;
    for(int i = 0; i < port1FormattedSize; i++)
    {
      Bluetooth.write(port1Formatted[i]);
      delay(2);
    }
  }
  if (port1DataReady)
  {
    for(int i = 0; i < port1FormattedSize; i++)
    {
      Bluetooth.write(port1Formatted[i]);
      delay(2);
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
