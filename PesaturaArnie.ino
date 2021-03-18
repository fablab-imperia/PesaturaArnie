/*
 * Scopo di questo progetto è la realizzazione di un semplice sistema per la pesatura 
 * e il monitoraggio di Arnie. Il sistema controlla il peso di un'arnia mediante bilancia
 * a celle di carico, consente un monitoraggio continuo appoggiandosi ad una piattaforma
 * Web e avvisa mediante notifica in caso di anomalia (peso troppo basso, batteria al minimo).
 *
 * Il sistema funziona mediante batteria e GPRS ed è basato sulla scheda Arduino MKR GSM 1400
 * 
 * Più informazioni nel repository gihub https://github.com/fablab-imperia/PesaturaArnie
 */

#include "secrets.h"                                            // Inserisci in questo file le credenziali dei servizi adafruit e telegram
#include <MKRGSM.h>                                             // Per gestire il modem GSM
#include "HX711.h"                                              // Per gestire la bilancia
#include <RTCZero.h>                                            // Par gestire l'RTC interno
#include <PubSubClient.h>                                       // Per pubblicare messaggi via MQTT
#include <NMEAGPS.h>                                            // Per il GPS
//#include <MemoryFree.h>;


/*////////////////////////////////////////////////////////////////
// Configura i parametri qua sotto, oltre alle credenziali nel  //
// file secret.h                                                //
////////////////////////////////////////////////////////////////*/

// Configurazione log generale
#define DEBUG                                                 // Scommentare per attivare le stampe di debug
//#define DEBUG_SD                                              // Scommentare per attivare il debug su scheda SD esterna
//#define SCAN                                                  // Scommentare per ottenere i dati della rete cellulare dopo la connessione (scommentare anche DEBUG)



// APN SIM
const char apn[]  = "TM";                                       // Apn Things Mobile
//const char apn[]  = "internet.wind"                           // Apn WIND
//const char apn[] = "iliad";                                   // Apn Iliad
//const char apn[] = "ibox.tim.it"                              // Apn TIM (NON controllato)
//const char apn[] = "tre.it";                                  // Apn Tre
//const char apn[] = "mobile.vodafone.it";                      // Apn Vodafone (1)
//const char apn[] = "blackberry.net ";                         // Apn Vodafone (2)
//const char apn[] = "wap.omnitel.it";                          // Apn Vodafone (3)


// Orario risveglio e invio dati
const byte secondi_default = 0;                                 // Questa è l'ora a cui mandare i dati (UTC, non considera il fuso orario e il 
const byte minuti_default = 0;                                  // cambio ora solare/legale). Si consiglia di impostarlo di notte per non avere
const byte ore_default = 2;                                     // misure falsate della carica della batteria, dovute al pannello solare


//Bilancia
#define PESO_BASSO -10                                          // Peso minimo (in kg) considerato rischioso
#define PESO_ALTO 80                                            // Peso massimo (in kg) considerato rischioso


// definizione FEED mqtt
#define FEED_PESO "fablabimperia/feeds/Peso"                    // Feed peso
#define FEED_BATTERIA "fablabimperia/feeds/Batteria"            // Feed stato batteria
#define FEED_POSIZIONE "fablabimperia/feeds/Posizione/csv"      // Feed posizione GPS
#define FEED_DEBUG "fablabimperia/feeds/Debug"                  // Feed debug
#define FEED_STATO "fablabimperia/feeds/Stato"                  // Feed stato complessivo


// Definizione codice colori per lo stato generale (mostrato nel pannello adafruit)
#define COLORE_SETUP "#0000FF"                                  // Colore durante il setup (blu)
#define COLORE_OK "#00FF00"                                     // Colore se tutto funziona correttamente (verde)
#define COLORE_ORA_ERRATA "#FFEE00"                             // Colore se l'ora non è corretta (giallo)
#define COLORE_PROBLEMA_PESO "#FF6600"                          // Colore se il peso è troppo alto o basso (arancione)
#define COLORE_BATTERIA_BASSA "#FF0000"                         // Colore se la batteria è scarica (rosso)
#define COLORE_ALLARME "#FF0000"                                // Colore se l'arnia viene spostata (rosso)
#define COLORE_GPS_STACCATO "#9300FF"                           // Colore se il GPS viene staccato (viola)


