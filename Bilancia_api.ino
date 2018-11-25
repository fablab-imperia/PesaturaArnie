#define SerialMon Serial
#define SerialAT SerialGSM
#define TINY_GSM_MODEM_UBLOX

#define DEBUG

#include "HX711.h"                                              // Per gestire la bilancia
#include <RTCZero.h>                                            // Par gestire la sleep mode
#include <TinyGsmClient.h>                                      // Per gestire il modem GSM
#include <PubSubClient.h>                                       // Per pubblicare messaggi via MQTT


//Stampiamo usando debug print per accendere e spegnere le stampe all'occorenza
#ifdef DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)   Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x) 
#endif


#define ADAFRUIT_USERNAME  "stefanosemeria"                     // Credenziali broker MQTT Adafruit
#define AIO_KEY  "a0254d03dfdf4f35bbfd65874fc7b912"

#define FEED_PESO "stefanosemeria/feeds/Peso"                   // Feed peso
#define FEED_BATTERIA "stefanosemeria/feeds/Batteria"           // Feed stato batteria
#define FEED_POSIZIONE "stefanosemeria/feeds/Posizione/csv"     // Feed posizione GPS
#define FEED_DEBUG "stefanosemeria/feeds/Debug"                 // Feed debug
#define FEED_STATO "stefanosemeria\feeds/Stato"                 // Feed stato complessivo

#define colore_ora_errata "#FFEE00"                             // Colore se l'ora non è corretta (giallo)
#define colore_ok "#00FF00"                                     // Colore se tutto funziona correttamente (verde)
#define colore_allarme_gps "#FF0000"                            // Colore se l'arnia viene spostata (rosso)
#define colore_batteria_bassa "#FF0000"                         // Colore se la batteria è scarica (rosso)
#define colore_problema_peso "#FF6600"                          // Colore se il peso è troppo alto o basso
#define colore_setup "#0000FF"                                  // Colore durante il setup (blu)

#define peso_basso 20                                           // Peso minimo (in kg) considerato rischioso
#define peso_alto 80                                            // Peso massimo (in kg) considerato rischioso

RTCZero rtc;
HX711 scale;

                                                                // Credenziali modem GSM
//const char apn[]  = "internet.wind"                           // Apn WIND
//const char apn[] = "iliad";                                   // Apn Iliad
//const char apn[] = "ibox.tim.it"                              // Apn TIM (NON controllato)
//const char apn[] = "tre.it";                                  // Apn Tre
const char apn[] = "mobile.vodafone.it";                      // Apn Vodafone (1)
//const char apn[] = "blackberry.net ";                         // Apn Vodafone (2)
//const char apn[] = "wap.omnitel.it";                          //Apn Vodafone (3)

const char user[] = "";
const char pass[] = "";

const char* broker = "io.adafruit.com";                         // Broker MQTT


