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

// Uncomment to see progress / debug information
//#define BLE_SHOW_DATA 1

///////////////////////////////////////////////////////
// Program variables
///////////////////////////////////////////////////////
uint8_t BlockCounter = 0;             // keep track of current block to send
int16_t TotalMessageLength;           // keep track of total message bytes left to send
uint8_t *BlockContent;
uint16_t BlockContentLength;
uint8_t PendingCommand;               // expect RECEIVED_OK from central on this command
uint8_t CentralCmd;                   // commands/feedback received from central.
uint8_t CurrentMTUSize = 0;           // This will hold the agreed MTU size

// Base to create sliding complete message
int MessageCounter = 0;

char     input[10];                 // keyboard input buffer
int      inpcnt = 0;                // keyboard input buffer length

///////////////////////////////////////////////////////
// BLE defines
///////////////////////////////////////////////////////
const char BLE_PERIPHERAL_NAME[] = "Mobo";

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
// SPE Callback
///////////////////////////////////////////////////////

static void rxCallback(byte * data, int dataLen, byte * senderMac)
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    port1MessageSize = dataLen;
    for (int i = 0; i < sizeof(senderMac); i++)
    {
      port1RecMAC[i] = senderMac[i];
    }
    for (int i = 0; i < dataLen; i++)
    {
      port1RecBuffer[i] = data[i];
    }
}

