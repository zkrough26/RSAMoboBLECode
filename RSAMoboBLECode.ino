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

////////////////////////////
// Bluetooth Var/Def
////////////////////////////
#define txPin G10
#define rxPin G11
SoftwareSerial Bluetooth = SoftwareSerial(rxPin, txPin);
////////////////////////////
// END Bluetooth Var/Def
////////////////////////////

////////////////////////////
//  SPE Instance Creation
////////////////////////////
SinglePairEthernet port1;
SinglePairEthernet port2;
SinglePairEthernet port3;
SinglePairEthernet port4;
////////////////////////////
// END SPE Instance Creation
////////////////////////////

unsigned long lastBlink = 0;  // Heartbeat LED Timer Counter

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

  unsigned long now = millis();
  if(now - lastBlink >= 1000)
  {;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); 
    lastBlink = now;
  }
}
