    /**
   Pre-TPI
   Nom : Station Météo
   Version initial par: Esteban.GIORGIS
   Version initial créé le: 12.05.2022
   Dernière version le: 12.05.2022
**/

#include <Adafruit_BME280.h>//librairie pour le capteur BME280
#include <LiquidCrystal_I2C.h>//librairie pour l'affichage LCD
#include <RTClib.h>//librairie pour la RTC


Adafruit_BME280 bme;//initialisation du bme
LiquidCrystal_I2C lcd(0x27, 16, 2); //régle l'adresse du LCD à 0x27 pour un affichage de 16 caractères et 2 lignes.
RTC_DS1307 rtc;//initialisation de la RTC

//constantes//
const int arrayMaxSize = 5;//constante pour la taille max des tableaux et du nombre de donnée que l'on veut mesurer avant de calculer et aficher la moyenne
const unsigned delayTime = 1500;//constante du delai en millisecondes


//variables et tableaux//
unsigned bmeStatus;
unsigned rtcStatus;
int countValues = 1;
int buttonState = 1;
float tempArray[arrayMaxSize];
float pressureArray[arrayMaxSize];
float humidityArray[arrayMaxSize];

/*
 * Fonction setup
*/
void setup() {
  Serial.begin(9600);//Ouvre le port série
  bmeStatus = bme.begin();//initialisation du capteur BME, protocole de communication: I2C
  rtcStatus = rtc.begin();//initialisation du port de communication pour la RTC
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//recupère la date et le temps indiqué par notre propre ordinateur
  lcd.init();//initialisation de l'affichage LCD
  pinMode(5, INPUT_PULLUP);//initialisation de la pin 2 en INPUT_PULLUP
  
  reset();//reinitialise toutes les variables et tableaux
  
  //Si le capteur BME280 n'est pas trouvé alors affiche un text l'indiquant et arrête le programme
  if (bmeStatus != 1 || rtcStatus != 1) {
    Serial.println("Problème de branchage");
    exit(0);//arrêt complet du programme
  }
}


/*
 * Fonction loop
*/
void loop() {
  lcd.clear();//Remmet l'affichage LCD à 0
  buttonState = digitalRead(5);//Lecture de l'état du bouton
  Serial.println(buttonState);
  
  //cette condition permet d'afficher les moyennes dans le terminal et dans l'affichage LCD au bout de 5 prises de mesures
  if (countValues % arrayMaxSize == 0) {
    valuesDisplayed();
    delay(delayTime);
    avgValuesDisplayed();
    delay(delayTime);
    minValuesDisplayed();
    delay(delayTime);
    maxValuesDisplayed();
    delay(delayTime);
    reset();
  }
  
  valuesDisplayed();
  delay(delayTime);
  countValues++;//incrémentation de countValues
}

/*
 * Permet de calculer l'heure actuelle en heure minutes et seconde
 * @return une valeur de type char qui correspond a l'heure actuelle 
*/
char* getCurrentTime(){
  char* currentTime;
  currentTime = (char*)malloc(20);//malloc pour memory allocation, permet, comme son nom peut l'indiquer, de détérminer manuellement le nombre de bit pour cette variable
    
   
   DateTime now = rtc.now();
   sprintf(currentTime,"%d:%d:%d |",now.hour(),now.minute(),now.second());//permmet de "formatter" le contenu de la variable ". Attetion ! sprintf ne permet pas d'afficher du contenu dans le terminal
   return currentTime;
}

/*
 * Fonction qui retourne la température en degrés celsius "°C"
 * @return une valeur type float correspondant à la valeur de la température en degrés celsius "°C"
*/
float getTempCelsius(){ 
  return bme.readTemperature();
}


/*
 * Fonction qui retourne la température en degrés fahrenheit "°F" avec une convertion à partir de la valeur en degré celsius 
 * @return une valeur type float correspondant à la valeur de la température en degrés fahrenheit "°F"  
*/
float getTempFahrenheit(){
  return bme.readTemperature() * 9/5 + 32;
}


/*
 * Fonction qui retourne la préssion atmosphérique en hectopascal "hPa"
 * @return une valeur type float correspondant à la valeur de la préssion atmosphérique en hectopascal "hPa"
*/
float getPressure(){
  return bme.readPressure() / 100;//on divise par 100 pour obtenir la valeur en hPa au lieu de Pa
}

/*
 * Fonction qui retourne l'humidité en pourcent "%"
 * @return une valeur type float correspondant au taux d'humidité en pourcent "%"
*/
float getHumidity(){
  return bme.readHumidity();
}

