/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <SoftwareSerial.h>
#define RxCount_Val 10
#define TxCount_Val 3
#define DEBUG 0

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery
SoftwareSerial uart(15,17);
int Rxcounter = RxCount_Val;
int Txcounter = TxCount_Val;
bool sendFlag = false;
bool connectFlag = false;
uint8_t buf[64];
// String message;
int count;


void setup()
{
   uart.begin(115200);
   // Serial.begin(115200);

#if CFG_DEBUG
  // Blocking wait for connection when debug mode is enabled via IDE
  while ( !Serial ) yield();
#endif
  
  //Serial.println("Bluefruit52 BLEUART Example");
  //Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behavior, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  Bluefruit.Periph.setConnSupervisionTimeout(500);    //timeout after 5 mins of no data transmission. Hopefully. 

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop()
{
  // Forward data from HW Serial to BLEUART

  if (connectFlag){
   
  if (uart.available())
  {
    // Delay to wait for enough input, since we have a limited transmission buffer
    delay(2);
    // String recv = uart.readStringUntil('\n');
    // recv.getBytes(buf, sizeof(recv));
    // count = sizeof(recv);
//    while (uart.available()>0){
//      char inByte = Serial.read();
//      while (inByte != '\n'){
//        buf[buffpos] = inByte;
//        buffpos++;
//        delay(10);
//      }
//      buf[buffpos] = '\0'; 
//      count = buffpos;
//      transmit();
//      buffpos = 0;
//    }
    count = uart.readBytes(buf, sizeof(buf));
//    for (int i=0; i<sizeof(buf)+1; i++){
//      mcuBuffer[i] = buf[i];
//    }
//    message = uart.readStringUntil('\n');
//    buffSize = message.length() + 1;
//    message.toCharArray(buf, sizeof(message)+1);
    transmit();
    

  // Forward from BLEUART to HW Serial
  }
  if (sendFlag){
    receive();
  }
}
//  if (Serial.available()){
//    Serial.read();
//  }
}
// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));
  connectFlag = true;
  delay(100);
  uart.print("JOIN_SUCCESS\n");

//  Serial.print("Connected to ");
//  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

//  Serial.println();
//  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  //Serial.write("Disconnected, reason = 0x"); 
  // Serial.write("Disconnected");   // let the mcu know you have disconnected
  delay(100);
  uart.print("DISCONNECTED\n");       // let the mcu konw that you will be switching to LoRa
}

void receive(void){
  if (Txcounter > 0){
    if (Rxcounter > 0){
      if ( bleuart.available() )
       {
        uint8_t ch;           // was uint8_t
        ch = (uint8_t)bleuart.read(); //(uint8_t) bleuart.read();
//        if (ch == "1"){
            Rxcounter = RxCount_Val;
            Txcounter = TxCount_Val;
            uart.print("TX_SUCCESS\n");
            count = 0;
            sendFlag = false;
//        }
        // uart.print(ch);
//        else{
//          uart.print("TX_FAIL\n");
//          sendFlag = false;
//          return;
//        }
        //TODO: Check if this is enough for the interface - Vis
      }
      else{
        Rxcounter--;
        if (DEBUG){
        Serial.println("Rx Count Value:" + (String)Rxcounter);
        }
        delay(1000);
      if (Rxcounter == 0 and Txcounter > 0){
        Rxcounter = RxCount_Val;
        transmit();
      }
      
    }
  }
}
//if (Txcounter == 0 and Rxcounter == 0){
//         sendFlag = false;
//         uart.print("TX_FAIL\n");
//      }
}

void transmit(void){
  if (Txcounter > 0){
    // bleuart.write( buf, buffSize );
    bleuart.write(buf, count);
    // bleuart.flush();
    sendFlag = true;
    Txcounter--;
    if (DEBUG){
    Serial.println("Tx Count Value:" + (String)Txcounter);
   }
   if (Txcounter == 0){
           uart.print("TX_FAIL\n");
           sendFlag = false;
        } 
  }
}
