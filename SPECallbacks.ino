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
    ConvertToPacketStruct(1);
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
    ConvertToPacketStruct(2);
    port2DataReady = true;
    interrupts();
  }
}

static void port3RxCallback(byte * data, int dataLen, byte * senderMac)
{
  if (!port3DataReady)
  {
    noInterrupts();
    port3MessageSize = dataLen;
    for(int i = 0; i < 6; i++)
    {
      port3RecMAC[i] = senderMac[i];
    }
    for(int i = 0; i < dataLen; i++)
    {
      port3RecBuffer[i] = data[i];
    }
    ConvertToPacketStruct(3);
    port3DataReady = true;
    interrupts();
  }
}

static void port4RxCallback(byte * data, int dataLen, byte * senderMac)
{
  if(!port4DataReady)
  {
    noInterrupts();
    port4MessageSize = dataLen;
    for(int i = 0; i < 6; i++)
    {
      port4RecMAC[i] = senderMac[i];
    }
    for(int i = 0; i < dataLen; i++)
    {
      port4RecBuffer[i] = data[i];
    }
    ConvertToPacketStruct(4);
    port4DataReady = true;
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