    /**
   Nom : 
   Version initial par: Esteban GIORGIS
   Version initial créé le: 13.05.2022
   Dernière version le: 13.05.2022
**/

//Librairie
#include <Adafruit_BME280.h>//librairie pour le capteur BME280
#include <RTClib.h>//librairie pour la RTC
#include "Adafruit_LEDBackpack.h"//librairie pour l'affichage 7 segments

//Variables Globales
Adafruit_BME280 bme;//initialisation du bme
RTC_DS1307 rtc = RTC_DS1307();//initialisation de la RTC
Adafruit_24bargraph bar = Adafruit_24bargraph();

//Variables 
unsigned rtcStatus;

void setup() {
  Serial.begin(9600);

  //Partie RTC
  rtcStatus = rtc.begin();//initialisation du port de communication pour la RTC
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//recupère la date et le temps indiqué par notre propre ordinateur

  //Partie affichage 7 segments
  bar.begin(0x70);  // pass in the address
  bar.setBrightness(15);

}

void loop() {

  //secondes

  bar.writeDigitNum(1, 0);
  delay(195);

  Serial.println(getCurrentTime());//affiche l'heure actuelle
}



char* getCurrentTime(){
  char* currentTime;
  currentTime = (char*)malloc(20);//malloc pour memory allocation, permet, comme son nom peut l'indiquer, de détérminer manuellement le nombre de bit pour cette variable
    
   
   DateTime now = rtc.now();
   sprintf(currentTime,"%d:%d",now.hour(),now.minute());//permmet de "formatter" le contenu de la variable ". Attetion ! sprintf ne permet pas d'afficher du contenu dans le terminal
   return currentTime;
}
