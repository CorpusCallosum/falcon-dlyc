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

// Change to 434.0 or other frequency, mumst match RX's freq!
#define RF69_FREQ 915.0

//#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13

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
#define ledPIN            11
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      48

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, ledPIN, NEO_GRB + NEO_KHZ800);

//define colors
uint32_t red = strip.Color(255, 0, 0);
uint32_t orange = strip.Color(255, 100, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t black = strip.Color(0, 0, 0);

void setup() 
{
  Serial.begin(115200);
  //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("Feather RFM69 RX Test!");
  Serial.println();

  //If you're having glitchy hangs, you can fix this intermediately by either...
  pinMode(1, INPUT_PULLUP);

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

  strip.begin(); // This initializes the NeoPixel library.
  //turn off LEDS on start
  clearLEDs();

}


bool playDefaultAnimation = true; //play default animation on start...
int defaultAnimationTime = 10000; // 10s
int timerEvent;
bool timerRunning = false;
uint8_t prevBuf[6] = {'0','0','0','0','0'};;

void loop() {
 if (rf69.available()) {
    // Should be a message for us now   
    uint8_t buf[6] = {'0','0','0','0','0'};
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      Serial.print("Received [");
      Serial.print(len);
      Serial.print("]: ");
      Serial.println((char*)buf);

      //make sure data is not corrupt

      //handle message

      //start by clearing them...
      
      for(int i=0; i<6; i++){
       // bool isOn = true;
        
        if (buf[i] == '0'){
          lightLED(i, false);
        }
        else if (buf[i] == '1') {
          lightLED(i, true);
        }
        else{
          Serial.println("corrupt data received");
          return;
        }
           
      }

      strip.show(); // This sends the updated pixel color to the hardware.
      //copy buf to memory
      memcpy(prevBuf, buf, 6);
    } else {
      Serial.println("Receive failed");
    }
    playDefaultAnimation = false;
    timerRunning = false;
    t.stop(timerEvent);
  }
  else{
    //no message
    //Serial.println("no messages");
    if(!playDefaultAnimation){
     // Serial.println("default animation not running");
      //start timer
      if(!timerRunning){
       // Serial.println("timer not running - start timer");
        t.stop(timerEvent);
        timerEvent = t.after(defaultAnimationTime, onTimerDone); //after 1 minute, start default animation
        timerRunning = true;
      }
    }else{
      updateDefaultAnimation();
    }
  }
  //update timer class
  t.update();
}

int defaultLEDIndex = 0;
int defaultLEDCounter = 1;
void onTimerDone(){
  Serial.println("timer done - start default animation");
  playDefaultAnimation = true;
  defaultLEDIndex = 0;
  defaultLEDCounter = 1;
  timerRunning = false;
}

void lightLED(int i, bool isOn){
  if(isOn){
   // Serial.println(" is ON");
    if(i<2){
      //GREEN
      lightLEDGroup(i, green);
    }
    else if(i<4){
      //ORANGE
      lightLEDGroup(i, orange);
    }
    else{
      //RED
      lightLEDGroup(i, red);
    }
  }
  else
  {
    lightLEDGroup(i, black);
  }
    
}

int pps = 8; //number of pixels per segment
void lightLEDGroup(int i, uint32_t c){
  int startIndex = i * pps;
  int endIndex = ((i+1) * pps) - 1;
  for(int l=startIndex; l<= endIndex; l++){
    if(l!=0)
      strip.setPixelColor(l, c); 
  }
}

//DEFAULT ANIAMTION CODE
void updateDefaultAnimation(){
  if(defaultLEDIndex >= NUMPIXELS){
    defaultLEDCounter = -1;
   
  }else if(defaultLEDIndex<=0){
    defaultLEDCounter = 1;
    
  }
  defaultLEDIndex += defaultLEDCounter;
  
  lightLEDRange(0,defaultLEDIndex);
  delay(defaultLEDIndex+10);
}

int cg = NUMPIXELS/3; //number of pixels per color
void lightLEDRange(int first, int last){
  strip.clear();
  if(first == 0)
    first++; //SKIP FIRST LED
  for(int i=first;i<last; i++){
    if(i<cg){
      //green
      strip.setPixelColor(i, green); // Moderately bright green color.c
    }
    else if(i<cg*2){
      strip.setPixelColor(i, orange);
    }
    else if(i<cg*3){
      strip.setPixelColor(i, red);
    }
  }
  strip.show();
}

void clearLEDs(){
  strip.clear();
  strip.show();
}