//GPS e pertinenti
#define MIN_SAT_CHECK 5                                         // Numero minimo satelliti
#define maxTimeGpsInactived 120000                              // Tempo massimo di inattivita del GPS prima dell' invio dell allarme per GPS danneggiato
#define timeOutMQTT 120000                                      // Tempo massimo connessione MQTT
#define timeOutGPRS 120000                                      // Tempo massimo connessione rete cellulare
#define tolleranza_GPS 0.005                                    // Tolleranza di errore accettabile (in gradi) prima di chiamare allarme()


//INGRESSI DIGITALI
#define pin_interrupt 0                                         // Pin per interrupt per il risveglio
#define pin_GPS 1                                               // Pin alimentazione GPS

/*////////////////////////////////////////////////////////////////
// Normalmente i principali parametri da modificare si trovano  //
// qua sopra, da qui in poi dovrebbe essere standard            //
////////////////////////////////////////////////////////////////*/





#ifdef DEBUG_SD
  #include <SPI.h>
  #include <SD.h>
#endif

#define gpsPort Serial1
#define SerialMon Serial
#define SerialAT SerialGSM
#define TINY_GSM_MODEM_UBLOX
#define DEBUG_PORT Serial




//Stampiamo usando debug print per accendere e spegnere le stampe all'occorenza
#ifdef DEBUG_SD
  #define DEBUG_PRINT(x)             log_debug(x, false)
  #define DEBUG_PRINTLN(x)           log_debug(x, true)
  #define DEBUG_PRINT_MOBILE(x, y)   Serial.print (x, y)
  #define DEBUG_PRINTLN_MOBILE(x, y) Serial.println (x, y)
//  #define SVEGLIA
#else
  #ifdef DEBUG
    #define DEBUG_PRINT(x)             Serial.print(x)
    #define DEBUG_PRINTDEC(x)          Serial.print(x, DEC)
    #define DEBUG_PRINTLN(x)           Serial.println(x)
    #define DEBUG_PRINT_MOBILE(x, y)   Serial.print(x, y)
    #define DEBUG_PRINTLN_MOBILE(x, y) Serial.println(x, y)
    //    #define SVEGLIA
  #else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTDEC(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINT_MOBILE(x, y)
    #define DEBUG_PRINTLN_MOBILE(x, y)
  #endif
#endif


const char* broker = "io.adafruit.com";                         // Broker MQTT

int stato;                                                      // 0 - Setup, 1 - OK, 2 - Ora, 3 - Peso, 4 - Batteria, 5 - Gps (arnia spostata) 6 - Collegamento GPS danneggiato   (Gli stati sono in ordine di importanza crescente)

// Messaggi Telegram
#define SERVER  "api.telegram.org"                              // Indirizzo del sever
#define PORT  443                                               // Porta di default per HTTPS



int ore;                                                        //
int minuti;                                                     // Orario GPS
int secondi;                                                    //
bool fix_Loc = false;                                           // FIX3D del gps
bool fix_Loc_error = false;                                     // disponibilita GPS
NMEAGPS  gps; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values




//lat e lon della cella gsm
//double lat_GSM = 0;   //definite dentro la funzione loc_GSM
//double lon_GSM = 0;


//variabili per la gestione
bool orario_settato = false;

double latitudine_casa = 0.0;                                   // Coordinate in cui si trova di solito l'arnia (ricavate in automatico all'accensione)
double longitudine_casa = 0.0;
bool casa_trovata;

double latitud;                                                 // Coordinate in cui si trova l'arnia effettivamente
double longitud;
float altitudine;
int satelliti;                                                  // Numero satelliti utilizzati
unsigned int progressivo = 0;                                   // Progressivo ricerche GPS

volatile bool arnia_sollevata = false;                          // Variabile usata per attivazione allarme

float tens = 0;                                                 // Variabile usata per il controllo batteria


RTCZero rtc;
HX711 scale;
GSMScanner scannerNetworks;
GSMLocation location;
GSMClient client;
GSMSSLClient clientSSL;
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

  delay(40000);

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
  Pubblica(FEED_DEBUG, "SD: " + String(log_debug("Test SD...", true)));
