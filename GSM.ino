void init_GSM (){
  DEBUG_PRINTLN("init_GSM()> Accendo Modulo GPRS");
  // initialize serial communications
  Serial.begin(9600);
  DEBUG_PRINTLN("init_GSM()> Inizializzo Modem");
  // connection state

  DEBUG_PRINTLN("init_GSM()> Aggancio la rete cellulare...");

  boolean notConnected = true;
  DEBUG_PRINT("init_GSM()> Tento connessione ad APN GPRS ");
  DEBUG_PRINT(apn);
  DEBUG_PRINT(" con utente ");
  DEBUG_PRINT(user);
  DEBUG_PRINT(" e pwd ");
  DEBUG_PRINTLN(user); 
  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  DEBUG_PRINT("init_GSM()> Ready:    ");

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
     }*/
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
  DEBUG_PRINTLN("orario_GSM()> contatto il server NTP per l'orario UTC");
  //Pubblica(FEED_DEBUG, "orario_GSM()> contatto il server NTP per l'orario UTC");
  long int epoch = gsmAccess.getTime();
  DEBUG_PRINTLN("orario_GSM()> start");
  DEBUG_PRINT("Unix time = ");
  DEBUG_PRINTLN(epoch);
  rtc.setEpoch(epoch);
  //Pubblica(FEED_DEBUG, "orario_GSM()> Server NTP contattato ora rilevata");
  // print the hour, minute and second:
  DEBUG_PRINT("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
  //ore_NTP = (epoch  % 86400L) / 3600;
  ore_NTP = rtc.getHours();
  DEBUG_PRINT(ore_NTP); // print the hour (86400 equals secs per day)
  DEBUG_PRINT(':');
  if ( ((epoch % 3600) / 60) < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    DEBUG_PRINT('0');
  }
  //minuti_NTP = (epoch  % 3600) / 60;
  minuti_NTP = rtc.getMinutes();
  DEBUG_PRINT(minuti_NTP); // print the minute (3600 equals secs per minute)
  DEBUG_PRINT(':');
  if ( (epoch % 60) < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    DEBUG_PRINT('0');
  }
  //secondi_NTP = epoch % 60;
  secondi_NTP = rtc.getSeconds();
  anno_NTP = rtc.getYear();
  mese_NTP = rtc.getMonth();
  giorno_NTP = rtc.getDay();
  DEBUG_PRINTLN(secondi_NTP); // print the second
  DEBUG_PRINT("Del giorno: ");
  DEBUG_PRINT(giorno_NTP);
  DEBUG_PRINT(" / ");
  DEBUG_PRINT(mese_NTP);
  DEBUG_PRINT(" / 20");
  DEBUG_PRINTLN(anno_NTP);
  //rtc.setTime(byte(ore_NTP), byte(minuti_NTP), byte(secondi_NTP));
  DEBUG_PRINTLN("orario_GSM()> end");
}
