/*************************************************** 
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED 24 Bargraph Backpack
  ----> http://www.adafruit.com/products/721

  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_24bargraph bar = Adafruit_24bargraph();

void setup() {
  Serial.begin(9600);
  Serial.println("HT16K33 Bi-Color Bargraph test");
  
  bar.begin(0x70);  // pass in the address
  bar.setBrightness(15);

  /*
  for (uint8_t b=0; b<50; b++){
   bar.setBar(b, LED_YELLOW);
   delay(300);
   bar.writeDisplay();
  }
  */


}


void loop() {

  //secondes
  bar.setBar(9, LED_YELLOW);
  bar.writeDisplay();
  delay(195);
  
  bar.clear();
  bar.writeDisplay();
  delay(800);
  
  
}
