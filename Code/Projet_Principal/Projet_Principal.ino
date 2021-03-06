/**
   Nom : Projet TPI 
   Version initial par: Esteban GIORGIS
   Version initial créé le: 13.05.2022
   Dernière version le: 29.05.2022
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
#define MAX_CO2_ALLOWED 1500 //Le taux de CO2 maximal autorisé avant une alerte
#define ALARME_DURATION 2000//Valeur en milliseconde indiquant la durée max de l'alerte en cas de trop haut taux de CO2
#define DELAY_ALARME_DURATION 5000//Valeur en milliseconde indiquant la durée à laquelle même si le taux de CO2 est trop élevé, aucune alarme ne sera générée


//Variables Globales
Adafruit_BME280 bme;//initialisation du bme
RTC_DS1307 rtc = RTC_DS1307();//initialisation de la RTC
Adafruit_SGP30 sgp = Adafruit_SGP30();//initialisation du capteur de CO2 SGP30
Adafruit_7segment backPackDisplay = Adafruit_7segment();//initialisation de l'affichage 7 segments
Adafruit_NeoPixel ledClock(NUMPIXELS, 2);//Initialisation de l'horloge 60 LED en commençant par indiquer le nombre de LED total que possède notre horloge(60) et sur quel pin elle est branchée(2)


//Variables liées au status des capteurs
unsigned rtcStatus;
unsigned bmeStatus;
unsigned sgpStatus;


//Variables liées aux capteurs
int calibrationCounter;//cette variable est un compteur pour effectuer régulièrement un calibrage du capteur SGP30 


//Variables liées au temps
int hours = 0;
int minutes = 0;
int seconds = 0;
unsigned long nextAlarm = 0;//cette variable indique le temps d'attente avant la prochaine alarme
bool doublePoints;//cette variable permet l'affichage des secondes en alternant sa valeur par true et false chaque seconde
unsigned long timeToWait;//Cette variable permet au programme, en plus de la constante "WAITING_TIME", d'alterner les 3 affichages. À noter que le type choisi "unsigned long" permet de faire durer un maximum de temps l'alternance des 3 affichages, en effet le type "unsigned" simple permet une valeur au maximum codée sur 4 octets, contrairement au type "unsigned long" qui permet une valeur max codée sur 8 octets.
unsigned lastSecond;//Cette variable permet de garder en mémoire pour chaque boucle effectué par la fonction "loop()" la dernière valeur retournée par la fonction "millis()"


//Variables liées à l'affichage 7 segments
int displayChoice;//Variable indiquant au programme qu'est-ce que l'affichage 7 segments doit afficher
int phase;//variable indiquant la phase dans laquelle on se trouve
int stateDisplayButton;//variable lié a l'état du bouton poussoir pour changer l'affichage 7 segments
int reverseStateDisplayButton;//variable contenant l'inverse de la variable "stateDisplayButton" qui permettra ensuite de bloquer le changement d'état si l'utilisateur reste appuié sur le boutton poussoir


//Variables liées aux alertes
int stateAlertButton;//variable lié a l'état du bouton poussoir pour changer l'alerte en cas de trop haut taux de CO2
int reverseStateAlertButton;//variable contenant l'inverse de la variable "stateAlertButton" qui permettra ensuite de bloquer le changement d'état si l'utilisateur reste appuié sur le boutton poussoir
int alertChoice;//Variable indiquant au programme qu'elle alarme il doit utiliser dans le cas ou le taux de CO2 dépasse un seuil voulu


void setup() {
  Serial.begin(9600);
  displayChoice = 0;//Initialisation de la variable "displayChoice" à 0 
  alertChoice = 0;//Initialisation de la variable "alertChoice" à 0 
  calibrationCounter = 0;//Initialisation de la variable "calibrationCounter" à 0 

  //Configuration des entrée et sortie 
  pinMode(3, INPUT_PULLUP);//initialisation de la pin pour le bouton poussoir lié au changement du contenu de l'affichage 7 segments en INPUT_PULLUP
  pinMode(4, INPUT_PULLUP);//initialisation de la pin pour le bouton poussoir lié au buzzer en INPUT_PULLUP
  pinMode(5, INPUT_PULLUP);//initialisation de la pin pour le bouton poussoir lié au changement du contenu de l'option d'alarme en INPUT_PULLUP
  phase = 1;//initialisation de la variable de phase a 1
  timeToWait = WAITING_TIME;//initialisation de la variable "timeToWait" en la rendant correspondant à la durée de chaque alternance d'affichage 

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

  //Fonction qui étain les LED
  turnOffClockLED();
  
  //Si le capteur BME280 et/ou la RTC n'est pas trouvé alors affiche un texte l'indiquant et arrête le programme
  if (bmeStatus != 1 || rtcStatus != 1 || sgpStatus != 1) {
    Serial.println("Problème de branchement");
    exit(0);//arrêt complet du programme
  }
}


void loop() {
  fallingEdgeDetection();//Appel de la fonction pour vérifier l'état des deux boutons poussoir

  //Cette condition permet de verifier le taux de CO2 tant qu'aucune alarme n'est générée 
  if(millis() > nextAlarm) {
    alarmDetection();//Appel de la function qui permet d'alerter si le taux de CO2 est trop élevé
  }
  
  //cette condition permet, dans le cas ou le programme est dans la phase une, d'alterner les 3 affichages
  if(phase == 1){
   
    //Permet de faire varier le contenu de l'affichage 7 segments suivant la valeur de la variable "phase"
    if(millis() > timeToWait)
    {
      timeToWait = millis() + WAITING_TIME;//A noté que la variable timeToWait est de type unsigned donc peut stock au maximum 65000 caracte contrairement a la fonction millis() qui elle ne s'arretera jms de grandis dans qu'il n'y a pas d'interuption du programme 
    
      displayChoice++;
            
      //Une fois que la variable "displayChoice" a dépassé 2 alors elle repasse à 0
      if(displayChoice > 2){
        displayChoice = 0;
      }
    }else{
        timeToWait--;
      }
  }
  
  
  //Ce switch case permet de faire varier le contenu de l'affichage 7 segments suivant la valeur de la variable "displayChoice"
  switch(displayChoice){
  
  case 0:
      displayTemperature();//Appel de la fonction permettant d'afficher la température sur l'affichage 7 segments
    break;
  
  case 1:
      displayCurrentTime();//Appel de la fonction permettant d'afficher l'heure sur l'affichage 7 segments
    break;

  case 2:
      displayCOTwoRate();//Appel de la fonction permettant d'afficher le taux de CO2 sur l'affichage 7 segments;
    break;
    
  default:
      displayTemperature();//Appel de la fonction permettant d'afficher la température sur l'affichage 7 segments
    break;
  }
}


/*
 * Description : Cette fonction permet de changer la veleur de certaine variable selon l'état de chaque bouton poussoir.
 * Elle empêche aussi l'utilisateur de changer les valeurs en restant appuyé
 * @return Retourne une valeur de type integer correspondant à l'état du bouton lié au changement du contenu de l'affichage 7 segments
 */
 int fallingEdgeDetection()
 {
    stateAlertButton = digitalRead(5);//Lecture de la pin 5

    //Incrémente la variable "alertChoice" à chaque clique du bouton poussoir "stateAlertButton"
    if(stateAlertButton == 0 && stateAlertButton == reverseStateAlertButton){
      //delay(150);//Ce délai permet de bloquer le fait d'appuyer en permanence sur le bouton
        alertChoice++;

        //si "alertChoice" a atteint plus que 3 comme valeur alors la remet à 0
        if(alertChoice > 3){
          alertChoice = 0;
          }      
          
    }
    reverseStateAlertButton = !stateAlertButton;  


    //Partie liée au bouton poussoir permettant de changer le contenu de l'affichage 7 segments
    stateDisplayButton = digitalRead(3);//Lecture de la pin 3
    
    if(phase == 1 && stateDisplayButton == 0 && stateDisplayButton == reverseStateDisplayButton){

      displayChoice = -1;
      phase = 2;
      }
    
    //Incrémente la variable "displayChoice" à chaque clique du bouton poussoir "stateDisplayButton"
    if(stateDisplayButton == 0 && stateDisplayButton == reverseStateDisplayButton){        
        
        displayChoice++;
        
        //si "displayChoice" a atteint plus que 2 comme valeur alors la remet à 0 et retourne dans la première phase
        if(displayChoice > 2){
          phase = 1;
          displayChoice = 0;
          timeToWait = millis() + WAITING_TIME;//Permet que l'affichage lié à la valeur de la variable "displayChoice" numéro 0 puisse durer le temps voulu complet
          }
    }
    
    reverseStateDisplayButton = !stateDisplayButton;
    return stateDisplayButton;
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
  doublePoints = false;//De base les double point des secondes sont éteintes
  unsigned long doublePointDuration = millis() + 200;//Variable indiquant la durée ou les doubles points des secondes doivent rester allumée

  backPackDisplay.print(getCurrentTime());//Prépare l'affichage de l'horraire actuel sur l'affichage 7 segments

  //ajoute un 0 au digit précédent dans le cas ou la valeur des heures et/ou des minutes sont en dessous de 10
  if (hours < 10) {
    backPackDisplay.writeDigitNum(0, 0);//ajoute un 0 avant l'affichage des heures
    if (minutes < 10) {
      backPackDisplay.writeDigitNum(2, 0);//ajoute un 0 avant l'affichage des minutes
    }
  }


  //cette boucle while permet d'allumer les doubles points des secondes chaque seconde durant un certain temps
  if(seconds - lastSecond != 0){
        while(millis() < doublePointDuration){
        doublePoints = true;//Inverse la valeur de la variable "doublePoints" pour l'affichage des deux petits points 
        backPackDisplay.drawColon(doublePoints);//Prépare l'affichage ou non des deux petits point selon la valeur de la variable "doublePoints"
        backPackDisplay.writeDisplay();//Affiche toutes les informations préparées au préalable sur l'affichage 7 segments
        lastSecond = seconds;
        if(millis() == doublePointDuration){
           break;  
        }
    }
  }

  backPackDisplay.drawColon(doublePoints);//Prépare l'affichage ou non des deux petits point selon la valeur de la variable "doublePoints"
  backPackDisplay.writeDisplay();//Affiche toutes les informations préparées au préalable sur l'affichage 7 segments
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
  
  backPackDisplay.print(getTempCelsius());//Prépare l'affichage de la température mesurée par le capteur BME280 sur l'affichage 7 segments
  backPackDisplay.writeDisplay();//Affiche toutes les informations préparées au préalable sur l'affichage 7 segments
}


