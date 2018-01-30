// rf69 demo tx rx.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF69 class. RH_RF69 class does not provide for addressing or
// reliability, so you should only use RH_RF69  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf69_server.
// Demonstrates the use of AES encryption, setting the frequency and modem 
// configuration

#include <SPI.h>
#include <RH_RF69.h>

#include "Timer.h"
Timer t;

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 915.0

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
#endif

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission

//int leds[6] = {5,6,9,10,11,12};
int leds[6] = {12,11,10,9,6,5};

void setup() 
{
  Serial.begin(115200);
  //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("Feather RFM69 TX Test!");
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
  pinMode(LED, OUTPUT);

  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");

   for(int i=0; i<6; i++){
    pinMode(leds[i], INPUT_PULLUP);
   }
  //data LED input
  //pinMode(5, INPUT);
  //pinMode(6, INPUT);
 //pinMode(9, INPUT);
  //pinMode(10, INPUT);
  //pinMode(11, INPUT);
  //pinMode(12, INPUT);
}


int timeout = 10000;
int timerEvent;
bool sendMessages = false;
bool timerIsRunning = false;
bool stateHasChanged = false;
char prevPacket[6];
void loop() {
  delay(10);  // Wait 1 second between transmits, could also 'sleep' here!
  
  char radiopacket[6];
  
  //get LED state...
   bool allAreOff = true;
   stateHasChanged = false;
   //Serial.print("new packet:");
   for(int i=0; i<6; i++){
    int buttonState = digitalRead(leds[i]);
    //Serial.print(i);
    //Serial.print(":");
    //Serial.print(radiopacket[i]);
    //Serial.println(radiopacket);
      if(buttonState){
        //led is OFF
         radiopacket[i] = '0';
      }
      else{
        //led is on
         radiopacket[i] = '1';
         allAreOff = false;
      }
      //compare current led state to previous led state
      //stateHasChanged = false;
     
      if(radiopacket[i] != prevPacket[i]){
       /* Serial.print("new packet [");
        Serial.print(i);
        Serial.print("] = ");
        Serial.println(radiopacket[i]);*/
        stateHasChanged = true;
      }
   }
  //Serial.println("end of packet");
  
  // Send a message!
  if(stateHasChanged){
    Serial.print("Sending "); Serial.println(radiopacket);
    rf69.send((uint8_t *)radiopacket, strlen(radiopacket));
  }

  //save previous led state
  memcpy(prevPacket, radiopacket, 6);
}


