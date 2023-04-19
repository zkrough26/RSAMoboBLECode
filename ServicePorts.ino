void ServicePorts()
{
  if (port1Active) {
    if (!port1.begin(port1MAC, LED_BUILTIN, port1INT, RST, port1CS)) {
      SendPortError(1);
    }
    else {
      port1.setRxCallback(port1RxCallback);
      for(int i = 0; i < 500; i++) {
        if(port1LinkStatus) {
          i = 500;
        }
        delay(1);
      }
      if (port1.getLinkStatus()) {
        if (!port1LinkStatus){
          port1FirstPacket = true;
        }
        port1LinkStatus = true;
        delay(1000);
      }
      else {
        port1LinkStatus = false;
        port1DataReady = false;
      }
    }
  }
  detachInterrupt(port1INT);
  if (port2Active) {
    if (!port2.begin(port2MAC, LED_BUILTIN, port2INT, RST, port2CS)) {
      SendPortError(2);
    }
    else
    {
      port2.setRxCallback(port2RxCallback);
      for(int i = 0; i < 500; i++) {
        if(port2LinkStatus) {
          i = 500;
        }
        delay(1);
      }
      if (port2.getLinkStatus()) {
        if (!port2LinkStatus){
          port2FirstPacket = true;
        }
        port2LinkStatus = true;
        delay(1000);
      }
      else {
        port2LinkStatus = false;
        port2DataReady = false;
      }
    }
  }
  detachInterrupt(port2INT);
}

void SendPortError(int port)
{
  switch(port){
    case 1:
      port1Active = false;
      Bluetooth.write(PORT1 | MESSAGE | OTHERERR);
      break;
    case 2:
      port2Active = false;
      Bluetooth.write(PORT2 | MESSAGE | OTHERERR);
      break;
    case 3:
      port3Active = false;
      Bluetooth.write(PORT3 | MESSAGE | OTHERERR);
      break;
    case 4:
      port4Active = false;
      Bluetooth.write(PORT4 | MESSAGE | OTHERERR);
      break;
  }
  Bluetooth.write((byte)0);
  Bluetooth.write((byte)0);
  Bluetooth.write((byte)0);
  Bluetooth.write((byte)255);
}

void SendDisconPacket(int port)
{
  switch(port){
    case 1:
      Bluetooth.write(PORT1 | DISCONNECTED | TEMPSENS);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)255);
      break;
    case 2:
      Bluetooth.write(PORT2 | DISCONNECTED | TEMPSENS);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)255);
      break;
    case 3:
      Bluetooth.write(PORT3 | DISCONNECTED | TEMPSENS);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)255);
      break;
    case 4:
      Bluetooth.write(PORT4 | DISCONNECTED | TEMPSENS);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)0);
      Bluetooth.write((byte)255);
      break;
  }
}
