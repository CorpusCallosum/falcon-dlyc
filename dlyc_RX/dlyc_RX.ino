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
#include <SD.h>
#include <Adafruit_VS1053.h>
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
#define NUMPIXELS      54

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, ledPIN, NEO_GRB + NEO_KHZ800);

//define colors
uint32_t red = strip.Color(255, 0, 0);
uint32_t orange = strip.Color(255, 100, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t black = strip.Color(0, 0, 0);

//SOUND stuff
// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS       6     // VS1053 chip select pin (output)
  #define VS1053_DCS     10     // VS1053 Data/command select pin (output)
  #define CARDCS          5     // Card chip select pin
  // DREQ should be an Int pin *if possible* (not possible on 32u4)
  #define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

  Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

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

  strip.begin(); // This initializes the NeoPixel library.
  //turn off LEDS on start
  clearLEDs();

  //init sound
  initSound();
}


bool playDefaultAnimation = true; //play default animation on start...
int defaultAnimationTime = 5000; // 1 minute
int timerEvent;
bool timerRunning = false;
void loop() {
 if (rf69.available()) {
    // Should be a message for us now   
    uint8_t buf[6];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      Serial.print("Received [");
      Serial.print(len);
      Serial.print("]: ");
      Serial.println((char*)buf);

      //handle message
      for(int i=0; i<6; i++){
        bool isOn = true;
        
        if (buf[i] == '0') 
          isOn = false;

        lightLED(i, isOn);
      }

      strip.show(); // This sends the updated pixel color to the hardware.

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

void onTimerDone(){
  Serial.println("timer done - start default animation");
  playDefaultAnimation = true;
  int defaultLEDIndex = 0;
  int defaultLEDCounter = 1;
  timerRunning = false;
}

void lightLED(int i, bool isOn){
  //Serial.print(isOn);
  Serial.print("LED #");
  Serial.print(i);
  
  if(isOn){
    Serial.println(" is ON");
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

void lightLEDGroup(int i, uint32_t c){
  int startIndex = i * 9;
  int endIndex = ((i+1) * 9) - 1;
  for(int l=startIndex; l<= endIndex; l++){
    strip.setPixelColor(l, c); // Moderately bright green color.
  }
}

//DEFAULT ANIAMTION CODE
int defaultLEDIndex = 0;
int defaultLEDCounter = 1;
void updateDefaultAnimation(){
  if(defaultLEDIndex >= NUMPIXELS){
    defaultLEDCounter = -1;
  }else if(defaultLEDIndex<=0){
    defaultLEDCounter = 1;
    musicPlayer.startPlayingFile("searchp.mp3");
  }
  defaultLEDIndex += defaultLEDCounter;
  
  lightLEDRange(0,defaultLEDIndex);
  delay(defaultLEDIndex+10);
}

void lightLEDRange(int first, int last){
  int cg = NUMPIXELS/3;
  strip.clear();
  for(int i=first;i<last; i++){
    if(i<cg){
      //green
      strip.setPixelColor(i, green); // Moderately bright green color.
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

//*************
//INITIALIZATION
//INIT SOUND
void initSound(){
  //disable radio as test...
  //pinMode(8, INPUT_PULLUP);
  
  // Wait for serial port to be opened, remove this line for 'standalone' operation
  //Need to uncomment this, to get serial communication working...
  //while (!Serial) { delay(1); }
  
  Serial.println("\n\nAdafruit VS1053 Feather Test");
  
  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }

  Serial.println(F("VS1053 found"));
 
  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(10,10);
  
#if defined(__AVR_ATmega32U4__) 
  // Timer interrupts are not suggested, better to use DREQ interrupt!
  // but we don't have them on the 32u4 feather...
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
#else
  // If DREQ is on an interrupt pin we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
#endif
  
  // Play a file in the background, REQUIRES interrupts!
  Serial.println(F("Playing startup sound"));
  musicPlayer.playFullFile("poweron.mp3");
  //delay(1000);
}

