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

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, mumst match RX's freq!
#define RF69_FREQ 915.0

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
#endif

#if defined(ARDUINO_SAMD_FEATHER_M0) // Feather M0 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     3
  #define RFM69_RST     4
  #define LED           13
#endif

#if defined (__AVR_ATmega328P__)  // Feather 328P w/wing
  #define RFM69_INT     3  // 
  #define RFM69_CS      4  //
  #define RFM69_RST     2  // "A"
  #define LED           13
#endif

#if defined(ESP8266)    // ESP8266 feather w/wing
  #define RFM69_CS      2    // "E"
  #define RFM69_IRQ     15   // "B"
  #define RFM69_RST     16   // "D"
  #define LED           0
#endif

#if defined(ESP32)    // ESP32 feather w/wing
  #define RFM69_RST     13   // same as LED
  #define RFM69_CS      33   // "B"
  #define RFM69_INT     27   // "A"
  #define LED           13
#endif

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission

//LED stuff
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define ledPIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      54

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, ledPIN, NEO_GRB + NEO_KHZ800);

void setup() 
{
  Serial.begin(115200);
  //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("Feather RFM69 RX Test!");
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

  pixels.begin(); // This initializes the NeoPixel library.
}


void loop() {
 if (rf69.available()) {
    // Should be a message for us now   
   // uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t buf[6];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      Serial.print("Received [");
      Serial.print(len);
      Serial.print("]: ");
      Serial.println((char*)buf);
     // Serial.print("RSSI: ");
     // Serial.println(rf69.lastRssi(), DEC);

      //handle message
      for(int i=0; i<6; i++){
        bool isOn = true;
        
        if (buf[i] == '0') 
          isOn = false;

        lightLED(i, isOn);
      }

      pixels.show(); // This sends the updated pixel color to the hardware.
      
/*if (strstr((char *)buf, "Hello World")) {
        // Send a reply!
        uint8_t data[] = "And hello back to you";
        rf69.send(data, sizeof(data));
        rf69.waitPacketSent();
        Serial.println("Sent a reply");
        Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
      }
      */
    } else {
      Serial.println("Receive failed");
    }
  }
}

//define colors
int red[] = {255, 0, 0};
int orange[] = {255, 100, 0};
int green[] = {0, 255, 0};
int off[] = {0, 0, 0};

void lightLED(int i, bool isOn){
  //Serial.print(isOn);
  Serial.print("LED #");
  Serial.print(i);
  
  if(isOn){
    Serial.println(" is ON");
    if(i<2){
      //GREEN
      lightLEDRange(i, green);
    //  pixels.setPixelColor(i, pixels.Color(0,255,0));
      
    }
    else if(i<4){
      //ORANGE
      lightLEDRange(i, orange);
      //pixels.setPixelColor(i, pixels.Color(255,255,0));
    }
    else{
      //RED
      lightLEDRange(i, red);
      //pixels.setPixelColor(i, pixels.Color(255,0,0));
    }
  }
  else
  {
    Serial.println(" is OFF");
    lightLEDRange(i, off);
   // pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
  }
    
}



void lightLEDRange(int i, int c[]){
  int startIndex = i * 9;
  int endIndex = ((i+1) * 9) - 1;
  for(int l=startIndex; l<= endIndex; l++){
    pixels.setPixelColor(l, pixels.Color(c[0],c[1],c[2])); // Moderately bright green color.
  }
  
}

void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
