#define SerialMon Serial
#define SerialAT SerialGSM
#define TINY_GSM_MODEM_UBLOX

#include "HX711.h"                                                        // Per gestire la bilancia
#include <RTCZero.h>                                                      // Par gestire la sleep mode
#include <TinyGsmClient.h>                                                // Per gestire il modem GSM
#include <PubSubClient.h>                                                 // Per pubblicare messaggi via MQTT

#define ADAFRUIT_USERNAME  "stefanosemeria"                               // Credenziali broker MQTT Adafruit
#define AIO_KEY  "a0254d03dfdf4f35bbfd65874fc7b912"
#define FEED_PATH_PESO "stefanosemeria/feeds/Peso"                        // Feed peso
#define FEED_PATH_BATTERIA "stefanosemeria/feeds/Batteria"                // Feed stato batteria
#define FEED_PATH_POSIZIONE "stefanosemeria/feeds/Posizione/csv"          // Feed posizione GPS
#define FEED_DEBUG "stefanosemeria/feeds/Debug"                           // Feed debug

RTCZero rtc;
HX711 scale;

//const char apn[]  = "internet.wind";                            // Credenziali modem GSM ---------------------------------------------------------------------------------
const char apn[] = "iliad";
const char user[] = "";
const char pass[] = "";

const char* broker = "io.adafruit.com";                         // Broker MQTT


const byte secondi_default = 0;                                 //
const byte minuti_default = 40;                                 //     Questa è l'ora a cui mandare i dati  ------------------------------------------------------------------
const byte ore_default = 18;                                     // ----------------------------------------------------------------------------------------------------------

double latitudine_casa = 0.0;                                   // Coordinate in cui si trova di solito l'arnia
double longitudine_casa = 0.0;
bool casa_trovata;                                              // Controllo se ho trovato le coordinate in cui si trova di solito l'arnia

double latitud;                                                 // Coordinate in cui si trova l'arnia effettivamente
double longitud;

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

PubSubClient mqtt(broker, 1883, client);



void setup() {

  SerialAT.begin(9600);                                         // Trasmissione seriale
  Serial1.begin(9600);

  delay(5000);

  pinMode(A3, INPUT);

  mqtt_init();                                                  // Connetto alla rete GPRS e al broker mqtt


  /*trova_casa();


  rtc.begin();                                                  // Inizializzo l' RTC interno

  String stringa = get_stringa();                               // Imposto l' ora con i dati del GPS
  int ora = orario_ore(stringa);
  int minuti = orario_minuti(stringa);
  int secondi = orario_secondi(stringa);
  
  unsigned long inizio_orario = millis();
  while((ora == 99 || minuti == 99 || secondi == 99) && millis()-inizio_orario < 600000){       // Aspetto dati validi dal GPS
    stringa = get_stringa();
    ora = orario_ore(stringa);
    minuti = orario_minuti(stringa);
    secondi = orario_secondi(stringa);
  }

  
  if (millis()-inizio_orario >= 600000){                        // Se i dati non sono validi imposto un'ora a caso che verrà corretta in seguito
    rtc.setTime(byte(1), byte(0), byte(0));
    mqtt.publish(FEED_DEBUG, "Errore impostazione ora");
  } else  {
    rtc.setTime(byte(ora), byte(minuti), byte(secondi));        // Se i dati sono validi imposto l' ora
    mqtt.publish(FEED_DEBUG, String("Impostata ora: "+String(ora)+" h, "+String(minuti)+" min").c_str());
  }

  delay(15000);


*/
  SerialMon.println("Initializing the scale");

  scale.begin(A1, A0);                                            // HX711.DOUT  - pin #A1            HX711.PD_SCK - pin #A0

  scale.set_scale(-18200);                                      // Calibro la scala (vedere il file README della libreria HX711) 
  scale.tare();				                                          // Taro la scala




}

void loop() {

  
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(20), 1);


  delay(2000);

  float peso = abs(scale.get_units(20));                             // Medio 20 letture per avere un peso affidabile

  while(!mqtt.connected()){
    mqttConnect();
  }

  mqtt.publish(FEED_PATH_PESO, String(peso+100).c_str());          // Pubblico il peso        --------------------------------------------------------- togliere +100
  float batteria = livello_batteria();
  mqtt.publish(FEED_PATH_BATTERIA, String(batteria).c_str());       // Pubblico lo stato della batteria in percentuale
  mqtt.publish(FEED_DEBUG, String(batteria).c_str());
  mqtt.publish(FEED_DEBUG, String(peso).c_str());


  if (!casa_trovata){
    trova_casa();
  }

  String posizione = GPS(3);                                        // Controllo se la posizione GPS è affidabile
  unsigned long inizio_GPS = millis();
  while(posizione == "0.000000000000000,0.000000000000000" && millis()-inizio_GPS < 600000){
    posizione = GPS(3);
    latitud = GPS(1).toDouble();
    longitud = GPS(2).toDouble();
  }

  if (millis()-inizio_GPS < 600000){                                                // Se la posizione è affidabile la pubblico
    mqtt.publish(FEED_PATH_POSIZIONE, String("1,"+posizione+",2").c_str());         // Se l'arnia è stata spostata chiamo Allarme_GPS()
    if(abs(latitudine_casa-latitud)>0.001 || abs(longitudine_casa-longitud)>0.001){allarme_GPS();}             // Controllare soglia minima!!
  } else {
    Serial.println("Errore gps");
    mqtt.publish(FEED_DEBUG, "Errore gps");
    delay(10000);
  }

  delay(5000);



  String stringa = get_stringa();                                   // Ripeto l'impostazione dell'ora per correggere eventuali errori
  int ora = orario_ore(stringa);
  int minuti = orario_minuti(stringa);
  int secondi = orario_secondi(stringa);
  
  unsigned long inizio_orario = millis();
  while((ora == 99 || minuti == 99 || secondi == 99) && millis()-inizio_orario < 300000){
    stringa = get_stringa();
    ora = orario_ore(stringa);
    minuti = orario_minuti(stringa);
    secondi = orario_secondi(stringa);
  }

  if (millis()-inizio_orario < 600000){                             // Se l'ora è affidabile la imposto altrimenti non faccio nulla
    rtc.setTime(byte(ora), byte(minuti), byte(secondi));
    mqtt.publish(FEED_DEBUG, String("Impostata ora: "+String(ora)+" h, "+String(minuti)+" min").c_str());
  } else {
    mqtt.publish(FEED_DEBUG, "Errore impostazione ora");
  }

  spegni_tutto(0, 0, 0);                                            // Deep sleep fino a domani alle 4:30:00 (default)
}

