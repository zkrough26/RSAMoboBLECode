#include <ArduinoBLE_P.h>
#include <SparkFun_SinglePairEthernet.h>

#define MAGICNUM          0XCF        // should be byte 0 in new message

// commands
#define NO_COMMAND          0         // No action Pending
#define REQ_NEW_MESSAGE     1         // central : new message req
#define CANCEL_MESSAGE      2         // Peripheral / Central : cancel and disgard message
#define NEW_BLOCK_AVAILABLE 3         // peripheral: to indicate new block is ready
#define RECEIVED_OK         4         // Peripheral / Central : acknowledge on good receive latest command
#define REQ_NEW_BLOCK       5         // Central : request new block  
#define RCD_CMPLT_MSG       6         // !!central : complete message received
#define REQ_BLOCKSIZE      13         // central: get block size to use (= MTU)
#define RSP_BLOCKSIZE      14         // peripheral : send block size to use (= MTU)
// errors
#define TIMEOUT_BLOCK       7         // !!central : timeout, req to resend
#define INVALID_BLOCK       8         // central : invalid blocknumber
#define CRC_BLOCK           9         // central : CRC error, req to resend
#define REQ_INVALID        10         // Peripheral / Central : Did not request out of sequence
#define MAGIC_INVALID      11         // Central : Invalid first block
#define ERR_INTERNAL       12         // Central : internal error

///////////////////////////////////////////////////////
// Program variables
///////////////////////////////////////////////////////
uint8_t BlockCounter = 0;             // keep track of current block to send
int16_t TotalMessageLength;           // keep track of total message bytes left to send
uint16_t MessageCounter = 0;          // Iterator to keep track of data being copied into blocks
uint8_t *BlockContent;                // Byte array to store data of individual blocks of agreed MTU size
uint16_t BlockContentLength;          // Length of data in each block
uint8_t PendingCommand;               // expect RECEIVED_OK from central on this command
uint8_t CentralCmd;                   // commands/feedback received from central.
uint8_t CurrentMTUSize = 0;           // This will hold the agreed MTU size

///////////////////////////////////////////////////////
// BLE defines
///////////////////////////////////////////////////////
const char BLE_PERIPHERAL_NAME[] = "T42-DAD";  // Name that will be broadcast

#define SERVICE "9e400001-b5a3-f393-e0a9-e14e24dcca9e"

// create characteristic and allow remote device to read and write
#define CHARACTERISTIC_R_UUID "9e400002-b5a3-f393-e0a9-e14e24dcca9e" // receive feedback
#define CHARACTERISTIC_W_UUID "9e400003-b5a3-f393-e0a9-e14e24dcca9e" // Send data characteristic
#define CHARACTERISTIC_N_UUID "9e400004-b5a3-f393-e0a9-e14e24dcca9e" // Notify characteristic

///////////////////////////////////////////////////////
// BLE variables
///////////////////////////////////////////////////////

// create the service
BLEService Service(SERVICE);

// create Write Characteristics
// you need to set the size to the maximum. if your MTU is larger than 
// the characteristic size, it will not work.
BLECharacteristic W_Characteristic(CHARACTERISTIC_W_UUID, BLERead | BLEWrite, 512);

// create feedback Characteristic
BLECharacteristic R_Characteristic(CHARACTERISTIC_R_UUID, BLERead | BLEWrite, 5);

// create notify
BLECharacteristic N_Characteristic(CHARACTERISTIC_N_UUID, BLERead | BLENotify, 5);

///////////////////////////////////////////////////////
// SPE Defines
///////////////////////////////////////////////////////

SinglePairEthernet port1;
#define RST CS
#define port1CS G0
#define port1INT G1

///////////////////////////////////////////////////////
// SPE Variables
///////////////////////////////////////////////////////

int port1MessageSize;
byte port1RecBuffer[1000];
byte port1MAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};
byte port1RecMAC[6];