void setup() {
   Serial.begin(115200);
   delay(1000);
   Serial.print("\nExample14 peripheral RW_notify_MTU. Compiled on: ");
   Serial.println(__TIME__);

  ///////////////////////////////////////////////////
  // BLE SETUP START
  ///////////////////////////////////////////////////

  //BLE.debug(Serial);         // enable display HCI messages
  
  // begin initialization
  if (!BLE.begin()) {
    Serial.println(F("starting BLE failed! freeze\r"));
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
  port1.setRxCallback(rxCallback);
  
  Serial.print(F("Peripheral ready to go with the name '"));
  Serial.print(BLE_PERIPHERAL_NAME);
  Serial.println("'");
}

void loop() {
  static uint8_t LoopCnt =0;
  
  // wait for connection
  while (! BLE.connected()){
    BLE.poll();
    delay(500);     // don't hammer
  }

  // this can be updated after connect by the client / central
  if (CurrentMTUSize != (uint8_t) BLE.readMTU())
  {
    CurrentMTUSize = (uint8_t) BLE.readMTU();
    Serial.print("Using MTU / blocksize : ");
    Serial.println(CurrentMTUSize);
  }
  
  // handle any keyboard input
  if (Serial.available()) {
    while (Serial.available()) handle_input(Serial.read());
  }

  if (CentralCmd != NO_COMMAND) {

    // wait a couple of BLE.poll() before acting (seems to be needed ??)
    if (LoopCnt++ > 3) {
      HandleCentralReq();
      LoopCnt = 0;
    }
  }
  
  delay(50);        // needed for BLE and don't hammer   
  BLE.poll();
}

/**
 * called when reading is being done by remote. 
 */
void ReadCallBack(BLEDevice *central, BLECharacteristic c) {
#ifdef BLE_SHOW_DATA 
    Serial.println(F("Read call_Back. Central is reading"));
#endif
    // no action only info
}

/**
 * called when a command has been written by the central. so we read
 * 
 * format 
 * Byte[0] = MAGICNUM
 * Byte[1] = command / feedback
 */
void WriteCallBack(BLEDevice *central, BLECharacteristic c) {
#ifdef BLE_SHOW_DATA   
  Serial.println(F("Central has written something."));
#endif

  int lenc = c.valueLength();
  uint8_t *valuec=(uint8_t*) c.value();

  // check for correct packet start
  if (valuec[0] != MAGICNUM) {
    Serial.print(F("Invalid magic number : 0x"));
    Serial.println(valuec[0],HEX);
    return;
  }
  
#ifdef BLE_SHOW_DATA 
  Serial.print("Received command (HEX) 0x");
  Serial.println(valuec[1]);
#endif

  // for now we only use ONE command
  CentralCmd = valuec[1];

  switch (CentralCmd){
    case RECEIVED_OK:
#ifdef BLE_SHOW_DATA 
      Serial.println("RECEIVED_OK");
#endif
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
    Serial.println(F("\nCancel current message."));
    TotalMessageLength = -1;
  }
 
  else if (CentralCmd == REQ_NEW_MESSAGE){
    Serial.println(F("\nRequest new message."));
    TotalMessageLength = 0;
    CreateSendBlock();
  }

  else if (CentralCmd == REQ_BLOCKSIZE){
    Serial.println(F("\nRequest blocksize\n"));
    SendCommand(RSP_BLOCKSIZE);
  }

  else if (CentralCmd == RCD_CMPLT_MSG){
    Serial.println(F("\nCentral confirms receipt of total message\n"));
    TotalMessageLength = 0;
  }

  CentralCmd = NO_COMMAND;
}

/**
 * create and send a block
 * 
 */
void CreateSendBlock(){
  uint8_t ret = CreateDataToSend();
  
  // if block of incomplete message has been sent
  if (ret == 0) {
    Serial.print(F("Sent Block "));
    Serial.println(BlockCounter);
  }
  else if (ret == 1) {
  
#ifdef BLE_SHOW_DATA 
    Serial.println(F("Complete message had been sent before"));
    Serial.println(F("No action now."));
#endif
    SendCommand(REQ_INVALID);
  }
  else if (ret == 2) {
    Serial.print(F("\nComplete message has now been sent in "));
    Serial.print(BlockCounter);
    Serial.println(F(" blocks."));
  }
}

/*
 * send a complete message of X bytes broken down in block of max MTU size 
 * 
 * Message format
 * 
 * Block 0
 * Byte 0  = MagicNum
 * Byte 1  = MSB total message length (uint16_t)
 * Byte 2  = LSB total message length
 * Byte 3 =  block number (starting 0) (uint8_t)
 * Byte 4 =  MSB block data length  (excluding 2 bytes CRC) (uint16_t)
 * Byte 5 =  LSB block data length
 * byte 5 - N  data (N is MAX  MAXBLOCKSIZE -3)
 * Byte N +1 = MSB CRC on ALL previous bytes in block (uint16_t)
 * Byte N +2 = LSB CRC
 * 
 * 
 * blocks following
 * Byte 1  = current block number X (uint8_t)
 * Byte 2 =  MSB block data length  (excluding 2 bytes CRC) (uint16_t)
 * Byte 3 =  LSB block data length
 * Byte 4 - N  data (N is MAX  MAXBLOCKSIZE -3)
 * byte N +1 = MSB CRC on ALL previous bytes in block (uint16_t)
 * Byte N +2 = LSB CRC
 * 
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
 * 
 * @return 
 *  0 ; block created and send
 *  1 ; complete message had been sent before
 *  2 ; last block of the message has been sent
 * 
 */

uint8_t CreateDataToSend(){
  int16_t i = 0;

  // complete message had been sent already
  if (TotalMessageLength < 0)  return(1);

  if (! BlockContent) {
    
    // allocate memory for block
    BlockContent = (uint8_t *) malloc(CurrentMTUSize);
  
    if (! BlockContent) {
      Serial.println(F("Can Not allocate memory. freeze\n"));
      while(1);
    }
  }
  
  // if start of new message
  if (TotalMessageLength == 0) {

    BlockContent[i++] = MAGICNUM;                       // set magicnum

    TotalMessageLength = port1MessageSize;

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
   BlockContent[i] = port1RecBuffer[MessageCounter++];
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
  
#ifdef BLE_SHOW_DATA 
  Serial.println("Sending content: ");
  // display Hex
  for (int k = 0; k < i;k++){
    Serial.print(BlockContent[k],HEX);
    Serial.print(" ");
  }
  Serial.print("\nlength: ");
  Serial.println(i);
#endif
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
    Serial.println(F("ERROR : NOT CONNECTED"));
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
          // fall through
    case NEW_BLOCK_AVAILABLE:
    case REQ_INVALID:
    case RSP_BLOCKSIZE:
#ifdef BLE_SHOW_DATA 
      Serial.println("Previous command acknowledged");
#endif
      break;

    default:
      Serial.println("Unexpected Receive_OK. Ignored");
  }

  PendingCommand = NO_COMMAND;
}


/**
 * handle local keyboard input
 */
void handle_input(char c)
{
  if (c == '\r') return;    // skip CR

  if (c != '\n') {          // act on linefeed
    input[inpcnt++] = c;
    if (inpcnt < 9 ) return;
  }

  input[inpcnt] = 0x0;

  // only the first character is used here
  switch (input[0]) {
    
    case '1' :    // start sending new message
      Serial.println(F("Request new message"));

      if (PendingCommand == NO_COMMAND) {
        
        if (TotalMessageLength < 0) {
         TotalMessageLength = 0;
         CreateDataToSend();       
        }
        else {
          Serial.println(F("First send cancel current message"));
        }
      }
      else
        Serial.println(F("Waiting on Received_OK from central"));
      break;
  
    case '2' :    // send command cancel
      Serial.println(F("Cancel current message\n"));
      SendCommand(CANCEL_MESSAGE);
      break;

    case '3':     // disconnect
      Serial.println(F("Disconnect from Central"));
      BLE.disconnect();
      // reset keyboard buffer
      inpcnt = 0;
      return;
      break;
    default :
      Serial.print(F("Ignore unknown request: "));
      Serial.println(input[0]);
      break;
  }
  
  display_menu();

  // reset keyboard buffer
  inpcnt = 0;
}

void display_menu()
{
  Serial.println(F("1.  Start sending new message\r"));
  Serial.println(F("2.  Cancel current message\r"));
  Serial.println(F("3.  Disconnect\r"));
}

/**
 * called when central connects
 */
void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  TotalMessageLength = 0;
  PendingCommand = NO_COMMAND;
}

/**
 * called when central disconnects
 */
void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());

  // release memory if allocated
  if (BlockContent) free(BlockContent);
  BlockContent = NULL;

  // start advertising
  BLE.advertise();
}
