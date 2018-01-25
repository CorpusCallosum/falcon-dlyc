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

//declare LED patterns
/*uint8_t biDimArray [5][5] { //initialize to zero
  {'0', '0', '0', '0', '0'},  //most recent - trigger sound here, if pattern matches?
  {'0', '0', '0', '0', '0'},  //one back
  {'0', '0', '0', '0', '0'},  //two ago
  {'0', '0', '0', '0', '0'},  //three ago
  {'0', '0', '0', '0', '0'}   //four ago
};*/

//Pattern patterns[5];

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
  

  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");

  //init sound
  initSound();
}

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

       for(int i=0; i<6; i++){
        if (buf[i] == '0'){
        }
        else if (buf[i] == '1') {
        }
        else{
          Serial.println("corrupt data received");
          return;
        }
      }

      handleSounds(buf, prevBuf);
      
      //copy buf to memory
      memcpy(prevBuf, buf, 6);
    } else {
      Serial.println("Receive failed");
    }

    timerRunning = false;
    t.stop(timerEvent);
  }
  else{
    //no message

  }
  //update timer class
  t.update();
}

void onTimerDone(){
  Serial.println("timer done");
  timerRunning = false;
}


int searchCounter;
void handleSounds(uint8_t buf[6], uint8_t prevBuf[6]){
  if(buf[0] == '1' && prevBuf[0] == '0'){
    //if(searchCounter < 7){
      //search plays 7 times?
      playFile("searchp.mp3");
     // searchCounter++;
    //}
  }

  if(buf[0] == '1' && prevBuf[5] == '1'){
  //  playFile("searchp.mp3");
  }
  
}

int soundTimer;
bool soundTimerDone = true;
void playFile(char *n){
 // if(soundTimerDone){
 //   soundTimer = t.after(1000, onSoundTimerDone); //after 1 minute, start default animation
    musicPlayer.stopPlaying();
    musicPlayer.startPlayingFile(n);
 // }
}

void onSoundTimerDone(){
  soundTimerDone = true;
  t.stop(soundTimer);
}

bool compareBufs(uint8_t a[6], uint8_t b[6]){
  //compare two arrays...
}

//*************
//INITIALIZATION
//INIT SOUND
void initSound(){
  //disable radio as test...
  //pinMode(8, INPUT_PULLUP);
  
  // Wait for serial port to be opened, remove this line for 'standalone' operation
  //Need to uncomment this, to get serial communication working...
  while (!Serial) { delay(1); }
  
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
  Serial.println("Playing startup sound");
  musicPlayer.startPlayingFile("poweron.mp3");
  //playFile("poweron.mp3");
  //delay(1000);
}