///////////////////////////////////////////////////////
// Packet Formatting Variables
///////////////////////////////////////////////////////

#define PORT1 0
#define PORT2 32
#define PORT3 64
#define PORT4 96

#define DISCONNECTED   0
#define CONNWITHDATA   8
#define CONNWITHNODATA 16
#define MESSAGE        24

#define TEMPSENS  0
#define HUMSENS   1
#define VIBSENS   2
#define SOUNDSENS 3
#define LOWBATT   4
#define OTHERERR  5
#define COLLSTOP  6
#define COLLSTART 7

byte port1Formatted[1275];
byte port2Formatted[1275];
byte port3Formatted[1275];
byte port4Formatted[1275];

byte port1Header = 0;
byte port2Header = 0;
byte port3Header = 0;
byte port4Header = 0;

int port1FormattedSize, port2FormattedSize, port3FormattedSize, port4FormattedSize;

void setup() {
   delay(1000);  // Give everything time to power up before starting

  ///////////////////////////////////////////////////
  // BLE SETUP START
  ///////////////////////////////////////////////////
  
  // begin initialization
  if (!BLE.begin()) {
    digitalWrite(LED_BUILTIN, HIGH);
    while (1);
  }

  // set the local name peripheral advertises
  BLE.setLocalName(BLE_PERIPHERAL_NAME);

  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(Service);

  // add the characteristics to the service
  Service.addCharacteristic(R_Characteristic); // characteristic that provides feedback from central
  Service.addCharacteristic(W_Characteristic); // characteristic to send data
  Service.addCharacteristic(N_Characteristic); // characteristic to notify central with commands

  // add the service
  BLE.addService(Service);

  // set connection handlers
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // start advertising
  BLE.advertise();

  // called when read is performed by remote
  W_Characteristic.setEventHandler(BLERead, (BLECharacteristicEventHandler) ReadCallBack);

  // called when something written by remote (and thus we read)
  R_Characteristic.setEventHandler(BLEWritten, (BLECharacteristicEventHandler) WriteCallBack);

  ///////////////////////////////////////////////////
  // BLE SETUP END
  ///////////////////////////////////////////////////

  ///////////////////////////////////////////////////
  // SPE SETUP
  ///////////////////////////////////////////////////

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

  ///////////////////////////////////////////////////
  // SPE SETUP END
  ///////////////////////////////////////////////////
}

void loop() {
  static uint8_t LoopCnt = 0;
  
  // Wait for Connection
  while (! BLE.connected()){
    BLE.poll();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);  // Needed for BLE 
  }

  // Can be updated after connection by the client / central
  if (CurrentMTUSize != (uint8_t) BLE.readMTU())
  {
    CurrentMTUSize = (uint8_t) BLE.readMTU();
  }

  if (CentralCmd != NO_COMMAND) 
  {
    if (LoopCnt++ > 3) {
      HandleCentralReq();
      LoopCnt = 0;
    }
  }

  if (port1.getLinkStatus())
  {
    if (port1.getRxAvailable())
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      port1MessageSize = port1.getRxData(port1RecBuffer, 1000, port1RecMAC);
    }
  }
  
  delay(50);  // Needed for BLE 
  BLE.poll();
}

/**
 * called when reading is being done by remote. 
 */
void ReadCallBack(BLEDevice *central, BLECharacteristic c) {
    // No action needed, called when central is reading data sent by us
}

/**
 * called when a command has been written by the central. so we read
 * 
 * format 
 * Byte[0] = MAGICNUM
 * Byte[1] = command / feedback
 */