/*
 * Description: Retourne le taux de CO2 en ppm (part per million).
 * Note : Le capteur a besoin d'environ 15 secondes avant qu'il puisse donner la vraie valeur du taux de CO2, durant les 15 premières secondes il retourne constamment la valeur 400
 * @return une valeur type float correspondant au taux de CO2 mesuré par le capteur SGP30
*/
float getCOTwoRate(){ 

   //Effectue régulièrement un calibrage du capteur SGP30
   if(calibrationCounter == 100){
    calibrationCounter = 0;

    Serial.println("calibration");
    
    uint16_t TVOC_base, eCO2_base;
    sgp.getIAQBaseline(&eCO2_base, &TVOC_base);
   }
   
    calibrationCounter++;
    
    sgp.IAQmeasure();//demande au capteur de mesurer une seul mesure de eCO2 et de VOC. Place ensuite les mesures dans TVOC et eCO2
   
   //Cette condition permet de bloquer la valeur à 9999 dans le cas où le capteur aurait mesuré une valeur supérieure à 9999
   if(sgp.eCO2 > 9999){
    return 9999; //retourne 9999 comme valeur
    }


   return sgp.eCO2; //retourne le taux de CO2 en ppm
}


/*
 * Description: Suivant certaines conditions, cette fonction appellera une ou plusieurs alertes dans le cas ou le taux de CO2 dépasse un seuil voulu et que le délai de pause de l'alarme est égale à 0
*/
void alarmDetection()
{
  sgp.IAQmeasure();//demande au capteur de mesurer une seul mesure de eCO2 et de VOC. Place ensuite les mesures dans TVOC et eCO2
  unsigned long alarmDuration = millis() + ALARME_DURATION;//Indique la durée de l'alarme
  unsigned long delayAlarm = 0;//remise à 0 de la variable correspondant à durée de "pause" de l'alarme
  
  //Tant que le taux de CO2 mesuré dépasse la valeur de la constante "MAX_CO2_ALLOWED" et qu'il n'a pas dépassé un certain temps il effectue une alarme spécifique
  while(sgp.eCO2 >= MAX_CO2_ALLOWED && millis() < alarmDuration){
    
      //Ce switch case permet de séléctionner l'alarme en cas de trop haut taux de CO2 suivant la valeur de la variable "alertChoice"
      switch(alertChoice){
      case 0:
            doubleAlert();//Appel de la fonction qui effectue l'alarme visuelle et l'alarme sonore en simultané
          break;
      
      case 1:
            visualAlertForCOTwoRate();//Appel de la fonction qui effectue une alarme visuelle
        break;

      case 2:
            sonorAlertForCOTwoRate();//Appel de la fonction qui effectue une alarme sonore
        break;

      case 3:
            Serial.println("Aucune alerte");
        break;
                
      default:
          doubleAlert();//Appel de la fonction qui effectue l'alarme visuelle et l'alarme sonore en simultané
        break;
      }

      sgp.IAQmeasure();//Reprise des mesures

      
      if(delayAlarm == 0) {
        delayAlarm = DELAY_ALARME_DURATION;
      }
  }
  nextAlarm = millis() + delayAlarm;
}


