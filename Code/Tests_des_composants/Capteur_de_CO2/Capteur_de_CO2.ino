/**
   Nom : Prise en main du capteur SGP30
   Version initial par: Adafruit_SGP30
   Version initial créé le: 12.05.2022
   Dernière version le: 12.05.2022
**/

#include <Wire.h>
#include "Adafruit_SGP30.h"//librairie pour le capteur SGP30

Adafruit_SGP30 sgp = Adafruit_SGP30();

void setup() {
  Serial.begin(9600);
  while (!Serial) { delay(10); } // Wait for serial console to open!


  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.print("Found SGP30 serial #");



}

int counter = 0;
void loop() {

  //IAQmeasure demande au capte de mesurer une seul mesure de eCO2 et de VOC. Place ensuite les mesures dans TVOC et eCO2
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
 
  Serial.print("eCO2 "); 
  Serial.print(sgp.eCO2);//sgp.eCO2: valeur récupérée de IAQmeasure()
  Serial.println(" ppm");
 
  delay(500);

  
  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x"); 
    Serial.print(eCO2_base);
  }
  
}
