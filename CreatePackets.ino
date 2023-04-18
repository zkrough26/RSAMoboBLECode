void ConvertToPacketStruct(int port)
{
    ////////////////////////////////
    // Switch to clear old data
    ////////////////////////////////
    switch(port) {
        case 1:
            port1Header = 0;
            port1FormattedSize = 0;
            memset(port1Formatted, 0, sizeof(port1Formmated));
            break;
        case 2:
            port2Header = 0;
            port2FormattedSize = 0;
            memset(port2Formatted, 0, sizeof(port2Formmated));
            break;
        case 3:
            port3Header = 0;
            port3FormattedSize = 0;
            memset(port3Formatted, 0, sizeof(port3Formmated));
            break;
        case 4:
            port4Header = 0;
            port4FormattedSize = 0;
            memset(port4Formatted, 0, sizeof(port4Formmated));
            break;
    }
    /////////////////////////////////
    // END Clear old data switch
    /////////////////////////////////

    /////////////////////////////////
    // Switches to create metadata
    /////////////////////////////////
    switch(port) {
        case 1:
            port1Header = port1Header | PORT1;
            if ((port1MessageSize > 0) && (port1MessageSize <= 512)) {
                port1Header = port1Header | CONNWITHDATA;
            }
            else {
                port1Header = port1Header | CONNWITHNODATA;
            }
            switch(port1RecMAC[5]) {
                case 1:
                    port1Formatted[0] = port1Header | HUMSENS;
                    port1Formatted[5] = port1Header | TEMPSENS;
                    port1Formatted[4] = 255;
                    port1Formatted[9] = 255;
                    break;
                case 2:
                    port1Header = port1Header | VIBSENS;
                    break;
                case 3:
                    port1Header = port1Header | SOUNDSENS;
                    break;
            }
            break;
        case 2:
            port2Header = port2Header | port2;
            if ((port2MessageSize > 0) && (port2MessageSize <= 512)) {
                port2Header = port2Header | CONNWITHDATA;
            }
            else {
                port2Header = port2Header | CONNWITHNODATA;
            }
            switch(port2RecMAC[5]) {
                case 1:
                    port2Formatted[0] = port2Header | HUMSENS;
                    port2Formatted[5] = port2Header | TEMPSENS;
                    port2Formatted[4] = 255;
                    port2Formatted[9] = 255;
                    break;
                case 2:
                    port2Header = port2Header | VIBSENS;
                    break;
                case 3:
                    port2Header = port2Header | SOUNDSENS;
                    break;
            }
            break;
        case 3:
            port3Header = port3Header | port3;
            if ((port3MessageSize > 0) && (port3MessageSize <= 512)) {
                port3Header = port3Header | CONNWITHDATA;
            }
            else {
                port3Header = port3Header | CONNWITHNODATA;
            }
            switch(port3RecMAC[5]) {
                case 1:
                    port3Formatted[0] = port3Header | HUMSENS;
                    port3Formatted[5] = port3Header | TEMPSENS;
                    port3Formatted[4] = 255;
                    port3Formatted[9] = 255;
                    break;
                case 2:
                    port3Header = port3Header | VIBSENS;
                    break;
                case 3:
                    port3Header = port3Header | SOUNDSENS;
                    break;
            }
            break;
        case 4:
            port4Header = port4Header | port4;
            if ((port4MessageSize > 0) && (port4MessageSize <= 512)) {
                port4Header = port4Header | CONNWITHDATA;
            }
            else {
                port4Header = port4Header | CONNWITHNODATA;
            }
            switch(port4RecMAC[5]) {
                case 1:
                    port4Formatted[0] = port4Header | HUMSENS;
                    port4Formatted[5] = port4Header | TEMPSENS;
                    port4Formatted[4] = 255;
                    port4Formatted[9] = 255;
                    break;
                case 2:
                    port4Header = port4Header | VIBSENS;
                    break;
                case 3:
                    port4Header = port4Header | SOUNDSENS;
                    break;
            }
            break;
    }
    //////////////////////////////////
    // END Switches to create metadata
    //////////////////////////////////

    //////////////////////////////////
    // Switch to add data to packets
    //////////////////////////////////
    swtich(port1) {
        case 1:
            if ((port1RecMAC[5] == 2) || (port1RecMAC[5] == 3)) {
            for (int i = 0; i < (port1MessageSize / 2) - 1; i++) {
                port1Formatted[i * 5] = port1Header;
                port1Formatted[(i * 5) + 1] = i;
                port1Formatted[(i * 5) + 2] = port1RecBuffer[i * 2];
                port1Formatted[(i * 5) + 3] = port1RecBuffer[(i * 2) + 1];
                port1Formatted[(i * 5) + 4] = 255;
            }
            port1FormattedSize = sizeof(port1Formatted);
            }
            if (port1RecMAC[5] == 1) {
            port1Formatted[2] = port1RecBuffer[0];
            port1Formatted[7] = port1RecBuffer[1];
            port1FormattedSize = 10;
            }
            break;
        case 2:
            if ((port2RecMAC[5] == 2) || (port2RecMAC[5] == 3)) {
            for (int i = 0; i < (port2MessageSize / 2) - 1; i++) {
                port2Formatted[i * 5] = port2Header;
                port2Formatted[(i * 5) + 1] = i;
                port2Formatted[(i * 5) + 2] = port2RecBuffer[i * 2];
                port2Formatted[(i * 5) + 3] = port2RecBuffer[(i * 2) + 1];
                port2Formatted[(i * 5) + 4] = 255;
            }
            port2FormattedSize = sizeof(port2Formatted);
            }
            if (port2RecMAC[5] == 1) {
            port2Formatted[2] = port2RecBuffer[0];
            port2Formatted[7] = port2RecBuffer[1];
            port2FormattedSize = 10;
            }
            break;
        case 3:
            if ((port3RecMAC[5] == 2) || (port3RecMAC[5] == 3)) {
            for (int i = 0; i < (port3MessageSize / 2) - 1; i++) {
                port3Formatted[i * 5] = port3Header;
                port3Formatted[(i * 5) + 1] = i;
                port3Formatted[(i * 5) + 2] = port3RecBuffer[i * 2];
                port3Formatted[(i * 5) + 3] = port3RecBuffer[(i * 2) + 1];
                port3Formatted[(i * 5) + 4] = 255;
            }
            port3FormattedSize = sizeof(port3Formatted);
            }
            if (port3RecMAC[5] == 1) {
            port3Formatted[2] = port3RecBuffer[0];
            port3Formatted[7] = port3RecBuffer[1];
            port3FormattedSize = 10;
            }
            break;
        case 4:
            if ((port4RecMAC[5] == 2) || (port4RecMAC[5] == 3)) {
            for (int i = 0; i < (port4MessageSize / 2) - 1; i++) {
                port4Formatted[i * 5] = port4Header;
                port4Formatted[(i * 5) + 1] = i;
                port4Formatted[(i * 5) + 2] = port4RecBuffer[i * 2];
                port4Formatted[(i * 5) + 3] = port4RecBuffer[(i * 2) + 1];
                port4Formatted[(i * 5) + 4] = 255;
            }
            port4FormattedSize = sizeof(port4Formatted);
            }
            if (port4RecMAC[5] == 1) {
            port4Formatted[2] = port4RecBuffer[0];
            port4Formatted[7] = port4RecBuffer[1];
            port4FormattedSize = 10;
            }
            break;
    }
    //////////////////////////////////////
    // END Switch to add data to packets
    //////////////////////////////////////
}