#endif

  DEBUG_PRINTLN("setup()> Configuro i pin");
  pinMode(pin_GPS, OUTPUT);
  pinMode(pin_interrupt, INPUT_PULLUP);
  //  attachInterrupt(digitalPinToInterrupt(pin_interrupt), sollevata, FALLING);



  DEBUG_PRINTLN("setup()> Effettuo la tara della Bilancia");
  scale.set_scale(-18200);                                      // Calibro la scala (vedere il file README della libreria HX711)
  scale.tare();                                                  // Taro la scala
  DEBUG_PRINTLN("setup()> Completato taratura  bilancia");
  Pubblica(FEED_DEBUG, "SETUP: Tara bilancia effettuata sistemare l'ARNIA!!");



  delay(1000);

  stato = 0;
  Pubblica(FEED_STATO, COLORE_SETUP);
  Pubblica(FEED_DEBUG, "SETUP: Accensione!");

  delay(1000);
  check_GPS();
  trova_casa();

  Telegram("Setup%20terminato,%20è%20possibile%20sistemare%20l'arnia", CHAT_ID, false);
}




void loop() {
  //  DEBUG_PRINTLN("Loop()> inizio loop lancio gps check");
  //  check_GPS();
  DEBUG_PRINT("loop()> Controllo se la casa e' stata trovata...  ");
  if (!casa_trovata) {
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
  DEBUG_PRINT(" % --> ");
  DEBUG_PRINT(tens);
  DEBUG_PRINTLN(" V");


  DEBUG_PRINTLN("loop()> Pubblico i FEED peso e batteria");
  Pubblica(FEED_DEBUG, "Inizio Loop");
  Pubblica(FEED_PESO, String(peso));                          // Pubblico il peso
  Pubblica(FEED_BATTERIA, String(batteria));                 // Pubblico lo stato della batteria in percentuale
  Pubblica(FEED_DEBUG, ("Batt: " + String(batteria) + " %  >" + String(tens) + "V   Peso: " + String(peso) + " kg"));

  Telegram("Batteria:%20" + String(batteria) + "%20%%0aPeso:%20" + String(peso) + "%20kg", CHAT_ID, true);


  if (batteria < 30) {
    DEBUG_PRINT("loop()> valore batteria basso: ");
    DEBUG_PRINT(batteria);
    DEBUG_PRINTLN(" % (< 30%)!");
    if (stato < 4) {
      stato = 4;
      Pubblica(FEED_STATO, COLORE_BATTERIA_BASSA);
    }
  } else if (stato == 4) {
    stato = 1;
    Pubblica(FEED_STATO, COLORE_OK);
  }

  if (peso < PESO_BASSO || peso > PESO_ALTO) {
    DEBUG_PRINT("loop()> Peso al di fuori dell'intervallo ( ");
    DEBUG_PRINT(PESO_BASSO);
    DEBUG_PRINT(" - ");
    DEBUG_PRINT(PESO_ALTO);
    DEBUG_PRINT(" )");

    if (stato < 3) {
      stato = 3;
      Pubblica(FEED_STATO, COLORE_PROBLEMA_PESO);
    }
  } else if (stato == 3) {
    stato = 1;
    Pubblica(FEED_STATO, COLORE_OK);
  }

  if (!fix_Loc_error) {
    if (stato < 1) {
      stato = 1;
      Pubblica(FEED_STATO, COLORE_OK);
    }
  }

  check_GPS();
  if (latitud != 0.0 || longitud != 0.0) {                              // Se la posizione è affidabile la pubblico
    DEBUG_PRINTLN("loop()> La posizione GPS recuperata e' affidabile... ");
    Pubblica(FEED_POSIZIONE, String(progressivo) + "," + String(latitud, 15) + "," + String(longitud, 15) + "," + String(altitudine, 1));

    if (((abs(latitudine_casa - latitud) > tolleranza_GPS || abs(longitudine_casa - longitud) > tolleranza_GPS) && casa_trovata) && !fix_Loc_error) {
      DEBUG_PRINTLN("loop()> ALLARME GPS!");
      allarme();                                                                                                              // Se l'arnia è stata spostata chiamo allarme()
    }
  }
  Pubblica(FEED_DEBUG, "Loop()> Dormo!!!!!");
  DEBUG_PRINTLN("loop()> Chiamo spegni tutto e avvio standby... ");

  delay(30000);

  dormi(ore_default, minuti_default, secondi_default);

  DEBUG_PRINTLN("");
  DEBUG_PRINT("Loop()> Valore variabile stato:  ");
  DEBUG_PRINTLN(stato);
  riaccendi_tutto();
}
