///////////////////////////
// SPE Per Port rxCallback
///////////////////////////
static void port1RxCallback(byte * data, int dataLen, byte * senderMac)
{
  if (!port1DataReady)
  {
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
  }
}

static void port2RxCallback(byte * data, int dataLen, byte * senderMac)
{
  if(!port2DataReady)
  {
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
  }
}

static void port3RxCallback(byte * data, int dataLen, byte * senderMac)
{
  if (!port3DataReady)
  {
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
  }
}

static void port4RxCallback(byte * data, int dataLen, byte * senderMac)
{
  if(!port4DataReady)
  {
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
  }
}
///////////////////////////
//  END SPE rxCallbacks
///////////////////////////
