void init_GSM(){
  DEBUG_PRINTLN("init_GSM()> Accendo Modulo GPRS");
  // initialize serial communications
  //Serial.begin(9600);
  DEBUG_PRINTLN("init_GSM()> Inizializzo Modem");
  // connection state

  DEBUG_PRINTLN("init_GSM()> Aggancio la rete cellulare...");
  
  
  DEBUG_PRINT("init_GSM()> Tento connessione ad APN GPRS ");
  DEBUG_PRINT(apn);
  DEBUG_PRINT(" con utente ");
  DEBUG_PRINT(user);
  DEBUG_PRINT(" e pwd ");
  DEBUG_PRINTLN(user); 
  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  /*DEBUG_PRINT("init_GSM()> Ready:    ");
  boolean notConnected = true;
  gsmAccess.setTimeout(60000);
  gprs.setTimeout(60000);
  
  unsigned long inizioGPRS = millis();
  while(notConnected && millis()-inizioGPRS < timeOutGPRS){
    if (gsmAccess.begin(pin_card, true, true) == GSM_READY) {
      DEBUG_PRINT("gsmAccess.begin passato!   ");
      if (gprs.attachGPRS(apn, user, pass) == GPRS_READY){
        notConnected = false;
        DEBUG_PRINTLN("!!!!!!!!!!Connesso!!!!!!!!!!!");
      } else {
        DEBUG_PRINTLN("Ritento gprs.attachGPRS");
      }
    } 
    else {
      DEBUG_PRINTLN("Ritento gsmAccess.begin");
      delay(1000);
     }
     
   /* GSM3_NetworkStatus_t g = gsmAccess.begin(pin_card, true, true);
    DEBUG_PRINTLN("Stampo gsm netkork status:  " + g);
    if (g == GSM_READY) {
      if (gprs.attachGPRS(apn, user, pass) == GPRS_READY){
        notConnected = false;
        DEBUG_PRINTLN("!!!!!!!!!!Connesso!!!!!!!!!!!");
      } else {
        DEBUG_PRINTLN("Ritento gprs.attachGPRS");
      }
    } 
    else {
      DEBUG_PRINTLN("Ritento gsmAccess.begin");
      delay(1000);
     }
  }*/


  unsigned long inizioGPRS = millis();
  bool connesso = false;
  int stato_GPRS = gprs.status();
  int acces;
  int atach;
  DEBUG_PRINT("Init()> Stato della connessione GSM_GPRS: 1-->Non connesso |4|-->Connesso al Web:  ");
  DEBUG_PRINTLN(stato_GPRS); 
  DEBUG_PRINTLN("Init()> Attendi Sto eseguendo la connessione......");

  // Era "while ((!connesso || stato_GPRS == 4) && millis()-inizioGPRS < timeOutGPRS) {"
  while (!connesso && (gsmAccess.status() != 3 || gprs.status() != 4) && millis()-inizioGPRS < timeOutGPRS) {
    do {
      acces = gsmAccess.begin(pin_card, true, true);
    } while (acces != 3 && millis()-inizioGPRS < timeOutGPRS);
    do {  
      atach = gprs.attachGPRS(apn, user, pass, true);
    } while (atach != 4 && millis()-inizioGPRS < timeOutGPRS);
    
    DEBUG_PRINT("Init_GSM()> Stato del GSM: 1-->Pronto 2-->Attesa |3|-->Attivo:  ");
    DEBUG_PRINTLN(acces); 
    DEBUG_PRINT("Init_GSM()> Stato del GPRS: 1-->Pronto 2-->Non connesso |4|-->Connesso:  ");
    DEBUG_PRINTLN(atach); 
    if (gprs.ping("www.google.com")> 0) {
      connesso = true;
    } else DEBUG_PRINTLN("Init_GSM> Non connesso ritento");
    delay(1000);
  }


  #ifdef SCAN 
    
    
    /*DEBUG_PRINT("init_GSM()> SCANSIONE RETE GSM    ");
    String scan = scannerNetworks.readNetworks();
    DEBUG_PRINTLN(scan);*/
  
    DEBUG_PRINTLN("init_GSM()> INFORMAZIONI RECUPERATE DA RETE CELLULARE");
    String cop = scannerNetworks.getCurrentCarrier();
    
    DEBUG_PRINT("init_GSM()> OPERATORE ");
    DEBUG_PRINTLN(cop);
    
    String csq = scannerNetworks.getSignalStrength();
    DEBUG_PRINT("init_GSM()> Segnale:  ");
    DEBUG_PRINTLN(csq);
  #else 
    DEBUG_PRINTLN("init_GSM()> Diagnostica modem disabilitata definire #SCAN per riattivare");
  #endif

  

  
}



void orario_GSM()
{
  
  DEBUG_PRINTLN("orario_GSM()> contatto la cella per l'orario UTC");
  //Pubblica(FEED_DEBUG, "orario_GSM()> contatto il server NTP per l'orario UTC");
  long int epoch = gsmAccess.getTime();
  DEBUG_PRINTLN("orario_GSM()> start");
  DEBUG_PRINT("Unix time = ");
  DEBUG_PRINTLN(epoch);
  rtc.setEpoch(epoch);
  DEBUG_PRINT("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
  DEBUG_PRINT(rtc.getHours());
  DEBUG_PRINT(':');
  DEBUG_PRINT(rtc.getMinutes());
  DEBUG_PRINTLN(rtc.getSeconds());
  DEBUG_PRINT("Del giorno: ");
  DEBUG_PRINT(rtc.getDay());
  DEBUG_PRINT(" / ");
  DEBUG_PRINT(rtc.getMonth());
  DEBUG_PRINT(" / 20");
  DEBUG_PRINTLN(rtc.getYear());
  DEBUG_PRINTLN("orario_GSM()> end");
}