void WriteCallBack(BLEDevice *central, BLECharacteristic c) {

  int lenc = c.valueLength();
  uint8_t *valuec=(uint8_t*) c.value();

  // check for correct packet start
  if (valuec[0] != MAGICNUM) {
    return;
  }

  // for now we only use ONE command
  CentralCmd = valuec[1];

  switch (CentralCmd){
    case RECEIVED_OK:
      HandlePendingCmd();     // check which command / error we did sent
      break;

    case TIMEOUT_BLOCK:       // these are received commands/errors from Central
    case CRC_BLOCK:
    case CANCEL_MESSAGE:   
    case INVALID_BLOCK:
    case REQ_NEW_MESSAGE:
    case MAGIC_INVALID:
    case ERR_INTERNAL:
    case REQ_NEW_BLOCK: 
    case REQ_BLOCKSIZE:
    case RCD_CMPLT_MSG:
      SendCommand(RECEIVED_OK); // let central know we have received it.
      break;

    default:  // unknown
      SendCommand(REQ_INVALID);
      break;      
  }
}

/**
 * act on received command/errors from central
 */
void HandleCentralReq()
{  
 
  if (CentralCmd == REQ_NEW_BLOCK) {
    CreateSendBlock();
  }
  
  else if (CentralCmd == CANCEL_MESSAGE){
    TotalMessageLength = -1;
  }
 
  else if (CentralCmd == REQ_NEW_MESSAGE){
    TotalMessageLength = 0;
    CreateSendBlock();
  }

  else if (CentralCmd == REQ_BLOCKSIZE){
    SendCommand(RSP_BLOCKSIZE);
  }

  else if (CentralCmd == RCD_CMPLT_MSG){
    TotalMessageLength = 0;
  }

  CentralCmd = NO_COMMAND;
}


// create and send a block
void CreateSendBlock(){
  uint8_t ret = CreateDataToSend();
  
  // if block of incomplete message has been sent
  if (ret == 0) {
  }
  else if (ret == 1) {
    SendCommand(REQ_INVALID);
  }
  else if (ret == 2) {
  }
}

/*
 * Protocol :
 * 
 * peripheral                   central
 * =============================================
 * Wait for request
 *                            send request for message
 * Send Received_OK             
 *                            Wait for notification
 * prepare block
 * add CRC
 * write block
 * write notification
 *                            receive block
 *                            check block Number and CRC
 *                            if not correct, request resend block#
 * resend block
 *                            else extract & store relevant data
 *                            write peripheral all Received_OK 
 *                            request next block
 * Send Received_OK             
 *                            Wait for notification
 * prepare block
 * add CRC
 * write block
 * write notification
 */

void ConvertToPacketStruct()
{
  port1Header = 0;
  memset(port1Formatted, 0, sizeof(port1Formatted));
  if(port1.getLinkStatus())
  {
    port1Header = port1Header | PORT1;
    if ((port1MessageSize > 0) && (port1MessageSize <= 512))
    {
      port1Header = port1Header | CONNWITHDATA;
    }
    else
    {
      port1Header = port1Header | CONNWITHNODATA;
    }
    switch (port1RecMAC[5])
    {
      case 0:
        port1Header = port1Header | TEMPSENS;
        break;
      case 1:
        port1Header = port1Header | HUMSENS;
        break;
      case 2:
        port1Header = port1Header | VIBSENS;
        break;
      case 3:
        port1Header = port1Header | SOUNDSENS;
        break;
      default:
        port1Header = port1Header | OTHERERR; 
    }
    if ((port1RecMAC[5] == 2) || (port1RecMAC[5] == 3))
    {
      for (int i = 0; i < (port1MessageSize / 2) - 1; i++)
      {
        port1Formatted[i * 5] = port1Header;
        port1Formatted[(i * 5) + 1] = i;
        port1Formatted[(i * 5) + 2] = port1RecBuffer[i * 2];
        port1Formatted[(i * 5) + 3] = port1RecBuffer[(i * 2) + 1];
        port1Formatted[(i * 5) + 4] = 255;
      }
      port1FormattedSize = sizeof(port1Formatted);
    }
  }
  else
  {
    port1Header = port1Header | PORT1;
    port1Header = port1Header | DISCONNECTED;
    port1Formatted[0] = port1Header;
    port1Formatted[4] = 255;
    port1FormattedSize = 5;
  }
}