const byte secondi_default = 0;                                 //
const byte minuti_default = 25;                                 //     Questa è l'ora a cui mandare i dati---------------------------------------------------------------------------------------
const byte ore_default = 18;                                     //

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

  DEBUG_PRINT("setup()> Broker MQTT: ");
  DEBUG_PRINTLN(broker);
  
  DEBUG_PRINTLN("setup()> MQTT Init...");
  mqtt_init();                                                  // Connetto alla rete GPRS e al broker mqtt
  unsigned long inizio_MQTT = millis();
  while(!mqtt.connected() && millis()-inizio_MQTT < 300000){
    mqttConnect();
    DEBUG_PRINTLN("setup()>  mqttConnect fallito: ritento...");
  }

  DEBUG_PRINTLN("setup()> Connessione MQTT riuscita...");

  mqtt.publish(FEED_STATO, colore_setup);
  DEBUG_PRINT("setup()> Pubblico su FEED_STATO valore: ");
  DEBUG_PRINTLN(colore_setup);
  mqtt.publish(FEED_DEBUG, "SETUP: Accensione!");

  DEBUG_PRINTLN("setup()> Cerco casa... ");
  trova_casa();

  DEBUG_PRINT("setup()> Inizializzo RTC su 18:24:00");
  rtc.begin();                                                  // Inizializzo l' RTC interno
  rtc.setTime(byte(18), byte(24), byte(0));

  DEBUG_PRINTLN("setup()> Recupero ora esatta da GPS...");
  String stringa = get_stringa();                               // Imposto l' ora con i dati del GPS
  DEBUG_PRINT("setup()> Stringa ora GPS ");
  DEBUG_PRINTLN(stringa);
  int ora = orario_ore(stringa);
  int minuti = orario_minuti(stringa);
  int secondi = orario_secondi(stringa);

  DEBUG_PRINT("setup()> Ora recuperata: ");
  DEBUG_PRINT(ora);
  DEBUG_PRINT(":");
  DEBUG_PRINT(minuti);
  DEBUG_PRINT(":");
  DEBUG_PRINTLN(secondi);
  
  unsigned long inizio_orario = millis();
  while((ora == 99 || minuti == 99 || secondi == 99) && millis()-inizio_orario < 600000){       // Aspetto dati validi dal GPS
    DEBUG_PRINTLN("setup()> Ora recuperata non valida... riprovo...");   
    stringa = get_stringa();
    DEBUG_PRINT("setup()> Stringa ora GPS ");
    DEBUG_PRINTLN(stringa);
    ora = orario_ore(stringa);
    minuti = orario_minuti(stringa);
    secondi = orario_secondi(stringa);
    DEBUG_PRINT("setup()> Ora recuperata: ");
    DEBUG_PRINT(ora);
    DEBUG_PRINT(":");
    DEBUG_PRINT(minuti);
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(secondi);
  }

  mqtt.publish(FEED_STATO, colore_ok);
  DEBUG_PRINT("setup()> Pubblico su FEED_STATO valore: ");
  DEBUG_PRINTLN(colore_ok);

  
  if (millis()-inizio_orario >= 600000){                        // Se i dati non sono validi imposto un'ora a caso che verrà corretta in seguito
    rtc.setTime(byte(1), byte(0), byte(0));
    DEBUG_PRINTLN("setup()> Errore impostazione ora su RTC...");
    mqtt.publish(FEED_DEBUG, "Errore impostazione ora");
    DEBUG_PRINT("setup()> Pubblico su FEED_STATO valore ");
    mqtt.publish(FEED_STATO, colore_ora_errata);
    DEBUG_PRINTLN("setup()> Pubblico su FEED_STATO valore ");
  } else  {
    rtc.setTime(byte(ora), byte(minuti), byte(secondi));        // Se i dati sono validi imposto l' ora
    mqtt.publish(FEED_DEBUG, String("Ora: "+String(ora)+" h, "+String(minuti)+" m").c_str());
  }

  delay(15000);



  DEBUG_PRINTLN("setup()> Inizializzo bilancia");

  scale.begin(A1, A0);                                          // HX711.DOUT  - pin #A1            HX711.PD_SCK - pin #A0

  scale.set_scale(-18200);                                      // Calibro la scala (vedere il file README della libreria HX711) 
  scale.tare();				                                          // Taro la scala

  DEBUG_PRINTLN("setup()> Completato taratura  bilancia");


}

