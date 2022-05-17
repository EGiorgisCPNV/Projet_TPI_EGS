/**
   Nom : Projet TPI 
   Version initial par: Esteban GIORGIS
   Version initial créé le: 13.05.2022
   Dernière version le: 17.05.2022
**/

//Librairies
#include <Adafruit_BME280.h>//librairie pour le capteur BME280
#include <RTClib.h>//librairie pour la RTC
#include "Adafruit_LEDBackpack.h"//librairie pour l'affichage 7 segments
#include "Adafruit_SGP30.h"//librairie pour le capteur de CO2 (SGP30)
#include <Adafruit_NeoPixel.h>//librairie pour l'horloge 60 LED

//Constantes
#define NUMPIXELS 60 //Nombre de LED
#define BACKPACK_LUMINOSITY 10 //Valeur indiquant la luminosité de l'affichage 7 segments de 0 à 15
#define PIXELS_LUMINOSITY 100 //Valeur indiquant la luminosité des LED de l'horloge de 0 à 255
#define WAITING_TIME 3000 //Valeur, en millisecondes, indiquant le nombre de millisecondes que va durer chaque alternance pour le premier mode d'affichage de l'affichage 7 segments
#define MAX_CO2_ALLOWED 550 //Le taux de CO2 maximal autorisé avant une alerte

//Variables Globales
Adafruit_BME280 bme;//initialisation du bme
RTC_DS1307 rtc = RTC_DS1307();//initialisation de la RTC
Adafruit_SGP30 sgp = Adafruit_SGP30();//initialisation du capteur de CO2 SGP30
Adafruit_7segment backPackDisplay = Adafruit_7segment();//initialisation de l'affichage 7 segments
Adafruit_NeoPixel ledClock(NUMPIXELS, 2);//Initialisation de l'horloge 60 LED en commençant par indiquer le nombre de LED total que possède notre horloge(60) et sur quel pin elle est branchée(2)

//Variables 
unsigned rtcStatus;
unsigned bmeStatus;
unsigned sgpStatus;
int hours = 0;
int minutes = 0;
int seconds = 0;
bool doublePoints = false;//cette variable permet l'affichage des secondes en alternant sa valeur par true et false chaque seconde
int displayChoice;//Variable indiquant au programme qu'est-ce que l'affichage 7 segments doit afficher
int alertChoice;//Variable indiquant au programme qu'elle alarme il doit utiliser dans le cas ou le taux de CO2 dépasse un seuil voulu
int stateDisplayButton;//variable lié a l'état du bouton poussoir pour changer l'affichage 7 segments
int reverseStateDisplayButton;//variable contenant l'inverse de la variable "stateDisplayButton" qui permettra ensuite de bloquer le changement d'état si l'utilisateur reste appuié sur le boutton poussoir
int stateAlertButton;//variable lié a l'état du bouton poussoir pour changer l'alerte en cas de trop haut taux de CO2
int reverseStateAlertButton;//variable contenant l'inverse de la variable "stateAlertButton" qui permettra ensuite de bloquer le changement d'état si l'utilisateur reste appuié sur le boutton poussoir



void setup() {
  Serial.begin(9600);
  displayChoice = 0;//Initialisation de la variable "displayChoice" à 0 
  alertChoice = 0;//Initialisation de la variable "alertChoice" à 0 
  pinMode(3, INPUT_PULLUP);//initialisation de la pin 3 en INPUT_PULLUP
  pinMode(4, INPUT_PULLUP); // initialisation de la pin 4 en INPUT_PULLUP
  pinMode(5, INPUT_PULLUP); // initialisation de la pin 5 en INPUT_PULLUP
    
  //Partie RTC
  rtcStatus = rtc.begin();//initialisation du port de communication pour la RTC
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//recupère la date et le temps indiqué par notre propre ordinateur

  //Partie affichage 7 segments
  backPackDisplay.begin(0x70);//Initialisation de l'affichage en indiquant l'addresse I2C
  backPackDisplay.setBrightness(BACKPACK_LUMINOSITY);//Taux de luminosité de 0 à 15

  //Parite température avec le capteur BME280
  bmeStatus = bme.begin();//initialisation du capteur BME

  //Partie capteur de CO2 (SGP30)
  sgpStatus = sgp.begin();//initialisation du capteur de CO2 SGP30

  //Partie horloge 60 LED
  ledClock.begin(); //initialisation de l'horloge a LED
  ledClock.setBrightness(PIXELS_LUMINOSITY);//Taux de luminosité de 0 à 255

  turnOffClockLED();//Fonction qui étain les LED

 
  //Si le capteur BME280 et/ou la RTC n'est pas trouvé alors affiche un texte l'indiquant et arrête le programme
  if (bmeStatus != 1 || rtcStatus != 1 || sgpStatus != 1) {
    Serial.println("Problème de branchement");
    exit(0);//arrêt complet du programme
  }
}


