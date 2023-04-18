///////////////////////////
//  SPE Port1 Var/Def
///////////////////////////
#define RST CS  // All ports use same RST line
#define port1CS G0
#define port1INT G1
int port1MessageSize;
byte port1RecBuffer[1000];
byte port1MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port1RecMAC[6];
bool port1LinkStatus = false;
bool port1FirstPacket = true;
bool port1SendDiscon = false;
///////////////////////////
//  SPE Port2 Var/Def
///////////////////////////
#define port2CS G2
#define port2INT G3
int port2MessageSize;
byte port2RecBuffer[1000];
byte port2MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port2RecMAC[6];
bool port2LinkStatus = false;
bool port2FirstPacket = true;
bool port2SendDiscon = false;
///////////////////////////
//  SPE Port3 Var/Def
///////////////////////////
#define port3CS G4
#define port3INT G5
int port3MessageSize;
byte port3RecBuffer[1000];
byte port3MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port3RecMAC[6];
bool port3LinkStatus = false;
bool port3FirstPacket = true;
bool port3SendDiscon = false;
///////////////////////////
//  SPE Port2 Var/Def
///////////////////////////
#define port2CS G6
#define port2INT G7
int port4MessageSize;
byte port4RecBuffer[1000];
byte port4MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port4RecMAC[6];
bool port4LinkStatus = false;
bool port4FirstPacket = true;
bool port4SendDiscon = false;
////////////////////////////
//  END All SPE Port Var/Def
////////////////////////////