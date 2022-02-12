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
#define TxCount_Val 4
#define DEBUG 0
#define UART 1

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery
SoftwareSerial uart(15,17);
int Rxcounter = RxCount_Val;
int Txcounter = TxCount_Val;
bool advFlag = false;
bool sendFlag = false;
bool connectFlag = false;
uint8_t buf[64];
int count;
int buffPos = 0;


void setup()
{
  if (UART){
    uart.begin(115200);
  }
  if (DEBUG){
    Serial.begin(115200);
  }
  

#if CFG_DEBUG
  // Blocking wait for connection when debug mode is enabled via IDE
  while ( !Serial ) yield();
#endif
 

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
  // Bluefruit.Periph.setConnSupervisionTimeout(500);    //timeout after 5 mins of no data transmission. Hopefully. 

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

//  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
//  Serial.println("Once connected, enter character(s) that you wish to send");
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

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));
  //todo: disconnect if connected to the wrong device. 
//  if(DEBUG){
//    Serial.print("Connected to ");
//    Serial.println(central_name);
//  }
  delay(100);
  Rxcounter = RxCount_Val;
  Txcounter = TxCount_Val;
  if (UART){
    uart.print("JS\n");
  }
  if (DEBUG){
    Serial.print("JS\n");
  }
  connectFlag = true;
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
  Rxcounter = 0;
  Txcounter = 0;
  if (UART){
    uart.print("DC\n");       // let the mcu know that you have disconnected
  }
  if (DEBUG){
    Serial.print("DC\n");       // let the mcu konw that you have disconnected
  }
  connectFlag = false;
  advFlag = true;
  delay(100);

//  Serial.println();
//  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}

void receive(void){
  if (Txcounter > 0){
    if (Rxcounter > 0){
      if ( bleuart.available() )
       {
        char ch;           // was uint8_t
        ch = bleuart.read(); //(uint8_t) bleuart.read();
        if (DEBUG){
          Serial.println(ch);
        }
        if (ch == '1'){
            Rxcounter = RxCount_Val;
            Txcounter = TxCount_Val;
            if (UART){
              uart.print("TS\n");     //Tx Success
            }
            if (DEBUG){
              Serial.print("TS\n");
            }
            count = 0;
            advFlag = true;
            sendFlag = false;
            connectFlag = false;
        }
        else{
          uart.print("TF\n");     //Tx Fail
          sendFlag = false;
          return;
        }
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
}

void transmit(void){
  if (Txcounter > 0){
    if (DEBUG){
      Serial.write(buf, count);
    }
    // delay(2000);
    bleuart.write(buf, count);
    sendFlag = true;
    Txcounter--;
    if (DEBUG){
    Serial.println("Tx Count Value:" + (String)Txcounter);
   }
   if (Txcounter == 0){
          if (UART){
           uart.print("TF\n");
          }
          if (DEBUG){
            Serial.print("TF\n");
          }
           sendFlag = false;
        }
   if (Txcounter < 0){
      Txcounter = 0;
   }
  }
}

void loop()
{
  while (connectFlag){
    // Forward data from HW Serial to BLEUART
    
    //if(Serial.available()){
    while (uart.available() > 0){
      
      // Delay to wait for enough input, since we have a limited transmission buffer
       delay(100);
//       if (DEBUG){
//        count = Serial.readBytes(buf, sizeof(buf));
//      }
      if (UART){
        count = uart.readBytes(buf, sizeof(buf));
      }
      transmit();
    }   
    if (sendFlag){
      receive();
    }
  }
  if (advFlag){
    advFlag = false;
    startAdv();
  }
  delay(100);
}