void loop() {
  
  //Ce switch case permet de faire varier le contenu de l'affichage 7 segments suivant la valeur de la variable "displayChoice"
  switch(displayChoice){
  case 0:
      alternationDisplay();//Appel de la fonction permettant d'afficher en alternance la température / heure / taux de CO2
      break;
  
  case 1:
      displayTemperature();//Appel de la fonction permettant d'afficher la température sur l'affichage 7 segments
    break;
  
  case 2:
      displayCurrentTime();//Appel de la fonction permettant d'afficher l'heure sur l'affichage 7 segments
    break;

  case 3:
      displayCOTwoRate();//Appel de la fonction permettant d'afficher le taux de CO2 sur l'affichage 7 segments;
    break;
    
  default:
      alternationDisplay();//Appel de la fonction permettant d'afficher en alternance la température / heure / taux de CO2
    break;
  }
}


 /*
 * Description : Cette fonction permet de changer l'état de certaine variable selon si le bouton poussoir branché sur la pin 3 est pressé ou non.
 * Elle empêche aussi l'utilisateur de changer la valeur de la variable "displayChoice" en restant appuyé
 */
 void fallingEdgeDetection(){
  stateDisplayButton = digitalRead(3);
  stateAlertButton = digitalRead(5);
  delay(100);
  
  //Incrémente la variable "displayChoice" à chaque clique du bouton poussoir "stateDisplayButton"
  if(stateDisplayButton == 0 && stateDisplayButton == reverseStateDisplayButton){
    
      displayChoice++;
      
      //Une fois que la variable "displayChoice" a atteint 3 alors elle repasse à 0
      if(displayChoice > 3){
        displayChoice = 0;
      }
      Serial.println(displayChoice);
      
  }
  reverseStateDisplayButton = !stateDisplayButton;


  //Incrémente la variable "alertChoice" à chaque clique du bouton poussoir "stateAlertButton"
  if(stateAlertButton == 0 && stateAlertButton == reverseStateAlertButton){
    
      alertChoice++;
      
      //Une fois que la variable "displayChoice" a atteint 3 alors elle repasse à 0
      if(alertChoice > 3){
        alertChoice = 0;
      }
      
  }
  reverseStateAlertButton = !stateAlertButton;
  
 }


/*
 * Description: Cette fonction retourne, au format integer, l'heure actuelle grâce à la RTC.
 * @return une valeur integer correspondant à l'heure actuelle récupérée par la RTC
*/
int getCurrentTime(){
  
  DateTime now = rtc.now();//Récupère le temps actuelle avec la RTC
  hours = now.hour();
  minutes = now.minute();
  seconds = now.second();
  
  int currentTime = hours*100 + minutes;//Converti le temps reçu par la RTC en numérique en le multipliant par 100 puis en additionnant les minutes.
  
  return currentTime;
}


