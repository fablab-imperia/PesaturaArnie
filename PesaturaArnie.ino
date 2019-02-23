/*
A0 --> SCK
A1 --> DD
*/

#define SerialMon Serial
#define SerialAT SerialGSM
#define TINY_GSM_MODEM_UBLOX
#define DEBUG_PORT Serial
#define DEBUG
#define SCAN
//#define RAM

//#include <TinyGsmClient.h>                                      // Per gestire il modem GSM
#include <MKRGSM.h>
#include "HX711.h"                                              // Per gestire la bilancia
#include <RTCZero.h>                                            // Par gestire l'RTC interno
#include <PubSubClient.h>                                       // Per pubblicare messaggi via MQTT
#include <NMEAGPS.h>                                            // Per il GPS
//#include <MemoryFree.h>;

#include <SPI.h>
#include <SD.h>


#define gpsPort Serial1




//Stampiamo usando debug print per accendere e spegnere le stampe all'occorenza


#ifdef DEBUG_SD
 #define DEBUG_PRINT(x)             log_debug(x, false)
// #define DEBUG_PRINTDEC(x)          Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)           log_debug(x, true)
 #define DEBUG_PRINT_MOBILE(x, y)   Serial.print (x, y)
 #define DEBUG_PRINTLN_MOBILE(x, y) Serial.println (x, y)
 #define SVEGLIA
 #else
 #ifdef DEBUG
  #define DEBUG_PRINT(x)             Serial.print(x)
   #define DEBUG_PRINTDEC(x)          Serial.print(x, DEC)
   #define DEBUG_PRINTLN(x)           Serial.println(x)
   #define DEBUG_PRINT_MOBILE(x, y)   Serial.print(x, y)
   #define DEBUG_PRINTLN_MOBILE(x, y) Serial.println(x, y)
   #define SVEGLIA
   #else
     #define DEBUG_PRINT(x)
     #define DEBUG_PRINTDEC(x)
     #define DEBUG_PRINTLN(x)
     #define DEBUG_PRINT_MOBILE(x, y)
     #define DEBUG_PRINTLN_MOBILE(x, y)
     #endif
  #endif

/*#ifdef DEBUG
 #define DEBUG_PRINT(x)             Serial.print(x)
 #define DEBUG_PRINTDEC(x)          Serial.print(x, DEC)
 #define DEBUG_PRINTLN(x)           Serial.println(x)
 #define DEBUG_PRINT_MOBILE(x, y)   Serial.print(x, y)
 #define DEBUG_PRINTLN_MOBILE(x, y) Serial.println(x, y)
 #define SVEGLIA
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINT_MOBILE(x, y)
 #define DEBUG_PRINTLN_MOBILE(x, y)
#endif*/


// definizione FEED mqtt
const char* broker = "io.adafruit.com";                         // Broker MQTT
#define ADAFRUIT_USERNAME  "fablabimperia"                      // Credenziali broker MQTT Adafruit
#define AIO_KEY "38367dac2353481e92bff223323fe751"
#define FEED_PESO "fablabimperia/feeds/Peso"                    // Feed peso
#define FEED_BATTERIA "fablabimperia/feeds/Batteria"            // Feed stato batteria
#define FEED_POSIZIONE "fablabimperia/feeds/Posizione/csv"      // Feed posizione GPS
#define FEED_DEBUG "fablabimperia/feeds/Debug"                  // Feed debug
#define FEED_STATO "fablabimperia/feeds/Stato"                  // Feed stato complessivo
#define colore_setup "#0000FF"                                  // Colore durante il setup (blu)
#define colore_ok "#00FF00"                                     // Colore se tutto funziona correttamente (verde)
#define colore_ora_errata "#FFEE00"                             // Colore se l'ora non è corretta (giallo)
#define colore_problema_peso "#FF6600"                          // Colore se il peso è troppo alto o basso (arancione)
#define colore_batteria_bassa "#FF0000"                         // Colore se la batteria è scarica (rosso)
#define colore_allarme "#FF0000"                                // Colore se l'arnia viene spostata (rosso)
#define colore_GPS_staccato "#9300FF"                           // Colore se il GPS viene staccato (viola)#9300FF
int stato;                                                      // 0 - Setup, 1 - OK, 2 - Ora, 3 - Peso, 4 - Batteria, 5 - Gps (arnia spostata) 6 - Collegamento GPS danneggiato   (Gli stati sono in ordine di importanza crescente)



