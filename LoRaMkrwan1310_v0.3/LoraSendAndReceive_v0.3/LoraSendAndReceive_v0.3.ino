/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive data with the MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/

#include <MKRWAN.h>

//LoRaModem modem;

// Uncomment if using the Murata chip as a module
LoRaModem modem(Serial1);

#include "arduino_secrets.h"
#define BAND US915
#define OPERATING_CLASS CLASS_C
#define LoRa_BAUD 115200
#define FPORT 2
#define DATA_RATE 0
#define ADR false
#define DUTYCYCLE false
#define RX2DR 0
#define JoinRetryCount 3
#define TxRetryCount 5

// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;
String operating_class;
bool adr = ADR;
bool duty_cycle = DUTYCYCLE;
int Rx2DR = RX2DR;
int port = FPORT;
int baud = LoRa_BAUD;
int data_rate = DATA_RATE;
// String band = BAND;
int join_retry_count = JoinRetryCount;         //join retries count.
int tx_retry_count = TxRetryCount;
bool bleFlag = false;
bool loraFlag = false;        // radio module choosing flag. RF 1 is LoRa, RF 0 is bluetooth.
bool lora_join_flag = false;
char message[] = "Ping";
bool loraSuccessFlag = false;
int err;

void setup() {
  // put your setup code here, to run once:
  lora_setup();
}

void lora_setup() {
  Serial.begin(baud);
 Serial1.begin(baud);
  while (!Serial);
  // change this to your regional band (eg. US915, AS923, ...)
if (!modem.begin(BAND)) {
//    Serial.println("Failed to start module");
    while (1) {}
  };
//  Serial.print("Your module version is: ");
//  Serial.println(modem.version());
//  Serial.print("Your device EUI is: ");
//  Serial.println(modem.deviceEUI());

  bool adr_setting = modem.setADR(adr);
  if (!adr_setting){
    return;
    //Serial.print("Failed to disable automatic data rate.\n");
  }
  
  bool drsetting = modem.dataRate(data_rate);
  if (!drsetting){
    //Serial.print("Failed to set data rate\n");
    return;
  }

  bool class_setting = modem.configureClass(OPERATING_CLASS);
  if (!class_setting){
//    Serial.print("Failed to set Class\n");
      return;
  }
  else{
  switch (OPERATING_CLASS){
    case CLASS_A:
    operating_class = "Class A";
    break;
    case CLASS_B:
    operating_class = "Class B";
    break;
    case CLASS_C:
    operating_class = "Class C";
    break;
  }

  bool port_setting = modem.setPort(port);
  if (!port_setting){
//    Serial.print("Failed to set port.\n");
      return;
  }

  bool dutycycle_setting = modem.dutyCycle(duty_cycle);
  if (!dutycycle_setting){
//    Serial.print("Failed to disable dutycycle.\n");
return;
  }

  bool rx2Dr_setting = modem.setRX2DR(Rx2DR);
  if (!rx2Dr_setting){
//    Serial.print("Failed to set Rx2 Datarate.\n");
return;
  }

  
  // Set poll interval to 60 secs.
  modem.minPollInterval(60);
  // NOTE: independent of this setting, the modem will
  // not allow sending more than one message every 2 minutes,
  // this is enforced by firmware and can not be changed. is this a joke?
}
}

void lora_join(void){
  int connected = modem.joinOTAA(appEui, appKey);
 
  while(!connected){
    if (join_retry_count != 0){
      join_retry_count--;
      Serial.print("Retry number:");
      Serial.println(join_retry_count);
      connected = modem.joinOTAA(appEui, appKey);
    }
    else{
      loraFlag = false;
      Serial.print("Changing Radio Module to BLE\n");
      join_retry_count = JoinRetryCount;
      break;
    }
    }
    if (connected){
      lora_join_flag = true;
      join_retry_count = JoinRetryCount;
      Serial.println("Join Success!");
    }
    else{
       Serial.println("Join Fail!\n");
      }
   }


void lora_transmission(void){
  
  while (tx_retry_count != 0){
      lora_send();
      if (tx_retry_count == 0 || loraSuccessFlag){
        lora_join_flag = false;
        loraFlag = false;
        Serial.print("switching to bt\n");
        break;//return?
      }  
    }
      
  if (!modem.available()) {       // receive not available
    return;
  }
  char rcv[64];
  int i = 0;
  while (modem.available()) {     // when receive buffer has data
    rcv[i++] = (char)modem.read();
  }
  delay(500);
}

void lora_send(void){
  modem.beginPacket();
  Serial.println(message);
  modem.print(message);             // transmit messsage
  err = modem.endPacket(true);      // check if message tx was successful
  Serial.println(err);
    if (err > 0){
      loraSuccessFlag = true;
      Serial.print("lora_success\n");
    }
    else{
      tx_retry_count--;
    }
  }

void loop() {
  if (loraFlag){
    lora_join();
    if (lora_join_flag){
    lora_transmission();
    }
  }
  else{
    // if (Serial.available()){
    while (Serial1.available() > 0){
      delay(100);
      String msg = Serial1.readStringUntil('\n');
      Serial.println(msg);
      if (msg == "JS"){
        delay(5000);    //for me to turn on tx characteristic subscription on mobile
        Serial1.print(message);
        }
      if (msg == "TF" or msg == "DC"){
        loraFlag = true;
        // Serial.print("Changing to LoRa\n");
        }

      if (msg == "TS"){
        delay(30000);
        //Do smth, send bt msg once every 30 seconds
        Serial1.print(message);
      }
      }
    }
    delay(100);
  }