/*
 * Description: Cette fonction affichage sur l'affichage 7 segments le taux de CO2 mesurée par le capteur SGP30 
*/
void displayCOTwoRate(){
  backPackDisplay.print(getCOTwoRate());//Prépare l'affichage du taux de CO2 sur l'affichage 7 segments
  delay(25);//ajout d'un délai pour avoir un affichage plus agréable est ne pas rendre l'affichage 7 segments illisible à cause de la trop grande variation de prise de mesure du taux de CO2 par le capteur SGP30
  backPackDisplay.writeDisplay();//Affiche tous ce qui a été préparé 

}


/*
 * Description: Cette fonction affiche l'heure sur l'horloge 60 LED
 * IMPORTANT : Cette fonction n'est pas fonctionnelle dans le sens ou elle ne permet pas d'afficher l'heure comme prévu mais allume juste chaque seconde une LED en vert
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
 * Description: Cette fonction étain chaque LED de l'horloge  
*/
void turnOffClockLED(){
  
  //initialise chaque LED à 0 donc sans couleurs
  for(int i=0; i<NUMPIXELS; i++) {
    ledClock.setPixelColor(i, ledClock.Color(0, 0, 0));
  }
  ledClock.show();//Affiche les couleurs précédemment initialisées  
  
}


/*
 * Description: Cette fonction fait clignoter les 60 LED de l'horloge 60 LED, elle servira comme alerte visuelle en cas de trop haut taux de CO2
*/
void visualAlertForCOTwoRate(){

  unsigned long nextDisplay = millis() + 100;
  
  //cette boucle while permet d'allumer chaque LED et de produire du son avec le buzzer durant 100 millisecondes
  while(millis() < nextDisplay){
    displayCOTwoRate();
      for(int i=0; i<NUMPIXELS; i++) {
    ledClock.setPixelColor(i, ledClock.Color(150, 0, 0));
  }
  ledClock.show();//Affiche les couleurs précédemment initialisées

    if(millis() == nextDisplay){
        break;  
      }
  }

  unsigned long secondNextDisplay = nextDisplay + 100;//Variable indiquant la durée ou les double points des secondes doivent rester allumés ou éteints

  //cette boucle while permet d'allumer chaque LED et de produire du son avec le buzzer durant 100 millisecondes
  while(millis() < secondNextDisplay){
    displayCOTwoRate();//Appel de la fonction permettant d'afficher le taux de CO2
      for(int i=0; i<NUMPIXELS; i++) {
    ledClock.setPixelColor(i, ledClock.Color(0, 0, 0));
  }
  ledClock.show();//Affiche les couleurs précédemment initialisées

    if(millis() >= secondNextDisplay){
        break;  
      }
  }
}