//GPS e pertinenti
#define MIN_SAT_CHECK 5                                         // Numero minimo satelliti
#define maxTimeGpsInactived 120000                               // Tempo massimo di inattivita del GPS prima dell' invio dell allarme per GPS danneggiato-----------------
#define timeOutMQTT 120000                                      // Tempo massimo connessione MQTT
#define timeOutGPRS 120000                                      // Tempo massimo connessione rete cellulare
#define tolleranza_GPS 0.005                                    // Tolleranza di errore accettabile (in gradi) prima di chiamare allarme()
int ore;                                                        //
int minuti;                                                     // orario GPS
int secondi;                                                    //
bool fix_Loc = false;                                           // FIX3D del gps
bool fix_Loc_error = false;                                     // disponibilita GPS
NMEAGPS  gps; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values


//INGRESSI DIGITALI
#define pin_interrupt 0                                         // Pin per interrupt per il risveglio
#define pin_GPS 1                                               // Pin alimentazione GPS
#define pin_buzzer 2                                            // buzzer avviso tara eseguita

//Bilancia
#define peso_basso -10                                          // Peso minimo (in kg) considerato rischioso
#define peso_alto 80                                            // Peso massimo (in kg) considerato rischioso



// Credenziali modem GSM e variabili GSM
const char apn[]  = "TM";                                     // Apn Things Mobile
//const char apn[]  = "internet.wind"                           // Apn WIND
//const char apn[] = "iliad";                                   // Apn Iliad
//const char apn[] = "ibox.tim.it"                              // Apn TIM (NON controllato)
//const char apn[] = "tre.it";                                  // Apn Tre
//const char apn[] = "mobile.vodafone.it";                      // Apn Vodafone (1)
//const char apn[] = "blackberry.net ";                         // Apn Vodafone (2)
//const char apn[] = "wap.omnitel.it";                          // Apn Vodafone (3)

const char user[] = "";
const char pass[] = "";
const char pin_card[] = "";

                                         //lat e lon della cella gsm
//double lat_GSM = 0;   //definite dentro la funzione loc_GSM
//double lon_GSM = 0;




//variabili per la gestione
const byte secondi_default = 0;                                 //
const byte minuti_default = 30;                                 //     Questa è l'ora a cui mandare i dati (UTC)
const byte ore_default = 4;                                     //

double latitudine_casa = 0.0;                                   // Coordinate in cui si trova di solito l'arnia
double longitudine_casa = 0.0;
bool casa_trovata;         

double latitud;                                                 // Coordinate in cui si trova l'arnia effettivamente
double longitud;
float altitudine;
int satelliti;                                                  // Numero satelliti utilizzati
unsigned int progressivo = 0;                                   // Progressivo ricerche GPS

volatile bool arnia_sollevata = false;

float tens = 0;


RTCZero rtc;
HX711 scale;
GSMScanner scannerNetworks;
GSMLocation location;
GSMClient client;
GPRS gprs;
GSM gsmAccess;
PubSubClient mqtt(broker, 1883, client);



