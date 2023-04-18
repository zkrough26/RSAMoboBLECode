void SendPackets()
{
    bool sentOneFFT = false;
    if (port1DataReady){
        if (port1FirstPacket){
            SendInitPacket(1);
            port1FirstPacket = false;
        }
        switch (port1RecMAC[5]){
            case 1:
                for (int i = 0; i < 10; i++){
                    Bluetooth.write(port1Formatted[i]);
                }
                port1DataReady = false;
                break;
            case 2:
                if (!sentOneFFT)
                {
                    for (int i = 0; i < port1FormattedSize; i++){
                        Bluetooth.write(port1Formatted[i]);
                        delay(1);
                    }
                    sentOneFFT = true;
                    port1DataReady = false;
                }
                break;
            case 3:
                if (!sentOneFFT)
                {
                    for (int i = 0; i < port1FormattedSize; i++){
                        Bluetooth.write(port1Formatted[i]);
                        delay(1);
                    }
                    sentOneFFT = true;
                    port1DataReady = false;
                }
                break;
        }
    }
    if (port2DataReady){
        if (port2FirstPacket){
            SendInitPacket(2);
            port2FirstPacket = false;
        }
        switch (port2RecMAC[5]){
            case 1:
                for (int i = 0; i < 10; i++){
                    Bluetooth.write(port2Formatted[i]);
                }
                port2DataReady = false;
                break;
            case 2:
                if (!sentOneFFT)
                {
                    for (int i = 0; i < port2FormattedSize; i++){
                        Bluetooth.write(port2Formatted[i]);
                        delay(1);
                    }
                    sentOneFFT = true;
                    port2DataReady = false;
                }
                break;
            case 3:
                if (!sentOneFFT)
                {
                    for (int i = 0; i < port2FormattedSize; i++){
                        Bluetooth.write(port2Formatted[i]);
                        delay(1);
                    }
                    sentOneFFT = true;
                    port2DataReady = false;
                }
                break;
        }
    }
    if (port3DataReady){
        if (port3FirstPacket){
            SendInitPacket(3);
            port3FirstPacket = false;
        }
        switch (port3RecMAC[5]){
            case 1:
                for (int i = 0; i < 10; i++){
                    Bluetooth.write(port3Formatted[i]);
                }
                port3DataReady = false;
                break;
            case 2:
                if (!sentOneFFT)
                {
                    for (int i = 0; i < port3FormattedSize; i++){
                        Bluetooth.write(port3Formatted[i]);
                        delay(1);
                    }
                    sentOneFFT = true;
                    port3DataReady = false;
                }
                break;
            case 3:
                if (!sentOneFFT)
                {
                    for (int i = 0; i < port3FormattedSize; i++){
                        Bluetooth.write(port3Formatted[i]);
                        delay(1);
                    }
                    sentOneFFT = true;
                    port3DataReady = false;
                }
                break;
        }
    }
    if (port4DataReady){
        if (port4FirstPacket){
            SendInitPacket(4);
            port4FirstPacket = false;
        }
        switch (port4RecMAC[5]){
            case 1:
                for (int i = 0; i < 10; i++){
                    Bluetooth.write(port4Formatted[i]);
                }
                port4DataReady = false;
                break;
            case 2:
                if (!sentOneFFT)
                {
                    for (int i = 0; i < port4FormattedSize; i++){
                        Bluetooth.write(port4Formatted[i]);
                        delay(1);
                    }
                    sentOneFFT = true;
                    port4DataReady = false;
                }
                break;
            case 3:
                if (!sentOneFFT)
                {
                    for (int i = 0; i < port4FormattedSize; i++){
                        Bluetooth.write(port4Formatted[i]);
                        delay(1);
                    }
                    sentOneFFT = true;
                    port4DataReady = false;
                }
                break;
        }
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