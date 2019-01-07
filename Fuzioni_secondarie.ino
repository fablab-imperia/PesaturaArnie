void sollevata(){
  arnia_sollevata = true;
}

void spegni_tutto(byte ora_sveglia, byte minuti_sveglia, byte secondi_sveglia, int giorni){
  DEBUG_PRINT("spegni_tutto() > Setto ora sveglia a ");
  DEBUG_PRINT(ora_sveglia);
  DEBUG_PRINT(":");
  DEBUG_PRINT(minuti_sveglia);
  DEBUG_PRINT(":");
  DEBUG_PRINTLN(secondi_sveglia);
  DEBUG_PRINT("spegni_tutto() > Ora attuale ");
  DEBUG_PRINT(rtc.getHours());
  DEBUG_PRINT(":");
  DEBUG_PRINT(rtc.getMinutes());
  DEBUG_PRINT(":");
  DEBUG_PRINTLN(rtc.getSeconds());

  rtc.setDate(1, 1, 18);

  //  Pubblica(FEED_DEBUG, String("Adesso: "+String(rtc.getHours())+":"+String(rtc.getMinutes())+":"+String(rtc.getSeconds())).c_str());
  //  Pubblica(FEED_DEBUG, String("Sveglia: "+String(ore)+":"+String(minuti)+":"+String(secondi)).c_str());

  SerialAT.write("AT+CPWROFF");
  scale.power_down();
  digitalWrite(pin_GPS, LOW);
  DEBUG_PRINTLN("Sleep!!!");
  gsmAccess.lowPowerMode();
     
//  int sveglia = 0;
  while (rtc.getDay()<giorni+1 || rtc.getHours()<ora_sveglia || rtc.getMinutes()<minuti_sveglia || rtc.getSeconds()<secondi_sveglia){
//  while (sveglia < 6) {
    DEBUG_PRINT("spegni_tutto() > Aspetto: ");
//    DEBUG_PRINT(6 - sveglia);
//    DEBUG_PRINTLN(" secondi.");
    delay(10000);
//    sveglia ++;

    if (arnia_sollevata){
      riaccendi_tutto();
      allarme();
    }
  }
}

void riaccendi_tutto(){
  gsmAccess.noLowPowerMode();
  pinMode(GSM_DTR, OUTPUT);                 // Accendi modulo GPRS
  digitalWrite(GSM_DTR, LOW);
  delay(5);

  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);
  
  delay(1000);       
  scale.power_up();
  delay(2000);

  init_GSM();
  mqttConnect();
}


void trova_casa(){
  //|check_GPS();
  int validi = 0;
  casa_trovata = true;
  float Min_lat = 180;
  float MAX_lat = -180;
  float Min_long = 180;
  float MAX_long = -180;
  
//  mqttConnect();
  Pubblica(FEED_DEBUG, "Cerco posizione casa");

  DEBUG_PRINTLN("trova_casa()> Raccolgo 10 posizioni GPS per calcolo casa...");

  if (!fix_Loc_error) {
    for(int i=1; i<=10; i++){
      check_GPS();
      
      if (latitud != 0.0 || longitud != 0.0){
        validi++;
        latitudine_casa = latitudine_casa + latitud;
        longitudine_casa = longitudine_casa + longitud;
        DEBUG_PRINT(validi);
        DEBUG_PRINT("° posizione trovata su ");
        DEBUG_PRINT(i);
        DEBUG_PRINTLN(" tentativi");
      
        if (latitud > MAX_lat){
          MAX_lat = latitud;
        }
        if (latitud < Min_lat){
          Min_lat = latitud;
        } 
        if (longitud > MAX_long){
          MAX_long = longitud;
        }
        if (longitud < Min_long){
          Min_long = longitud;
        }
      }
    }
  

    DEBUG_PRINT("trova_casa()> Massimo valore di longitudine recuperato:  ");
    DEBUG_PRINTLN(MAX_long);
    DEBUG_PRINT("trova_casa()> Minimo valore di longitudine recuperato:  ");
    DEBUG_PRINTLN(Min_long);
    DEBUG_PRINT("trova_casa()> Massimo valore di latitudine recuperato:  ");
    DEBUG_PRINTLN(MAX_lat);
    DEBUG_PRINT("trova_casa()> Minimo valore di latitudine recuperato:  ");
    DEBUG_PRINTLN(Min_lat);

  }
  
   
  if (abs(MAX_long-Min_long)<tolleranza_GPS && abs(MAX_lat-Min_lat)<tolleranza_GPS && validi >= 5 && !fix_Loc_error){
    latitudine_casa = latitudine_casa/validi;
    longitudine_casa = longitudine_casa/validi;
    DEBUG_PRINT("trova_casa()> Latitudine casa trovata:  ");
    DEBUG_PRINTLN(latitudine_casa);
    DEBUG_PRINT("trova_casa()> Longitudine casa trovata:  ");
    DEBUG_PRINTLN(longitudine_casa);
    Pubblica(FEED_DEBUG, String("Posizione casa trovata").c_str());  
  } else {
    casa_trovata = false;
    DEBUG_PRINTLN("Errore gps, posizione non valida");
    Pubblica(FEED_DEBUG, "Errore gps, posizione non valida");
  }
}


