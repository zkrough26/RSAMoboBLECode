void SendPackets()
{
  if (!port1LinkStatus)
  {
    SendDisconPacket(1);
  }
  if (!port1LinkStatus)
  {
    SendDisconPacket(2);
  }
  if (port1DataReady){
    if (port1FirstPacket){
      SendInitPacket(1);
      port1FirstPacket = false;
    }
    for (int i = 0; i < port1FormattedSize; i++) {
      Bluetooth.write(port1Formatted[i]);
      delay(2);
    }
    port1DataReady = false;
  }
  if (port2DataReady){
    if (port2FirstPacket){
      SendInitPacket(2);
      port2FirstPacket = false;
    }
    for (int i = 0; i < port2FormattedSize; i++) {
      Bluetooth.write(port2Formatted[i]);
      delay(2);
    }
    port2DataReady = false;
  }
}

void SendInitPacket(int port)
{
    switch (port){
        case 1:
            Bluetooth.write(PORT1 | CONNWITHNODATA | port1RecMAC[5]);
            break;
        case 2:
            Bluetooth.write(PORT2 | CONNWITHNODATA | port2RecMAC[5]);
            break;
        case 3:
            Bluetooth.write(PORT3 | CONNWITHNODATA | port3RecMAC[5]);
            break;
        case 4:
            Bluetooth.write(PORT4 | CONNWITHNODATA | port4RecMAC[5]);
            break;
    }
    Bluetooth.write((byte)0);
    Bluetooth.write((byte)0);
    Bluetooth.write((byte)0);
    Bluetooth.write((byte)255);
}