void setup() {

  delay(10000);
  
  Serial.begin(9600);

  #ifdef DEBUG_SD
  Serial.println("DEBUG ATTIVO CON SD RICORDARSI DI COLLEGARE IL DATALOGGER");
  #else
    #ifdef DEBUG
      Serial.println("DEBUG ATTIVO SENZA SD ");
    #else
    Serial.println("DEBUG DISATTIVO");
    #endif
  #endif

//  delay(40000);                             ---------------------------------------------
  
  Serial.print("2");
  DEBUG_PRINTLN("Init()> Inizializzo i dispositivi");
  rtc.begin();                                                    // Inizializzo l' RTC interno
  gpsPort.begin(9600);
  scale.begin(A1, A0);                                            //HX711.DOUT  - pin #A1            HX711.PD_SCK - pin #A0

  #ifdef DEBUG_SD
  Serial.print("Inizializzazione Card: ");
 
  if (!SD.begin(4)) //il Pin 4 è collegato a CS
  {
    Serial.println("FALLITA!");
  } else {
    Serial.println("ESEGUITO!");
  }
  Pubblica(FEED_DEBUG, "SD: "+String(log_debug("Test SD...", true)));
  #endif

  pinMode(pin_GPS, OUTPUT);  
  pinMode(pin_interrupt, INPUT_PULLUP);
  pinMode(pin_buzzer, OUTPUT);
//  attachInterrupt(digitalPinToInterrupt(pin_interrupt), sollevata, FALLING);

  DEBUG_PRINT("setup()> Broker MQTT: ");
  DEBUG_PRINTLN(broker);
  
  DEBUG_PRINTLN("setup()> MQTT Init..."); 
  
  
  
  DEBUG_PRINTLN("setup()> Effettuo la tara della Bilancia");                                                       
  scale.set_scale(-18200);                                      // Calibro la scala (vedere il file README della libreria HX711) 
  scale.tare();                                                  // Taro la scala
  DEBUG_PRINTLN("setup()> Completato taratura  bilancia");
  Pubblica(FEED_DEBUG, "SETUP: Tara bilancia effettuata sistemare l'ARNIA!!");
  digitalWrite(pin_buzzer, HIGH);
  delay(300);
  digitalWrite(pin_buzzer, LOW);
  delay(1000);
  digitalWrite(pin_buzzer, HIGH);
  delay(300);
  digitalWrite(pin_buzzer, LOW);
  delay(1000);
  digitalWrite(pin_buzzer, HIGH);
  delay(300);
  digitalWrite(pin_buzzer, LOW);
  delay(1000);

  DEBUG_PRINTLN("setup()> Configuro i pin e gli Interrupt");
  delay(1000);
  
  stato = 0;
  Pubblica(FEED_STATO, colore_setup);
  Pubblica(FEED_DEBUG, "SETUP: Accensione!");
//  DEBUG_PRINT("setup()> Pubblico su FEED_STATO valore: ");
//  DEBUG_PRINTLN(colore_setup);
  
  delay(1000);  
  check_GPS();
  trova_casa();
}