void allarme(){                                                                                             // Se l'arnia viene spostata...
/*  unsigned long inizio_MQTT = millis();
  while(!mqtt.connected() && millis()-inizio_MQTT < timeOutMQTT){                                           // Se non sono connesso al boroker
    DEBUG_PRINTLN("allarme()> chiamo mqttConnect...");
    mqttConnect();                                                                                          // mi connetto
  }*/
  
  Pubblica(FEED_STATO, colore_allarme);
  Pubblica(FEED_DEBUG, "Allarme GPS");
  DEBUG_PRINT("allarme() > L'arnia è stata mossa!");

  int controllo = 120;
  while(controllo > 0){
    check_GPS();                                                                                            // Controllo se la posizione GPS è affidabile
//    mqttConnect();                                                                                        // mi connetto
    if (latitud != 0 || longitud != 0){                                                                     // Se è affidabile la pubblico
      Pubblica(FEED_POSIZIONE, String(String(progressivo)+","+String(latitud, 8)+","+String(longitud, 8)+","+String(altitudine, 1)).c_str());
      if(abs(latitudine_casa-latitud)<tolleranza_GPS && abs(longitudine_casa-longitud)<tolleranza_GPS){     // Se l'arnia è a posto
        controllo--;                                                                                        // controllo altre volte
      } else {
        controllo = 120;                                                                                    // altrimenti continuo
        //delay(30000);
      }
    }
    delay(60000);
  }

//  mqttConnect();
  switch (stato){
    case 1:
//      DEBUG_PRINT("allarme()> Pubblico su FEED_STATO valore ");
//      DEBUG_PRINTLN(colore_ok);
      Pubblica(FEED_STATO, colore_ok);
      break;

    case 2:
//      DEBUG_PRINT("allarme()> Pubblico su FEED_STATO valore ");
//      DEBUG_PRINTLN(colore_ora_errata);
      Pubblica(FEED_STATO, colore_ora_errata);
      break;

    case 3:
//      DEBUG_PRINT("allarme()> Pubblico su FEED_STATO valore ");
//      DEBUG_PRINTLN(colore_problema_peso);
      Pubblica(FEED_STATO, colore_problema_peso);
      break;

    case 4:
//      DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
//      DEBUG_PRINTLN(colore_batteria_bassa);
      Pubblica(FEED_STATO, colore_batteria_bassa);
      break;
  }

  arnia_sollevata = false;
}


void orario_SET_RTC() {
//  mqttConnect();
  if (fix_Loc_error){                        // Se i dati gps non sono validi imposto orario tramite server NTP
    //rtc.setTime(byte(ore_NTP), byte(minuti_NTP), byte(secondi_NTP));            //gia aggiornato non occorre
    DEBUG_PRINTLN("setup()> RTC aggiornato tramite NTP UTC");
    Pubblica(FEED_DEBUG, String("Ora impostata tramite NTP UTC: "+String(ore_NTP)+" h, "+String(minuti_NTP)+" m").c_str());

    if (stato <= 2){
      stato = 2;
      Pubblica(FEED_STATO, colore_ora_errata);
//      DEBUG_PRINT("setup()> Pubblico su FEED_STATO valore ");
//      DEBUG_PRINTLN(colore_ora_errata);
    }
  } else  {
    rtc.setTime(byte(ore), byte(minuti), byte(secondi));        // Se i dati sono validi imposto l' ora
    rtc.setDate(1, 1, 18);
    Pubblica(FEED_DEBUG, String("Ora impostata tramite GPS: "+String(ore)+" h, "+String(minuti)+" m").c_str());
  }
}

bool log_debug(String LOG, bool nl){
  if (nl){
    Serial.println(LOG);
  } else {
    Serial.print(LOG);
  }
  
  File file;
  file = SD.open("log.txt", FILE_WRITE); //File in scrittura
  if (file) //Se il file è stato aperto correttamente
  {
    file.println(String(rtc.getHours())+':'+String(rtc.getMinutes())+':'+String(rtc.getSeconds())+"  -->  "+LOG); //Scrivo su file il numero
    
    file.close(); //Chiusura file

    return true;
  } else {
    return false;
  }
}

bool log_debug(int LOG, bool nl){
  return log_debug(String(LOG), nl);
}

bool log_debug(float LOG, bool nl){
  return log_debug(String(LOG), nl);
}

bool log_debug(double LOG, bool nl){
  return log_debug(String(LOG), nl);
}

bool log_debug(long LOG, bool nl){
  return log_debug(String(LOG), nl);
}
