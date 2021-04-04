// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

#include <Bounce2.h> // https://github.com/thomasfredericks/Bounce2
#include <U8x8lib.h>
#include <SPI.h>
#include <RH_RF95.h>

// heltec wifi lora 32 v2
#define RFM95_CS 18
#define RFM95_RST 14
#define RFM95_INT 26

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define LED 25

#define BUTTON_A_PIN 36 

Button button_A = Button();

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

typedef struct {
  int light;
} Payload;

Payload theData;

Payload recData;

void setup() 
{

u8x8.begin();
  
  u8x8.setFont(u8x8_font_7x14B_1x2_f);
   u8x8.clear();
   u8x8.setCursor(0,0); 
   u8x8.print("Starting...");
   delay(1000);
   
  button_A.attach( BUTTON_A_PIN, INPUT ); // USE EXTERNAL PULL-UP
  button_A.interval(5); 
  button_A.setPressedState(LOW);
  
  pinMode(LED, OUTPUT);
  
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);
  /*while (!Serial) {
    delay(1);
  }
  */

  delay(100);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop()
{

  button_A.update();
  
   if ( button_A.pressed() ) {
    packetnum++;
    if (packetnum==2) packetnum=0;

    theData.light = packetnum;
    
   // Wait 1 second between transmits, could also 'sleep' here!
  Serial.println("Transmitting..."); // Send a message to rf95_server
  
  Serial.println("Sending...");
  delay(10);
  
  rf95.send((uint8_t *)&theData, sizeof(theData));


  Serial.println("Waiting for packet to complete..."); 
  delay(10);
  digitalWrite(LED, HIGH);
  rf95.waitPacketSent();
  digitalWrite(LED, LOW);
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply...");
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("Got reply: ");
      recData = *(Payload*)buf;
      
      Serial.print("Got: ");
      Serial.println(recData.light);
      //Serial.println(rf95.lastRssi(), DEC);    

      u8x8.clear();
     u8x8.setCursor(0,0); 

     //if(recData.light==1) {
     if(packetnum==1) {
      u8x8.print("light: ON");
     }
     //if(recData.light==0) {
     if(packetnum==0) {
     u8x8.print("light: OFF");
     }
    }
    else
    {
     u8x8.clear();
     u8x8.setCursor(0,0);
     u8x8.print("no link?"); 
    }
  }
  else
  {
     u8x8.clear(); 
     u8x8.setCursor(0,0);
     u8x8.print("no link?");
  }

  //delay(5000);
   }
}
