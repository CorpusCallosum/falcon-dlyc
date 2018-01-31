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
const int numPins = 6;
int thresh = 50;
int threshs[numPins] = {900,800,800,700,700,900};
int pins[numPins] = {5,4,3,2,1,0};//define analog pin order

//smoothing to establish baseline
// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
const int numReadings = 10;

int readings[numPins][numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int totals[numPins];                  // the running total
int average = 0;                // the average

bool VERBOSE = false;
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

  // initialize all the averaged readings to 0:
  for (int i = 0; i < numPins; i++) {
    totals[i] = 0;
    for (int thisReading = 0; thisReading < numReadings; thisReading++) {
      readings[i][thisReading] = 0;
    }
  }
}

bool sendMessages = false;
bool stateHasChanged = false;
char prevPacket[6];
void loop() {
 
  
  char radiopacket[6];
  
  //get LED state...
   stateHasChanged = false;
   //Serial.print("new packet:");
   for(int i=0; i<6; i++){
    int v = analogRead(pins[i]);

    //calculate smoothed value
    int smoothV = calcSmoothing(i,v);

    if(VERBOSE){
      Serial.print(i);
      Serial.print(":");
      Serial.print(smoothV);
      Serial.print(":");
      Serial.print(v);
      Serial.print(",");
    }
    
      if(v - smoothV > thresh){
        //led is OFF
         radiopacket[i] = '1';
      }
      else if(smoothV - v > thresh){
        //led is on
         radiopacket[i] = '0';
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

    // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  //Serial.println("");

  delay(10);  // Wait between transmits, could also 'sleep' here!
  // Send a message!
  if(stateHasChanged){
    
    if(VERBOSE){
      Serial.print("Sending "); Serial.println(radiopacket);
    }
      
    rf69.send((uint8_t *)radiopacket, strlen(radiopacket));
  }

  //save previous led state
  memcpy(prevPacket, radiopacket, 6);

 // Serial.flush();
}

int calcSmoothing(int index, int value){
  // subtract the last reading:
  totals[index] -= readings[index][readIndex];
  // read from the sensor:
  readings[index][readIndex] = value;
  // add the reading to the total:
  totals[index] += value;

  // calculate the average:
  average = totals[index] / numReadings;
  // send it to the computer as ASCII digits
 // Serial.println(average);
  return(average);
 // delay(1);        // delay in between reads for stability
}


