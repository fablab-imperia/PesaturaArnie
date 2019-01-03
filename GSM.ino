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
  
  while(notConnected)
  {
    if ((gsmAccess.begin(pin_card) == GSM_READY) && (gprs.attachGPRS(apn, user, pass) == GPRS_READY)) {
      notConnected = false;
      DEBUG_PRINTLN("!!!!!!!!!!Connesso!!!!!!!!!!!");
      } 
     else {
      DEBUG_PRINTLN("Ritento");
      delay(1000);
     }
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
