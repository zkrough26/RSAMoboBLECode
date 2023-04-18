void ServicePorts()
{
  if (port1Active && !port1DataReady) {
    if (!port1.begin(port1MAC, LED_BUILTIN, port1INT, RST, port1CS)) {
      SendPortError(1);
    }
    port1.setRxCallback(port1RxCallback);
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
      SendDisconPacket(1);
    }
  }
  if (port2Active && !port2DataReady) {
    if (!port2.begin(port2MAC, LED_BUILTIN, port2INT, RST, port2CS)) {
      SendPortError(2);
    }
    port2.setRxCallback(port2RxCallback);
    if (port2.getLinkStatus()) {
      if (!port2LinkStatus){
        port2FirstPacket = true;
      }
      port2LinkStatus = true;
      delay(1000);
    }
    else {
      port2LinkStatus = false;
      port1DataReady = false;
      SendDisconPacket(2);
    }
  }
  if (port3Active && !port3DataReady) {
    if (!port3.begin(port3MAC, LED_BUILTIN, port3INT, RST, port3CS)) {
      SendPortError(3);
    }
    port3.setRxCallback(port3RxCallback);
    if (port3.getLinkStatus()) {
      if (!port3LinkStatus){
        port3FirstPacket = true;
      }
      port3LinkStatus = true;
      delay(1000);
    }
    else {
      port3LinkStatus = false;
      port1DataReady = false;
      SendDisconPacket(3);
    }
  }
  if (port4Active && !port4DataReady) {
    if (!port4.begin(port4MAC, LED_BUILTIN, port4INT, RST, port4CS)) {
      SendPortError(4);
    }
    port4.setRxCallback(port4RxCallback);
    if (port4.getLinkStatus()) {
      if (!port4LinkStatus){
        port4FirstPacket = true;
      }
      port4LinkStatus = true;
      port1DataReady = false;
      delay(1000);
    }
    else {
      port4LinkStatus = false;
      SendDisconPacket(4);
    }
  }
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
      break;
    case 2:
      Bluetooth.write(PORT2 | DISCONNECTED | TEMPSENS);
      break;
    case 3:
      Bluetooth.write(PORT3 | DISCONNECTED | TEMPSENS);
      break;
    case 4:
      Bluetooth.write(PORT4 | DISCONNECTED | TEMPSENS);
      break;
  }
  Bluetooth.write((byte)0);
  Bluetooth.write((byte)0);
  Bluetooth.write((byte)0);
  Bluetooth.write((byte)255);
}
