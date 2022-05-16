    /**
   Nom : 
   Version initial par: Esteban GIORGIS
   Version initial créé le: 13.05.2022
   Dernière version le: 16.05.2022
**/

//Librairies
#include <Adafruit_BME280.h>//librairie pour le capteur BME280
#include <RTClib.h>//librairie pour la RTC
#include "Adafruit_LEDBackpack.h"//librairie pour l'affichage 7 segments
#include "Adafruit_SGP30.h"//librairie pour le capteur de CO2 (SGP30)

//Variables Globales
Adafruit_BME280 bme;//initialisation du bme
RTC_DS1307 rtc = RTC_DS1307();//initialisation de la RTC
Adafruit_7segment backPackDisplay = Adafruit_7segment();//initialisation de l'affichage 7 segments

//Variables 
unsigned rtcStatus;
unsigned bmeStatus;
int hours = 0;
int minutes = 0;
int seconds = 0;
bool doublePoints = false;//cette variable permet l'affichage des secondes en alternant sa valeur par true et false chaque seconde
int displayChoice;//Variable indiquant au programme quel type d'affichage pour l'affichage 7 segments

void setup() {
  Serial.begin(9600);
  displayChoice = 0;
  
  //Partie RTC
  rtcStatus = rtc.begin();//initialisation du port de communication pour la RTC
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//recupère la date et le temps indiqué par notre propre ordinateur

  //Partie affichage 7 segments
  backPackDisplay.begin(0x70);//Initialisation de l'affichage en indiquant l'addresse I2C
  backPackDisplay.setBrightness(15);//Taux de luminosité de 0 à 15

  //Parite température avec le capteur BME280
  bmeStatus = bme.begin();//initialisation du capteur BME


  //Si le capteur BME280 et/ou la RTC n'est pas trouvé alors affiche un text l'indiquant et arrête le programme
  if (bmeStatus != 1 || rtcStatus != 1) {
    Serial.println("Problème de montage");
    exit(0);//arrêt complet du programme
  }
}

void loop() {

 switch(displayChoice){
  case 0:
    displayCurrentTime();
    delay(1000);
    displayTemp();//Appelle de la fonction permettant d'affichage l'heure sur l'affichage 7 segments
    delay(15000);
    break;
    
  default:
    displayCurrentTime();
    delay(3000);
    displayTemp();//Appelle de la fonction permettant d'affichage l'heure sur l'affichage 7 segments
    break;
 }
 
}


/*
 * Cette fonction retourne,au format integer, l'heure actuelle grace à la RTC 
 * @return une valeur integer correspondant à l'heure actuelle récupérée par la RTC
*/
int getCurrentTime(){
  
  DateTime now = rtc.now();//Récupère le temps actuelle avec la RTC
  hours = now.hour();
  minutes = now.minute();
  seconds = now.second();
  
  int currentTime = hours*100 + minutes; //Convertie le temps reçu par la RTC en numérique en le multipliant par 100 puis en additionnant les minutes.
  
  return currentTime;
}


/*
 * Cette fonction affichage sur l'affichage 7 segments l'heure reçu par la RTC
*/
void displayCurrentTime(){
  backPackDisplay.print(getCurrentTime());//Prépare l'affichage de l'horraire actuel sur l'affichage 7 segments

  //ajoute un 0 au digit précédent dans le cas ou la valeur des heures et/ou des minutes sont en dessous de 10
  if (hours == 0) {
    backPackDisplay.writeDigitNum(1, 0);//ajoute un 0 avant l'affichage des heures
    if (minutes < 10) {
      backPackDisplay.writeDigitNum(3, 0);//ajoute un 0 avant l'affichage des minutes
    }
  }

  doublePoints = !doublePoints;//Inverse la valeur de la variable "doublePoints" pour l'affichage des deux petits points 
  backPackDisplay.drawColon(doublePoints);//Prépare l'affichage ou non des deux petits point selon la valeur de la variable "doublePoints"
  backPackDisplay.writeDisplay();//Affiche toutes les informations préparées au préalable sur l'affichage 7 segments
  delay(1000);//delai d'une seconde
}



/*
 * Fonction qui retourne la température en degrés celsius "°C"
 * @return une valeur type float correspondant à la valeur de la température en degrés celsius "°C"
*/
float getTempCelsius(){ 
  return bme.readTemperature();
}


/*
 * Cette fonction affichage sur l'affichage 7 segments la température mesurée par le capteur BME280
*/
void displayTemp(){
  backPackDisplay.print(getTempCelsius());//Prépare l'affichage de la température mesurée par le capteur BME280 sur l'affichage 7 segments
  backPackDisplay.writeDisplay();//Affiche toutes les informations préparées au préalable sur l'affichage 7 segments
}