/*
 * Description: Cette fonction affichage sur l'affichage 7 segments l'heure reçu par la RTC
*/
void displayCurrentTime(){
  fallingEdgeDetection();//Appel de la fonction pour vérifier l'état des deux boutons poussoir
  
  backPackDisplay.print(getCurrentTime());//Prépare l'affichage de l'horraire actuel sur l'affichage 7 segments

  //ajoute un 0 au digit précédent dans le cas ou la valeur des heures et/ou des minutes sont en dessous de 10
  if (hours < 10) {
    backPackDisplay.writeDigitNum(0, 0);//ajoute un 0 avant l'affichage des heures
    if (minutes < 10) {
      backPackDisplay.writeDigitNum(2, 0);//ajoute un 0 avant l'affichage des minutes
    }
  }

  doublePoints = !doublePoints;//Inverse la valeur de la variable "doublePoints" pour l'affichage des deux petits points 
  backPackDisplay.drawColon(doublePoints);//Prépare l'affichage ou non des deux petits point selon la valeur de la variable "doublePoints"
  backPackDisplay.writeDisplay();//Affiche toutes les informations préparées au préalable sur l'affichage 7 segments
  delay(1000);//delai d'une seconde
  
}


/*
 * Description: Retourne la température en degrés celsius "°C"
 * @return une valeur type float correspondant à la valeur de la température en degrés celsius "°C"
*/
float getTempCelsius(){ 
  return bme.readTemperature();//retourne la valeur de température mesurée par le capteur BME280
}


/*
 * Description: Cette fonction affichage sur l'affichage 7 segments la température mesurée par le capteur BME280
*/
void displayTemperature(){
  fallingEdgeDetection();//Appel de la fonction pour vérifier l'état des deux boutons poussoir
  
  backPackDisplay.print(getTempCelsius());//Prépare l'affichage de la température mesurée par le capteur BME280 sur l'affichage 7 segments
  backPackDisplay.writeDisplay();//Affiche toutes les informations préparées au préalable sur l'affichage 7 segments
}


/*
 * Description: Retourne le taux de CO2 en ppm (part per million). Suivant certaines conditions, cette fonction appellera une ou plusieurs alertes dans le cas ou le taux de CO2 dépasse un seuil voulu
 * Note : Le capteur a besoin d'environ 15 secondes avant qu'il puisse donner la vraie valeur du taux de CO2, durant les 15 premières secondes il retourne constamment la valeur 400
 * @return une valeur type float correspondant au taux de CO2 mesuré par le capteur SGP30
*/
float getCOTwoRate(){ 
   sgp.IAQmeasure();//demande au capteur de mesurer une seul mesure de eCO2 et de VOC. Place ensuite les mesures dans TVOC et eCO2

  //Tant que le taux de CO2 mesuré dépasse la valeure de la constante "MAX_CO2_ALLOWED" il effectue une alarme spécifique
  while(sgp.eCO2 >= MAX_CO2_ALLOWED){
    
      //Ce switch case permet de séléctionner l'alarme en cas de trop haut taux de CO2 suivant la valeur de la variable "alertChoice"
      switch(alertChoice){
      case 0:
            visualAlertForCOTwoRate();//Appel de la fonction qui effectue une alarme visuelle
            sonorAlertForCOTwoRate();//Appel de la fonction qui effectue une alarme sonore
          break;
      
      case 1:
            visualAlertForCOTwoRate();//Appel de la fonction qui effectue une alarme visuelle
        break;

      case 2:
            sonorAlertForCOTwoRate();//Appel de la fonction qui effectue une alarme sonore
        break;

      case 3:
        break;
                
      default:
          visualAlertForCOTwoRate();//Appel de la fonction qui effectue une alarme visuelle
        break;
      }
      
      
      sgp.IAQmeasure();//Reprise de mesure
      
      if(sgp.eCO2 < MAX_CO2_ALLOWED){
        break;  
      }
  }
    
   //Cette condition permet de bloquer la valeur à 9999 dans le cas où le capteur aurait mesuré une valeur supérieure à 9999
   if(sgp.eCO2 > 9999){
    return 9999; //retourne 9999 comme valeur
    }
   
   return sgp.eCO2; //retourne le taux de CO2 en ppm
}