/*
 * Cette fonction retourne une moyenne de toutes les valeures du tableau reçu
 * @param float Array[] tableau qui contient les dernières valeurs mesurées
 * @return une valeur float correspondant à la moyenne de toutes les valeures du tableau reçu
*/
float getAvgValues(float Array[]){ 
  float average = 0;
  for(int i = 1; i <= countValues ; i++){
    average += Array[i];
    
    //si i atteint la valeur du nombre total de données que l'on veut alors effectue la division final pour obtenir la moyenne
    if(i == countValues){
      average = average / countValues;
    }
  }
  return average;
}


/*
 * Cette fonction retourne la valeur minimum de toutes les valeures du tableau reçu
 * @param float Array[] tableau qui contient les dernières valeurs mesurées
 * @return une valeur float correspondant à la valeur minimum de toutes les valeures du tableau reçu
*/
float getMinValue(float Array[]){ 
  float minimumVal = Array[1];

  for(int i = 1; i <= countValues ; i++){
    if(minimumVal > Array[i]){
      minimumVal = Array[i];
    }
  }
  return minimumVal;
}


/*
 * Cette fonction retourne la valeur maximum de toutes les valeures du tableau reçu
 * @param float Array[] tableau qui contient les dernières valeurs mesurées
 * @return une valeur float correspondant à la valeur maximum de toutes les valeures du tableau reçu
*/
float getMaxValue(float Array[]){ 
  float maximumVal = 0;

  for(int i = 1; i <= countValues ; i++){
    if(maximumVal < Array[i]){
      maximumVal = Array[i];
    }
  }
  return maximumVal;
}



/*
 * Cette fonction sert à la mise en forme et à l'affichage des données reçu par les capteurs
*/
void valuesDisplayed() {

  //Affichage des mesures dans le terminal
  Serial.println("Temps | Température | Pression | Humidité");
  
  Serial.print(getCurrentTime());//affiche l'heure actuelle
  
  //cette condiftion affiche la température avec une unité différente selon si le bouton poussoir est préssé ou non
  if(buttonState == 1){
    Serial.print(String(getTempCelsius()) + String(" °C | "));
  }else{
    Serial.print(String(getTempFahrenheit()) + String(" °F | "));
  }
  tempArray[countValues] = getTempCelsius();
  Serial.print(String(getPressure()) + String(" hPa | "));
  pressureArray[countValues] = getPressure();
  Serial.println(String(getHumidity()) + String(" %"));
  humidityArray[countValues] = getHumidity();

  
  //affichage des dernières données mesurées dans l'affichage LCD
  //cette condiftion affiche la température avec une unité différente selon si le bouton poussoir est pressé ou non
  if(buttonState == 1){
    lcd.print(String(getTempCelsius()) + ((char)223) + String("C"));//"((char)223)" sert à indiquer le petit du signe pour les degrés "°"
  }else{
    lcd.print(String(getTempFahrenheit()) + ((char)223) + String("F"));//"((char)223)" sert à indiquer le petit du signe pour les degrés "°"
  }
  lcd.setCursor(0, 1);//place le curseur à la colonne 0, ligne 1 (retour à la ligne)
  lcd.print(String(getPressure()) + String("hPa "));
  lcd.print(String(getHumidity()) + String("%"));

}

/*
 * Cette fonction sert à afficher les moyennes des différentes mesures
*/
void avgValuesDisplayed() {

  //variables//
  float tempAvg = getAvgValues(tempArray);
  float pressureAvg = getAvgValues(pressureArray);
  float humidityAvg = getAvgValues(humidityArray);
  
  //Si le boutton est pressé alors tempAvg est convertit en degrés fahrenheit
  if(buttonState == 0){
    tempAvg = (getAvgValues(tempArray)) * 9/5 + 32; 
  }
  
  //Affichage des moyennes dans le terminal
  Serial.println();
  Serial.print("Moyennes: ");
  
  //cette condiftion affiche la température avec une unité différente selon si le bouton poussoir est pressé ou non
  if(buttonState == 1){
    Serial.print(String(tempAvg) + String(" °C | "));
  }else{
    Serial.print(String(tempAvg) + String(" °F | "));
  }
  
  Serial.print(String(pressureAvg) + String(" hPa | "));
  Serial.println(String(humidityAvg) + String(" %"));
  Serial.println();
  
  //Affichage des moyennes dans l'affichage LCD
  lcd.clear();//Remmet l'affichage LCD à 0
  lcd.print("Moyennes:");

  //cette condiftion affiche la température avec une unité différente selon si le bouton poussoir est pressé ou non
  if(buttonState == 1){
    lcd.print(String(tempAvg) + ((char)223) + String("C"));//"((char)223)" sert à indiquer le petit du signe pour les degrés "°"
  }else{
    lcd.print(String(tempAvg) + ((char)223) + String("F"));//"((char)223)" sert à indiquer le petit du signe pour les degrés "°"
  }
  lcd.setCursor(0, 1);
  lcd.print(String(pressureAvg) + String("hPa "));
  lcd.print(String(humidityAvg) + String("%"));
  

}


