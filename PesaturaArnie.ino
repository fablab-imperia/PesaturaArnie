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

//#include <TinyGsmClient.h>                                      // Per gestire il modem GSM
#include <MKRGSM.h>
#include "HX711.h"                                              // Per gestire la bilancia
#include <RTCZero.h>                                            // Par gestire l'RTC interno
#include <PubSubClient.h>                                       // Per pubblicare messaggi via MQTT
#include <NMEAGPS.h>                                            // Per il GPS

#include <SPI.h>
#include <SD.h>


#define gpsPort Serial1



/*
//Stampiamo usando debug print per accendere e spegnere le stampe all'occorenza
#ifdef DEBUG
 #define DEBUG_PRINT(x)             Serial.print (x)
 #define DEBUG_PRINTDEC(x)          Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)           Serial.println (x)
 #define DEBUG_PRINT_MOBILE(x, y)   Serial.print (x, y)
 #define DEBUG_PRINTLN_MOBILE(x, y) Serial.println (x, y)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINT_MOBILE(x, y)
 #define DEBUG_PRINTLN_MOBILE(x, y)
#endif*/

#ifdef DEBUG
 #define DEBUG_PRINT(x)             log_debug(x, false)
// #define DEBUG_PRINTDEC(x)          Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)           log_debug(x, true)
 #define DEBUG_PRINT_MOBILE(x, y)   Serial.print (x, y)
 #define DEBUG_PRINTLN_MOBILE(x, y) Serial.println (x, y)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINT_MOBILE(x, y)
 #define DEBUG_PRINTLN_MOBILE(x, y)
#endif


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
#define maxTimeGpsInactived 60000                               // Tempo massimo di inattivita del GPS prima dell' invio dell allarme per GPS danneggiato-----------------
#define timeOutMQTT 300000                                      // Tempo massimo connessione MQTT
#define timeOutGPRS 300000                                      // Tempo massimo connessione rete cellulare
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
#define pin_reset_interrupt 3                                   // pin reset interrupt

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

int accuratezza = 9999;                                         //lat e lon della cella gsm
double lat_GSM = 0;
double lon_GSM = 0;

int ore_NTP;                                                        //
int minuti_NTP;                                                     // orario server NTP UTC
int secondi_NTP;                                                    //
int anno_NTP;
int mese_NTP;
int giorno_NTP;


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
unsigned long timeInactivity;                                   // Tempo di vita del GPS
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
  
  DEBUG_PORT.begin(9600);
  delay(2000);
  /*while (!Serial){
    ;
  }*/

  /*DEBUG_PRINTLN("Init()> Starting Arduino GPRS NTP client.");
  // connection state
  bool connected = false;
  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (!connected) {
    if ((gsmAccess.begin("") == GSM_READY) &&
        (gprs.attachGPRS(apn, user, pass) == GPRS_READY)) {
      connected = true;
      DEBUG_PRINTLN("Init()> Connected");
    } else {
      DEBUG_PRINTLN("Init()> Not connected");
      delay(1000);
    }
  }*/
  //DEBUG_PRINTLN("Init()> Starting connection to server...");
  //DEBUG_PRINTLN("Init()> OK UDP Connection");
  //long int orario_NEW = gsmAccess.getTime();
  //DEBUG_PRINTLN(orario_NEW);

  delay(4000);
  Serial.print("Inizializzazione Card: ");
  if (!SD.begin(4)) //il Pin 4 è collegato a CS
  {
    Serial.println("FALLITA!");
  } else {
    Serial.println("ESEGUITO!");
  }


  DEBUG_PRINTLN("Init()> Inizializzo i dispositivi");
  rtc.begin();                                                    // Inizializzo l' RTC interno
  gpsPort.begin(9600);
  scale.begin(A1, A0);                                            //HX711.DOUT  - pin #A1            HX711.PD_SCK - pin #A0

  pinMode(pin_GPS, OUTPUT);  
  pinMode(pin_interrupt, INPUT_PULLDOWN);
  pinMode(pin_buzzer, OUTPUT);
  pinMode(pin_reset_interrupt, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(pin_interrupt), sollevata, RISING);

  DEBUG_PRINT("setup()> Broker MQTT: ");
  DEBUG_PRINTLN(broker);
  
  DEBUG_PRINTLN("setup()> MQTT Init...");

  //mqtt_init();                                                  // Connetto alla rete GPRS e al broker mqtt
  //init_GSM();
  //mqttConnect();
  
  Pubblica(FEED_DEBUG, "SD: "+String(log_debug("Test SD...", true)));
  
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
  
  
/*  unsigned long inizio_MQTT = millis();
  while(!mqtt.connected() && millis()-inizio_MQTT < timeOutMQTT){
    DEBUG_PRINTLN("loop()> chiamo mqttConnect...");
  mqttConnect();

    if (mqtt.connected()){
      DEBUG_PRINTLN("loop()> Connessione MQTT riuscita...");
    } else {
      DEBUG_PRINTLN("loop()> mqttConnect fallito: ritento...");
    }*/
  

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


  /*check_GPS();
  mqttConnect();
  if (ore == 99 && minuti == 99 && secondi == 99){                        // Se i dati non sono validi imposto un'ora a caso che verrà corretta in seguito
    rtc.setTime(byte(1), byte(0), byte(0));
    DEBUG_PRINTLN("setup()> Errore impostazione ora su RTC...");
    Pubblica(FEED_DEBUG, "Errore impostazione ora");

    if (stato < 2){
      stato = 2;
      Pubblica(FEED_STATO, colore_ora_errata);
      DEBUG_PRINT("setup()> Pubblico su FEED_STATO valore ");
      DEBUG_PRINTLN(colore_ora_errata);
    }
  } else {
    rtc.setTime(byte(ore), byte(minuti), byte(secondi));                  // Se i dati sono validi imposto l' ora
    Pubblica(FEED_DEBUG, String("Ora: "+String(ore)+" h, "+String(minuti)+" m").c_str());

    if (stato == 2){
      stato = 1;
      DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
      DEBUG_PRINTLN(colore_ok);
      Pubblica(FEED_STATO, colore_ok);
    }
  }*/

/*
  if (stato == 1){
    DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
    DEBUG_PRINTLN(colore_ok);
    Pubblica(FEED_STATO, colore_ok);
  }
*/

  DEBUG_PRINTLN("loop()> Chiamo spegni tutto e avvio standby... ");
  #ifdef DEBUG
    if (rtc.getHours() == 23 && rtc.getMinutes() >= 58)
    {spegni_tutto(0, 1, byte(30), 0);}                                                      // Sleep mode per 1 minuto
    else if (rtc.getMinutes() >= 58)
    {spegni_tutto(rtc.getHours()+1, 1, byte(30), 0);}                                        // Sleep mode per 1 minuto
    else {spegni_tutto(rtc.getHours(), rtc.getMinutes() + 1, byte(30), 0);}
    
  #else
    spegni_tutto(ore_default, minuti_default, secondi_default, 1);                                          // Sleep mode fino a domani alle 4:30:00 UTC (default)
  #endif

  riaccendi_tutto();
}