/*
 * Description: Cette fonction affichage sur l'affichage 7 segments le taux de CO2 mesurée par le capteur SGP30 
*/
void displayCOTwoRate(){
  fallingEdgeDetection();//Appel de la fonction pour vérifier l'état des deux boutons poussoir
    
  backPackDisplay.print(getCOTwoRate());//Prépare l'affichage du taux de CO2 sur l'affichage 7 segments
  delay(250);//ajout d'un délai pour avoir un affichage plus agréable est ne pas rendre l'affichage 7 segments illisible à cause de la trop grande variation de prise de mesure du taux de CO2 par le capteur SGP30
  backPackDisplay.writeDisplay();//Affiche tous ce qui a été préparé 
}


/*
 * Description: Cette fonction affichera l'heure sur l'horloge 60 LED
*/
void displayTimeOnClock(){
  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  for(int i=0; i<NUMPIXELS; i++) {

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color
    ledClock.setPixelColor(i, ledClock.Color(0, 150, 0));

    ledClock.show();   // Send the updated pixel colors to the hardware.
    delay(500); // Pause before next pass through loop
    ledClock.clear();
    delay(500); // Pause before next pass through loop
  }
}


/*
 * Description: Cette fonction fait clignoter les 60 LED de l'horloge 60 LED, elle servira comme alerte visuelle en cas de trop haut taux de CO2
*/
void visualAlertForCOTwoRate(){
  //initialise chaque LED avec la couleur rouge
  for(int i=0; i<NUMPIXELS; i++) {
    ledClock.setPixelColor(i, ledClock.Color(150, 0, 0));
  }
  ledClock.show();//Affiche les couleurs précédemment initialisées
  delay(100);
  

  //initialise chaque LED à 0 donc sans couleurs
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    ledClock.setPixelColor(i, ledClock.Color(0, 0, 0));
  }
  ledClock.show();//Affiche les couleurs précédemment initialisées
  delay(100);
}


/*
 * Description: Cette fonction étaind chaque LED de l'horloge 
*/
void turnOffClockLED(){
  
  //initialise chaque LED à 0 donc sans couleurs
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    ledClock.setPixelColor(i, ledClock.Color(0, 0, 0));
  }
  ledClock.show();//Affiche les couleurs précédemment initialisées  
  
}


/*
 * Description: Cette fonction fait sonner le buzzer en alternance, elle servira comme alerte sonore en cas de trop haut taux de CO2
*/
void sonorAlertForCOTwoRate(){

  tone(4, 100); // Send 1KHz sound signal... Envoie 100 KHz de signal de son
  delay(5);
  noTone(4);//Stop le buzzer
  delay(1000);
  
}


/*
 * Description: Cette fonction permet d'afficher la température / heure / taux de CO2 en alternance. La durée de l'altérnance est défini par rapport a la constante "WAITING_TIME"
*/
void alternationDisplay(){
  fallingEdgeDetection();//Appel de la fonction pour vérifier l'état des deux boutons poussoir

  
  float nextDisplay = millis() + WAITING_TIME;//Variable contenant la durée de chaque alternance 
  
  //cette boucle while permet de faire altérner le contenu de l'affichage 7 segments
  while(millis() < nextDisplay){
    displayTemperature();//Appel de la fonction permettant d'afficher la température sur l'affichage 7 segments
  //Tant que la valeur retour de la fonction millis() n'a pas atteint celle de la variable "nextDisplay" alors continue à effectuer son code
  if(millis() >= nextDisplay){
    nextDisplay = millis() + WAITING_TIME;
    
     while(millis() < nextDisplay){
       displayCurrentTime();//Appel de la fonction permettant d'afficher l'heure sur l'affichage 7 segments
        if(millis() >= nextDisplay){
           nextDisplay = millis() + WAITING_TIME;
              
           while(millis() < nextDisplay){
             displayCOTwoRate();//Appel de la fonction permettant d'afficher le taux de CO2 sur l'affichage 7 segments
             if(millis() >= nextDisplay){
              break;
            }
          }
        }
      }
    }
  }
}
