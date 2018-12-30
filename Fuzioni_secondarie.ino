void sollevata(){
  arnia_sollevata = true;
}

void spegni_tutto(byte ora_sveglia, byte minuti_sveglia, byte secondi_sveglia, int giorni){

/*
  const byte day = 10;
  const byte month = 10;
  const byte year = 18;

  DEBUG_PRINT("spegni_tutto() > Setto data a ");
  DEBUG_PRINT(day);
  DEBUG_PRINT("/");
  DEBUG_PRINT(month);
  DEBUG_PRINT("/");
  DEBUG_PRINTLN(year);
  
//  rtc.setDate(day, month, year);

  #ifdef DEBUG
    rtc.setAlarmDate(day, month, year);
    
    DEBUG_PRINT("spegni_tutto() > Setto data sveglia a ");
    DEBUG_PRINT(day);
    DEBUG_PRINT("/");
    DEBUG_PRINT(month);
    DEBUG_PRINT("/");
    DEBUG_PRINTLN(year);
  #else
    rtc.setAlarmDate(day + 1, month, year);

    DEBUG_PRINT("spegni_tutto() > Setto data sveglia a ");
    DEBUG_PRINT(day + 1);
    DEBUG_PRINT("/");
    DEBUG_PRINT(month);
    DEBUG_PRINT("/");
    DEBUG_PRINTLN(year);
  #endif
*/
  
//  rtc.setAlarmTime(ora_sveglia, minuti_sveglia, secondi_sveglia);
  
//  rtc.enableAlarm(rtc.MATCH_HHMMSS);

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

//  mqtt.publish(FEED_DEBUG, String("Adesso: "+String(rtc.getHours())+":"+String(rtc.getMinutes())+":"+String(rtc.getSeconds())).c_str());
//  mqtt.publish(FEED_DEBUG, String("Sveglia: "+String(ore)+":"+String(minuti)+":"+String(secondi)).c_str());

  SerialAT.write("AT+CPWROFF");
  scale.power_down();
  digitalWrite(pin_GPS, LOW);


  DEBUG_PRINTLN("Sleep!!!");
//  rtc.standbyMode();

/*
  long tempo = domani*24*60*60 + (ora_sveglia-rtc.getHours())*60*60 + (minuti_sveglia-rtc.getMinutes())*60 + (secondi_sveglia-rtc.getSeconds());

  if (tempo < 0){
    domani = 1;
    tempo = domani*24*60*60 + (ora_sveglia-rtc.getHours())*60*60 + (minuti_sveglia-rtc.getMinutes())*60 + (secondi_sveglia-rtc.getSeconds());
  }

  arnia_sollevata = false;

  unsigned long inizio_sleep = millis();
  while (tempo*1000 >= millis() - inizio_sleep){
    DEBUG_PRINTLN("spegni_tutto() > Aspetto");
    delay(10000);

    if (arnia_sollevata){
      allarme();
    }
  }
*/
  

  rtc.setDate(1, 1, 18);
  
  while (rtc.getDay()<giorni+1 || rtc.getHours()<ora_sveglia || rtc.getMinutes()<minuti_sveglia || rtc.getSeconds()<secondi_sveglia){
    DEBUG_PRINTLN("spegni_tutto() > Aspetto");
    delay(10000);

    if (arnia_sollevata){
      riaccendi_tutto();
      allarme();
    }
  }
}

void riaccendi_tutto(){  
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

  mqtt_init();
}




void trova_casa(){
  int validi = 0;
  casa_trovata = true;
  float Min_lat = 180;
  float MAX_lat = -180;
  float Min_long = 180;
  float MAX_long = -180;
  
  mqttConnect();
  mqtt.publish(FEED_DEBUG, String("Cerco posizione casa").c_str());

  DEBUG_PRINTLN("trova_casa()> Raccolgo 20 posizioni GPS per calcolo casa...");
  
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

  mqttConnect();
  if (abs(MAX_long-Min_long)<tolleranza_GPS && abs(MAX_lat-Min_lat)<tolleranza_GPS && validi >= 5){
    latitudine_casa = latitudine_casa/validi;
    longitudine_casa = longitudine_casa/validi;
    DEBUG_PRINT("trova_casa()> Latitudine casa trovata:  ");
    DEBUG_PRINTLN(latitudine_casa);
    DEBUG_PRINT("trova_casa()> Longitudine casa trovata:  ");
    DEBUG_PRINTLN(longitudine_casa);
    mqtt.publish(FEED_DEBUG, String("Posizione casa trovata").c_str());  
  } else {
    casa_trovata = false;
    DEBUG_PRINTLN("Errore gps, posizione non valida");
    mqtt.publish(FEED_DEBUG, "Errore gps, posizione non valida");
  }
}






void allarme(){                                                                                             // Se l'arnia viene spostata...
/*  unsigned long inizio_MQTT = millis();
  while(!mqtt.connected() && millis()-inizio_MQTT < timeOutMQTT){                                           // Se non sono connesso al boroker
    DEBUG_PRINTLN("allarme()> chiamo mqttConnect...");*/
    mqttConnect();                                                                                          // mi connetto
//  }
  
  mqtt.publish(FEED_STATO, colore_allarme);
  mqtt.publish(FEED_DEBUG, "Allarme GPS");
  DEBUG_PRINT("allarme() > L'arnia è stata mossa!");

  int controllo = 120;
  while(controllo > 0){
    check_GPS();                                                                                            // Controllo se la posizione GPS è affidabile

/*
    unsigned long inizio_MQTT = millis();
    while(!mqtt.connected() && millis()-inizio_MQTT < timeOutMQTT){                                         // Se non sono connesso al boroker
      DEBUG_PRINTLN("loop()> chiamo mqttConnect...");*/
    mqttConnect();                                                                                        // mi connetto
//    }

    if (latitud != 0 || longitud != 0){                                                                     // Se è affidabile la pubblico
      mqtt.publish(FEED_POSIZIONE, String(String(progressivo)+","+String(latitud, 8)+","+String(longitud, 8)+","+String(altitudine, 1)).c_str());
      if(abs(latitudine_casa-latitud)<tolleranza_GPS && abs(longitudine_casa-longitud)<tolleranza_GPS){                       // Se l'arnia è a posto
        controllo--;                                                                                        // controllo altre volte
      } else {
        controllo = 120;                                                                                    // altrimenti continuo
        delay(30000);
      }
    }
    delay(60000);
  }

  mqttConnect();
  switch (stato){
    case 1:
      DEBUG_PRINT("allarme()> Pubblico su FEED_STATO valore ");
      DEBUG_PRINTLN(colore_ok);
      mqtt.publish(FEED_STATO, colore_ok);
      break;

    case 2:
      DEBUG_PRINT("allarme()> Pubblico su FEED_STATO valore ");
      DEBUG_PRINTLN(colore_ora_errata);
      mqtt.publish(FEED_STATO, colore_ora_errata);
      break;

    case 3:
      DEBUG_PRINT("allarme()> Pubblico su FEED_STATO valore ");
      DEBUG_PRINTLN(colore_problema_peso);
      mqtt.publish(FEED_STATO, colore_problema_peso);
      break;

    case 4:
      DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
      DEBUG_PRINTLN(colore_batteria_bassa);
      mqtt.publish(FEED_STATO, colore_batteria_bassa);
      break;
  }

  arnia_sollevata = false;
}