void loop() {
  DEBUG_PRINTLN("Loop()> inizio loop lancio gps check");
  check_GPS();
  DEBUG_PRINT("loop()> Controllo se la casa e' stata trovata...  ");
  if (!casa_trovata){
    DEBUG_PRINTLN("No... riprovo...");
    trova_casa();
  } else {
    DEBUG_PRINTLN("Sì!");
  }

  delay(2000);

  float peso = abs(scale.get_units(20));                        // Medio 20 letture per avere un peso affidabile
  float batteria = livello_batteria();

  DEBUG_PRINT("loop()> Valore peso letto da bilancia: ");
  DEBUG_PRINTLN(String(peso));
  DEBUG_PRINT("loop()> Valore livello batteria letto: ");
  DEBUG_PRINT(batteria);
  DEBUG_PRINT("....>.... in volt");
  //DEBUG_PRINTLN(tens);
  

  DEBUG_PRINTLN("loop()> Pubblico i FEED peso e batteria");
  Pubblica(FEED_DEBUG, "Inizio Loop");
  Pubblica(FEED_PESO, String(peso));                          // Pubblico il peso
  Pubblica(FEED_BATTERIA, String(batteria));                 // Pubblico lo stato della batteria in percentuale
  Pubblica(FEED_DEBUG, ("Batt: "+String(batteria)+">"+String(tens)+"V %   Peso: "+String(peso)+" kg"));

  
  if (batteria < 30){
    DEBUG_PRINT("loop()> valore batteria basso: ");
    DEBUG_PRINT(batteria);
    DEBUG_PRINTLN(" % (< 30%)!");
    if(stato < 4){
      stato = 4;
//      DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
//      DEBUG_PRINTLN(colore_batteria_bassa);
      Pubblica(FEED_STATO, colore_batteria_bassa);
    }
  } else if (stato == 4){
    stato = 1;
//    DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
//    DEBUG_PRINTLN(colore_ok);
    Pubblica(FEED_STATO, colore_ok);
  }

  if (peso < peso_basso || peso > peso_alto){
    DEBUG_PRINT("loop()> Peso al di fuori dell'intervallo ( ");
    DEBUG_PRINT(peso_basso);
    DEBUG_PRINT(" - ");
    DEBUG_PRINT(peso_alto);
    DEBUG_PRINT(" )");

    if (stato < 3){
      stato = 3;
//      DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
//      DEBUG_PRINTLN(colore_problema_peso);
      Pubblica(FEED_STATO, colore_problema_peso);
    }
  } else if (stato == 3) {
    stato = 1;
//    DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
//    DEBUG_PRINTLN(colore_ok);
    Pubblica(FEED_STATO, colore_ok);
  }

  if (!fix_Loc_error){
    if (stato < 1){
      stato = 1;
      Pubblica(FEED_STATO, colore_ok);
    }
  }

  //|check_GPS();
  if (latitud != 0.0 || longitud != 0.0){                               // Se la posizione è affidabile la pubblico
    DEBUG_PRINTLN("loop()> La posizione GPS recuperata e' affidabile... ");
//    mqttConnect();
//    DEBUG_PRINT("loop()> Pubblico su FEED_POSIZIONE valore ");
//    DEBUG_PRINTLN(String(String(progressivo)+","+String(latitud, 15)+","+String(longitud, 15)+","+String(altitudine, 1)).c_str());
    Pubblica(FEED_POSIZIONE, String(progressivo)+","+String(latitud, 15)+","+String(longitud, 15)+","+String(altitudine, 1));

    if (((abs(latitudine_casa-latitud)>tolleranza_GPS || abs(longitudine_casa-longitud)>tolleranza_GPS) && casa_trovata) && !fix_Loc_error) {
      DEBUG_PRINTLN("loop()> ALLARME GPS!");
      allarme();                                                                                                              // Se l'arnia è stata spostata chiamo allarme()
    }
  }
  Pubblica(FEED_DEBUG, "Loop()> Dormo!!!!!");
  DEBUG_PRINTLN("loop()> Chiamo spegni tutto e avvio standby... ");
  /*#ifdef SVEGLIA
    if (rtc.getHours() == 23 && rtc.getMinutes() >= 58)
    {spegni_tutto(0, 1, byte(30), 0);}                                                      // Sleep mode per 1 minuto
    else if (rtc.getMinutes() >= 58)
    {spegni_tutto(rtc.getHours()+1, 1, byte(30), 0);}                                        // Sleep mode per 1 minuto
    else {spegni_tutto(rtc.getHours(), rtc.getMinutes() + 1, byte(30), 0);}
    
  #else
    spegni_tutto(ore_default, minuti_default, secondi_default, 1);                                          // Sleep mode fino a domani alle 4:30:00 UTC (default)
  #endif*/

  spegni_tutto(ore_default, minuti_default, secondi_default, 0);
  
  //spegni_tutto(0,0,0,0);
  DEBUG_PRINTLN("");
  DEBUG_PRINT("Loop()> Valore variabile stato:  ");
  DEBUG_PRINTLN(stato);
  riaccendi_tutto();
}