/*
 * Cette fonction sert à afficher la valeurs minumum des différentes dernières mesures
*/

void minValuesDisplayed() {
  //variables//
  float tempMin = getMinValue(tempArray);
  float pressureMin = getMinValue(pressureArray);
  float humidityMin = getMinValue(humidityArray);

  //Si le boutton est pressé alors tempMax est convertit en degrés fahrenheit
  if(buttonState == 0){
    tempMin = (getMinValue(tempArray)) * 9/5 + 32; 
  }
  
  //Affichage des valeurs minimum dans le terminal
  Serial.println();
  Serial.print("Minimum: ");

  //cette condition affiche la température avec une unité différente selon si le bouton poussoir est pressé ou non
  if(buttonState == 1){
    Serial.print(String(tempMin) + String(" °C | "));
  }else{
    Serial.print(String(tempMin) + String(" °F | "));
  }

  Serial.print(String(pressureMin) + String(" hPa | "));
  Serial.println(String(humidityMin) + String(" %"));
  Serial.println();


  //Affichage des valeurs minimum dans l'affichage LCD
  lcd.clear();//Remmet l'affichage LCD à 0
  lcd.print("Minimum:");

  //cette condiftion affiche la température avec une unité différente selon si le bouton poussoir est pressé ou non
  if(buttonState == 1){
    lcd.print(String(tempMin) + ((char)223) + String("C"));//"((char)223)" sert à indiquer le petit du signe pour les degrés "°"
  }else{
    lcd.print(String(tempMin) + ((char)223) + String("F"));//"((char)223)" sert à indiquer le petit du signe pour les degrés "°"
  }
  lcd.setCursor(0, 1);
  lcd.print(String(pressureMin) + String("hPa "));
  lcd.print(String(humidityMin) + String("%"));
}


/*
 * Cette fonction sert à afficher la valeurs maximum des différentes dernières mesures
*/

void maxValuesDisplayed() {
  //variables//
  float tempMax = getMaxValue(tempArray);
  float pressureMax = getMaxValue(pressureArray);
  float humidityMax = getMaxValue(humidityArray);

  //Si le boutton est pressé alors tempMax est convertit en degrés fahrenheit
  if(buttonState == 0){
    tempMax = (getMaxValue(tempArray)) * 9/5 + 32; 
  }
  
  //Affichage des valeurs maximum dans le terminal
  Serial.println();
  Serial.print("Maximum: ");

  //cette condition affiche la température avec une unité différente selon si le bouton poussoir est pressé ou non
  if(buttonState == 1){
    Serial.print(String(tempMax) + String(" °C | "));
  }else{
    Serial.print(String(tempMax) + String(" °F | "));
  }

  Serial.print(String(pressureMax) + String(" hPa | "));
  Serial.println(String(humidityMax) + String(" %"));
  Serial.println();


  //Affichage des valeurs minimum dans l'affichage LCD
  lcd.clear();//Remmet l'affichage LCD à 0
  lcd.print("Maximum:");

  //cette condition affiche la température avec une unité différente selon si le bouton poussoir est pressé ou non
  if(buttonState == 1){
    lcd.print(String(tempMax) + ((char)223) + String("C"));//"((char)223)" sert à indiquer le petit du signe pour les degrés "°"
  }else{
    lcd.print(String(tempMax) + ((char)223) + String("F"));//"((char)223)" sert à indiquer le petit du signe pour les degrés "°"
  }
  lcd.setCursor(0, 1);
  lcd.print(String(pressureMax) + String("hPa "));
  lcd.print(String(humidityMax) + String("%"));
}


/*
 * Cette fonction sert à réinitialiser toutes sortes de choses, comme les variables/tableau ou encore l'affichage LCD
*/
void reset() {
    countValues = 1;
    memset(tempArray, 0, sizeof(tempArray));//Vide le tableau tempArray
    memset(pressureArray, 0, sizeof(pressureArray));//Vide le tableau pressureArray
    memset(humidityArray, 0, sizeof(humidityArray));//Vide le tableau humidityArray
    lcd.clear();//Remmet l'affichage LCD à 0
}