/*
 * Description: Cette fonction fait sonner le buzzer en alternance, elle servira comme alerte sonore en cas de trop haut taux de CO2
*/
void sonorAlertForCOTwoRate(){
  unsigned long nextDisplay = millis() + 100;
  
  //cette boucle while permet d'allumer chaque LED et de produire du son avec le buzzer durant 100 millisecondes
  while(millis() < nextDisplay){
    displayCOTwoRate();//Appel de la fonction permettant d'afficher le taux de CO2
     tone(4, 100); //Envoie 100 KHz de signal de son

    if(millis() >= nextDisplay){
        break;  
      }
  }

  unsigned long secondNextDisplay = nextDisplay + 100;
  
  //cette boucle while permet d'allumer chaque LED et de produire du son avec le buzzer durant 100 millisecondes
  while(millis() < secondNextDisplay){
    displayCOTwoRate();//Appel de la fonction permettant d'afficher le taux de CO2
    noTone(4); //Stop le buzzer

    if(millis() >= secondNextDisplay){
        break;  
      }
  }
  
}


/*
 * Description: Cette fonction fait clignoter les 60 LED de l'horloge 60 LED et faire "clignoter" le buzzer avec du son, elle servira comme alerte visuelle et sonore en cas de trop haut taux de CO2
*/
void doubleAlert(){
  unsigned long nextDisplay = millis() + 100;
  
  //cette boucle while permet d'allumer chaque LED et de produire du son avec le buzzer durant 100 millisecondes
  while(millis() < nextDisplay){
    displayCOTwoRate();//Appel de la fonction permettant d'afficher le taux de CO2
    
    //initialise chaque LED avec la couleur rouge
    for(int i=0; i<NUMPIXELS; i++) {
      ledClock.setPixelColor(i, ledClock.Color(150, 0, 0));
    }
    ledClock.show();//Affiche les couleurs précédemment initialisées
    tone(4, 100); //Envoie 100 KHz de signal de son
    if(millis() == nextDisplay){
        break;  
      }
  }
  
  unsigned long secondNextDisplay = nextDisplay + 100;
  
  //cette boucle while permet d'éteindre chaque LED et d'éteindre le buzzer durant 100 millisecondes
  while(millis() < secondNextDisplay){
  displayCOTwoRate();//Appel de la fonction permettant d'afficher le taux de CO2
    
    //initialise chaque LED à 0 donc éteinte
    for(int i=0; i<NUMPIXELS; i++) {
      ledClock.setPixelColor(i, ledClock.Color(0, 0, 0));
      
    }
    ledClock.show();//Affiche les couleurs précédemment initialisées
    noTone(4);//Stop le buzzer
    if(millis() >= secondNextDisplay){
        break;  
      }
  }
  }
