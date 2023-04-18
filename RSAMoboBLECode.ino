#include "src/SparkFun_SinglePairEthernet.h"
#include "SPEVarDef.h"
#include "PackagingVarDef.h"
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
        The setups only function is to intitalize the 
        software serial that is used to communicate with
        the HC-05 BT module that we use to communicate with
        the Handheld device
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
}

void loop() {

  ServicePorts();
  SendPackets();

  unsigned long now = millis();
  if(now - lastBlink >= 1000)
  {;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); 
    lastBlink = now;
  }
}
