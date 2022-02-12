/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive data with the MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/

#include <MKRWAN.h>

LoRaModem modem;

// Uncomment if using the Murata chip as a module
// LoRaModem modem(Serial1);

#include "arduino_secrets.h"
#define BAND US915
#define OPERATING_CLASS CLASS_C
#define LoRa_BAUD 115200
#define FPORT 2
#define DATA_RATE 0
#define ADR false
#define DUTYCYCLE false
#define RX2DR 0

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
int join_retry_count = 3;         //tx retries count.
bool loraFlag = false;        // radio module choosing flag. RF 1 is LoRa, RF 0 is bluetooth.
String message = "Ping\n";
bool loraSendFlag = false;

void setup() {
  // put your setup code here, to run once:
  lora_setup();
}

void lora_setup() {
 Serial.begin(baud);
  while (!Serial);
  // change this to your regional band (eg. US915, AS923, ...)
  if (!modem.begin(BAND)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  bool adr_setting = modem.setADR(adr);
  if (!adr_setting){
    Serial.print("Failed to disable automatic data rate.\n");
  }
  else Serial.print("Successfully disabled ADR!\n");
  
  bool drsetting = modem.dataRate(data_rate);
  if (!drsetting){
    Serial.print("Failed to set data rate\n");
  }
  else {
  Serial.print("Set data rate to ");
  Serial.print(data_rate); 
  Serial.print(" successfully\n");
  }
  bool class_setting = modem.configureClass(OPERATING_CLASS);
  if (!class_setting){
    Serial.print("Failed to set data rate\n");
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
  Serial.print("Set the class to: ");
  Serial.print(operating_class);
  Serial.print(" successfully\n");
  }

  bool port_setting = modem.setPort(port);
  if (!port_setting){
    Serial.print("Failed to set port.\n");
  }
  else{
  Serial.print("Set port to ");
  Serial.print(port); 
  Serial.print(" successfully\n");
  }

  bool dutycycle_setting = modem.dutyCycle(duty_cycle);
  if (!dutycycle_setting){
    Serial.print("Failed to disable dutycycle.\n");
  }
  else {
  Serial.print("Set dutycycle to ");
  Serial.print(duty_cycle); 
  Serial.print(" successfully\n");
  }


  bool rx2Dr_setting = modem.setRX2DR(Rx2DR);
  if (!rx2Dr_setting){
    Serial.print("Failed to set Rx2 Datarate.\n");
  }
  else{
  Serial.print("Set Rx2 DR to ");
  Serial.print(Rx2DR); 
  Serial.print(" successfully\n");
  }


  // Set poll interval to 60 secs.
  modem.minPollInterval(60);
  // NOTE: independent of this setting, the modem will
  // not allow sending more than one message every 2 minutes,
  // this is enforced by firmware and can not be changed.
} 

void loop() {
  if (loraFlag){
    lora_join();
//    Serial.println();
//    Serial.println("Enter a message to send to network");
    // Serial.println("(make sure that end-of-line 'NL' is enabled)");

    while (!Serial.available());
    String msg = Serial.readStringUntil('\n');
  
//    Serial.println();
//    if (msg == "exit"){
//      Serial.println("Exiting Application.");
//      }
//    Serial.print("Sending: " + msg + " - ");
//    for (unsigned int i = 0; i < msg.length(); i++) {
//      Serial.print(msg[i] >> 4, HEX);
//      Serial.print(msg[i] & 0xF, HEX);
//      Serial.print(" ");
//    }
//    Serial.println();
    
    int err;
    modem.beginPacket();
    modem.print(msg);
    err = modem.endPacket(true);
    if (err > 0) {
      Serial.println("Message sent correctly!");
      loraSendFlag = true;
    } else {
      Serial.println("Error sending message :(");
      Serial.println("(you may send a limited amount of messages per minute, depending on the signal strength");
      Serial.println("it may vary from 1 message every couple of seconds to 1 message every minute)");
    }
    delay(1000);

    if (loraSendFlag){
    char rcv[64];
    int i = 0;
    while (modem.available()) {
      rcv[i++] = (char)modem.read();

    }
    Serial.print("Received: ");
    for (unsigned int j = 0; j < i; j++) {
      Serial.print(rcv[j] >> 4, HEX);
      Serial.print(rcv[j] & 0xF, HEX);
      Serial.print(" ");
      Serial.println();
    }
  }
  }
  else{
    
    Serial.write(message);
    
    while (!Serial.available());
    String msg = Serial.readStringUntil('\n');

   switch (msg){
    case "loraFlag":
      loraFlag = true;
      break;
   
   default:
    break;
}
}
}

 void lora_join(void){
  Serial.println("Attempting to join the network.");
  int connected = modem.joinOTAA(appEui, appKey);
 
  while(!connected){
    if (join_retry_count != 0){
      join_retry_count--;
      connected = modem.joinOTAA(appEui, appKey);
      Serial.print("Retry number:");
      Serial.println(join_retry_count);
    }
    else{
      loraFlag = false;
      Serial.print("Changing Radio Module to BLE");
      join_retry_count = 7;
      break;
    }
    if (connected){
      Serial.println("Join Success!");
    }
 }
 }
