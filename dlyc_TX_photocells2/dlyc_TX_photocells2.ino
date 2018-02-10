// DLCY Transmit Code
// Made by: Floating Point LLC - www.floating.pt
// Developer: Jack Kalish
// Date: January 2018
//
// Read data in from analog pins and send as byte array message radio packet.


#include <SPI.h>
#include <RH_RF69.h>

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

//photocell thresholds
int thresh = 700;
int threshs[6] = {900,800,900,800,900,900};
int pins[6] = {5,4,3,2,1,0};//define analog pin order

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

}

bool sendMessages = false;
bool stateHasChanged = false;
char prevPacket[6];
void loop() {
  delay(10);  // Wait between transmits, could also 'sleep' here!
  
  char radiopacket[6];
  
  //get LED state...
   stateHasChanged = false;
   //Serial.print("new packet:");
   for(int i=0; i<6; i++){
    int v = analogRead(pins[i]);
    //Serial.print(i);
    //Serial.print(":");
    //Serial.print(v);
    //Serial.print(",");
      if(v < threshs[i]){
        //led is OFF
         radiopacket[i] = '0';
      }
      else{
        //led is on
         radiopacket[i] = '1';
      }
      //compare current led state to previous led state
     
      if(radiopacket[i] != prevPacket[i]){
       /* Serial.print("new packet [");
        Serial.print(i);
        Serial.print("] = ");
        Serial.println(radiopacket[i]);*/
        stateHasChanged = true;
      }
   }

  //Serial.println("");
  
  // Send a message!
  if(stateHasChanged){
    Serial.print("Sending "); Serial.println(radiopacket);
    rf69.send((uint8_t *)radiopacket, strlen(radiopacket));
  }

  //save previous led state
  memcpy(prevPacket, radiopacket, 6);
}