uint8_t CreateDataToSend(){

  int16_t i = 0;

  // complete message had been sent already
  if (TotalMessageLength < 0)  return(1);

  if (! BlockContent) {
    
    // allocate memory for block
    BlockContent = (uint8_t *) malloc(CurrentMTUSize);
  
    if (! BlockContent) {
      while(1);
    }
  }
  
  // if start of new message
  if (TotalMessageLength == 0) {

    ConvertToPacketStruct();

    BlockContent[i++] = MAGICNUM;                       // set magicnum

    TotalMessageLength = port1FormattedSize;

    BlockContent[i++] = TotalMessageLength >> 8 & 0xff;  // set MSB
    BlockContent[i++] = TotalMessageLength & 0xff;       // set LSB

    BlockContent[i++] = CurrentMTUSize & 0xff;           // indicate max size 

    BlockCounter = 0;

    MessageCounter=0;            // offset in source
  }

  // add content for any block 
  BlockContent[i++] = BlockCounter++;
  
  // remember position to add block datalength
  uint16_t LengthData = i++;   

  // block datalength is 16 bits
  i++;
  
  // add to content up to CRC (hence -2) OR Total message has been added
  for (; i < CurrentMTUSize - 2 && TotalMessageLength-- > 0 ;i++) {

   // copy content to block
   BlockContent[i] = port1Formatted[MessageCounter++];
  }

  // add block length (excluding 2 bytes CRC)
  BlockContent[LengthData++] = i >> 8 & 0xff; // set MSB
  BlockContent[LengthData] = i & 0xff;        // set LSB

  // calculate CRC over the curent block
  uint16_t BlockCrc = calc_crc(BlockContent,i);
 
   // add CRC
  BlockContent[i++] = BlockCrc >> 8 & 0xff;  // CRC MSB
  BlockContent[i++] = BlockCrc & 0xff;       // CRC LSB

  // save for sending
  BlockContentLength = i;
  
  SendBlock();

  // complete message has been sent now
  if (TotalMessageLength < 0)  return(2);
  else return(0);
}

/**
 * Write a block to the characteristic and let the central
 * know it is available on Notify
 */
void SendBlock()
{
  if (! BLE.connected()){
    return;
  }

  // if valid Block
  if (BlockContentLength) {
    W_Characteristic.writeValue(BlockContent,BlockContentLength);

    // send notification new data is available
    SendCommand(NEW_BLOCK_AVAILABLE);
  }
}

/**
 * send command on notify
 * Byte[0] = MAGICNUM
 * Byte[1] = command
 * Byte[2] = optional parameter
 */
void SendCommand(uint8_t cmd){
  uint8_t s[3];
  s[0] = MAGICNUM;
  s[1] = cmd;
  
  if (cmd == RSP_BLOCKSIZE) {
    s[2] = CurrentMTUSize;
  }
  
  N_Characteristic.writeValue(s,3);

  // set pending command
  PendingCommand = cmd;
}

/**
 * we got a receive OK on previous command send by us
 */
void HandlePendingCmd() {
  switch (PendingCommand){
    case CANCEL_MESSAGE:
          TotalMessageLength = 0;
    case NEW_BLOCK_AVAILABLE:
    case REQ_INVALID:
    case RSP_BLOCKSIZE:
      break;
    default:
      break;
  }
  PendingCommand = NO_COMMAND;
}

/**
 * called when central connects
 */
void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  TotalMessageLength = 0;
  PendingCommand = NO_COMMAND;
}

/**
 * called when central disconnects
 */
void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler

  // release memory if allocated
  if (BlockContent) free(BlockContent);
  BlockContent = NULL;

  // start advertising
  BLE.advertise();
}