void loop() {
delay(3000);
  
  Serial.print(scale.get_units(), 1);
  Serial.println(scale.get_units(20), 1);


  delay(2000);

  float peso = abs(scale.get_units(20));                        // Medio 20 letture per avere un peso affidabile

  DEBUG_PRINT("loop()> Valore peso letto da bilancia: ");
  DEBUG_PRINTLN(peso);

  
  unsigned long inizio_MQTT = millis();
  while(!mqtt.connected() && millis()-inizio_MQTT < 300000){
    mqttConnect();
    DEBUG_PRINTLN("loop()> mqttConnect fallito: ritento...");
  }


  DEBUG_PRINTLN("loop()> Connessione MQTT riuscita...");

  mqtt.publish(FEED_PESO, String(peso+100).c_str());            // Pubblico il peso--------------------------------------------------------- togliere +100
  DEBUG_PRINT("loop()> Pubblico su FEED_PESO valore: ");
  DEBUG_PRINTLN(String(peso+100).c_str());
  
  float batteria = livello_batteria();
  DEBUG_PRINT("loop()> Valore livello batteria letto: ");
  DEBUG_PRINTLN(batteria);
  
  mqtt.publish(FEED_BATTERIA, String(batteria).c_str());        // Pubblico lo stato della batteria in percentuale
  DEBUG_PRINT("loop()> Pubblico su FEED_BATTERIA valore: ");
  DEBUG_PRINTLN(String(batteria).c_str());
  
  mqtt.publish(FEED_DEBUG, ("Batt: "+String(batteria)+" %   Peso: "+String(peso)+" kg").c_str());

  
  if (batteria < 30){
    DEBUG_PRINTLN("loop()> valore batteria basso (< 30)!");

    DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
    DEBUG_PRINTLN(colore_batteria_bassa);
    mqtt.publish(FEED_STATO, colore_batteria_bassa);
  }

  if (peso < peso_basso || peso > peso_alto){
    DEBUG_PRINT("loop()> Peso al di fuori dell'intervallo ( ");
    DEBUG_PRINT(peso_basso);
    DEBUG_PRINT(" - ");
    DEBUG_PRINT(peso_alto);
    DEBUG_PRINT(" )");
    DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
    DEBUG_PRINTLN(colore_problema_peso);
    mqtt.publish(FEED_STATO, colore_problema_peso);
  }
  

  DEBUG_PRINTLN("loop()> Controllo se la casa e' stata trovata...");
  if (!casa_trovata){
    DEBUG_PRINTLN("loop()> Casa non trovata... riprovo...");
    trova_casa();
  }

  DEBUG_PRINTLN("loop()> Recupero posizione GPS attuale...");
  GPS();                                                        // Controllo se la posizione GPS è affidabile


   DEBUG_PRINT("loop()> Pubblico su FEED_POSIZIONE valore ");
   DEBUG_PRINTLN(String("1,"+String(latitud, 15)+","+String(longitud, 15)+",2").c_str());
   mqtt.publish(FEED_POSIZIONE, String("1,"+String(latitud, 15)+","+String(longitud, 15)+",2").c_str()); //--------------------------------------------------------------

  unsigned long inizio_GPS = millis();
  while((latitud == 0.0 || longitud == 0.0) && millis()-inizio_GPS < 600000){
    DEBUG_PRINTLN("loop()> Posizione GPS non valida... riprovo...");
    GPS();
  }

  if (millis()-inizio_GPS < 600000){                            // Se la posizione è affidabile la pubblico
    DEBUG_PRINT("loop()> La posizione GPS recuperata e' affidabile... ");
    DEBUG_PRINT("loop()> Pubblico su FEED_POSIZIONE valore ");
    DEBUG_PRINTLN(String("1,"+String(latitud, 15)+","+String(longitud, 15)+",2").c_str());
    mqtt.publish(FEED_POSIZIONE, String("1,"+String(latitud, 15)+","+String(longitud, 15)+",2").c_str()); // Se l'arnia è stata spostata chiamo Allarme_GPS()

    delay(10000);

    DEBUG_PRINTLN("loop()> Posizioni per calcolo allarme GPS: ");
    DEBUG_PRINT("loop()> latitudine_casa: ");
    DEBUG_PRINTLN(latitudine_casa);
    DEBUG_PRINT("loop()> latitud: ");
    DEBUG_PRINTLN(latitud);
    DEBUG_PRINT("loop()> longitudine_casa: ");
    DEBUG_PRINTLN(longitudine_casa);
    DEBUG_PRINT("loop()> longitud: ");
    DEBUG_PRINTLN(longitud);
    
    DEBUG_PRINTLN(latitudine_casa);
    if(abs(latitudine_casa-latitud)>0.005 || abs(longitudine_casa-longitud)>0.005 && casa_trovata){                       // Controllare soglia minima!!
      DEBUG_PRINTLN("loop()> ALLARME GPS!");
      allarme_GPS();
    } 
    else 
    {
       DEBUG_PRINTLN("loop()> NO ALLARME GPS!");
    }
  } else {
    DEBUG_PRINTLN("loop()> Errore gps");
    mqtt.publish(FEED_DEBUG, "Errore gps");
    delay(10000);
  }

  delay(5000);

  DEBUG_PRINTLN("setup()> Recupero ora esatta da GPS...");
  String stringa = get_stringa();                               // Imposto l' ora con i dati del GPS
  DEBUG_PRINT("setup()> Stringa ora GPS ");
  DEBUG_PRINTLN(stringa);
  int ora = orario_ore(stringa);
  int minuti = orario_minuti(stringa);
  int secondi = orario_secondi(stringa);

  DEBUG_PRINT("setup()> Ora recuperata: ");
  DEBUG_PRINT(ora);
  DEBUG_PRINT(":");
  DEBUG_PRINT(minuti);
  DEBUG_PRINT(":");
  DEBUG_PRINTLN(secondi);


  unsigned long inizio_orario = millis();
  while((ora == 99 || minuti == 99 || secondi == 99) && millis()-inizio_orario < 300000){
    DEBUG_PRINTLN("setup()> Ora recuperata non valida... riprovo...");   
    stringa = get_stringa();
    DEBUG_PRINT("setup()> Stringa ora GPS ");
    DEBUG_PRINTLN(stringa);
    ora = orario_ore(stringa);
    minuti = orario_minuti(stringa);
    secondi = orario_secondi(stringa);
    DEBUG_PRINT("setup()> Ora recuperata: ");
    DEBUG_PRINT(ora);
    DEBUG_PRINT(":");
    DEBUG_PRINT(minuti);
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(secondi);
  }

  if (millis()-inizio_orario < 600000){                         // Se l'ora è affidabile la imposto altrimenti non faccio nulla
    rtc.setTime(byte(ora), byte(minuti), byte(secondi));
    DEBUG_PRINT("Ora affidabile: setto RTC");
    mqtt.publish(FEED_DEBUG, String("Ora: "+String(ora)+" h, "+String(minuti)+" m").c_str());
    DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
    DEBUG_PRINTLN(colore_ok);
    mqtt.publish(FEED_STATO, colore_ok);
  } else {
    mqtt.publish(FEED_DEBUG, "Errore impostazione ora");
    mqtt.publish(FEED_STATO, colore_ora_errata);
  }
  
  DEBUG_PRINTLN("setup()> Chiamo spegni tutto e avvio standby... ");
  spegni_tutto(0, 0, 0);                                        // Sleep mode fino a domani alle 4:30:00 UTC (default)
}
