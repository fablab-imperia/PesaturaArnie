void spegni() {
  scale.power_down();
  digitalWrite(pin_GPS, LOW);
  gsmAccess.shutdown();
  delay(2000);
}


void dormi(byte ora_sveglia, byte minuti_sveglia, byte secondi_sveglia) {
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

  rtc.setDate(1, 1, 19);

  DEBUG_PRINTLN("Sleep!!!");

  spegni();

  DEBUG_PRINT("spegni_tutto() > Aspetto: ");

  do {
    DEBUG_PRINT(".");

    delay(10000);

    if (digitalRead(pin_interrupt) == 0) {
      arnia_sollevata = true;
    }

    if (arnia_sollevata) {
      riaccendi_tutto();
      allarme();
    }
  } while (rtc.getHours() != ora_sveglia || rtc.getMinutes() != minuti_sveglia);
}

void riaccendi_tutto() {

  delay(1000);
  scale.power_up();
  delay(2000);

}


void trova_casa() {
  int validi = 0;
  casa_trovata = true;
  float Min_lat = 180;
  float MAX_lat = -180;
  float Min_long = 180;
  float MAX_long = -180;

  Pubblica(FEED_DEBUG, "Cerco posizione casa");

  DEBUG_PRINTLN("trova_casa()> Raccolgo 10 posizioni GPS per calcolo casa...");

  if (!fix_Loc_error) {
    for (int i = 1; i <= 10; i++) {
      check_GPS();

      if (latitud != 0.0 || longitud != 0.0) {
        validi++;
        latitudine_casa = latitudine_casa + latitud;
        longitudine_casa = longitudine_casa + longitud;
        DEBUG_PRINT(validi);
        DEBUG_PRINT("° posizione trovata su ");
        DEBUG_PRINT(i);
        DEBUG_PRINTLN(" tentativi");

        if (latitud > MAX_lat) {
          MAX_lat = latitud;
        }
        if (latitud < Min_lat) {
          Min_lat = latitud;
        }
        if (longitud > MAX_long) {
          MAX_long = longitud;
        }
        if (longitud < Min_long) {
          Min_long = longitud;
        }
      }
    }


    DEBUG_PRINT("trova_casa()> Massimo valore di longitudine recuperato:  ");
    DEBUG_PRINTLN_MOBILE(MAX_long, 7);
    DEBUG_PRINT("trova_casa()> Minimo valore di longitudine recuperato:  ");
    DEBUG_PRINTLN_MOBILE(Min_long, 7);
    DEBUG_PRINT("trova_casa()> Massimo valore di latitudine recuperato:  ");
    DEBUG_PRINTLN_MOBILE(MAX_lat, 7);
    DEBUG_PRINT("trova_casa()> Minimo valore di latitudine recuperato:  ");
    DEBUG_PRINTLN_MOBILE(Min_lat, 7);

  }


  if (abs(MAX_long - Min_long) < tolleranza_GPS && abs(MAX_lat - Min_lat) < tolleranza_GPS && validi >= 8 && !fix_Loc_error) {
    latitudine_casa = latitudine_casa / validi;
    longitudine_casa = longitudine_casa / validi;
    DEBUG_PRINT("trova_casa()> Latitudine casa trovata:  ");
    DEBUG_PRINTLN_MOBILE(latitudine_casa, 7);
    DEBUG_PRINT("trova_casa()> Longitudine casa trovata:  ");
    DEBUG_PRINTLN_MOBILE(longitudine_casa, 7);
    Pubblica(FEED_DEBUG, String("Posizione casa trovata").c_str());
  } else {
    casa_trovata = false;
    DEBUG_PRINTLN("Errore gps, posizione non valida");
    Pubblica(FEED_DEBUG, "Errore gps, posizione non valida");
  }
}


void allarme() {                                                                                            // Se l'arnia viene spostata...
  init_GSM();

  DEBUG_PRINTLN("allarme() > L'arnia è stata mossa!");
  Pubblica(FEED_STATO, COLORE_ALLARME);
  Pubblica(FEED_DEBUG, "Allarme()> Allarme GPS");
  Telegram("Allarme:%20l'arnia%20è%20stata%20mossa!", CHAT_ID, false);

  int controllo = 10;
  while (controllo > 0 && casa_trovata) {                                                                   //Controllare----------------------------------------------------
    DEBUG_PRINTLN("allarme() > Allarme in corso: contatto il GPS!");
    check_GPS();
    DEBUG_PRINTLN("Allarme()> stampo le coordinate Lat e Long Casa");                                       // Controllo la posizione GPS
    DEBUG_PRINT("Latitudine  ");
    DEBUG_PRINT_MOBILE(latitudine_casa, 7);
    DEBUG_PRINT("   Longitudine  ");
    DEBUG_PRINTLN_MOBILE(longitudine_casa, 7);

    if (latitud != 0 || longitud != 0) {                                                                    // Se è affidabile la pubblico
      Pubblica(FEED_POSIZIONE, String(String(progressivo) + "," + String(latitud, 8) + "," + String(longitud, 8) + "," + String(altitudine, 1)).c_str());
      if (abs(latitudine_casa - latitud) < tolleranza_GPS && abs(longitudine_casa - longitud) < tolleranza_GPS) { // Se l'arnia è a posto
        controllo--;
        DEBUG_PRINTLN("Allarme()> L'Arnia sta rientrando nel raggio di casa");
        Pubblica(FEED_DEBUG, "Allarme()> L'Arnia sta rientrando nel raggio di casa");
        DEBUG_PRINT("Allarme()> Controllo ancora ");
        DEBUG_PRINTLN(controllo);
        delay(180000);
      } else {
        DEBUG_PRINTLN("Allarme() > L'Arnia è sempre fuori da casa");
        Pubblica(FEED_DEBUG, "Allarme() > L'Arnia è sempre fuori da casa");
        controllo = 10;                                                                                    // altrimenti continuo
      }
    }
    delay(60000);
    DEBUG_PRINTLN("allarme() > Controllo batteria");
    float batteria = livello_batteria();
    Pubblica(FEED_BATTERIA, String(batteria));                                                            // Pubblico lo stato della batteria in percentuale
    Pubblica(FEED_DEBUG, ("Batt: " + String(batteria)));
  }

  switch (stato) {
    case 1:
      Pubblica(FEED_STATO, COLORE_OK);
      break;

    case 2:
      Pubblica(FEED_STATO, COLORE_ORA_ERRATA);
      break;

    case 3:
      Pubblica(FEED_STATO, COLORE_PROBLEMA_PESO);
      break;

    case 4:
      Pubblica(FEED_STATO, COLORE_BATTERIA_BASSA);
      break;

    case 6:
      Pubblica(FEED_STATO, COLORE_GPS_STACCATO);
      break;
  }

  arnia_sollevata = false;
  Pubblica(FEED_DEBUG, "Allarme()> L'Arnia è tornata a posto");
  Telegram("Tutto%20è%20tornato%20a%20posto!", CHAT_ID, false);
  delay(20000);
  spegni();
}


void orario_SET_RTC() {
  if (fix_Loc_error) {                       // Se i dati gps non sono validi imposto orario tramite server NTP
    //rtc.setTime(byte(ore_NTP), byte(minuti_NTP), byte(secondi_NTP));            //gia aggiornato non occorre
    DEBUG_PRINTLN("setup()> RTC aggiornato tramite NTP UTC");
    Pubblica(FEED_DEBUG, String("Ora impostata tramite NTP UTC: " + String(rtc.getHours()) + " h, " + String(rtc.getMinutes()) + " m").c_str());

    if (stato <= 2) {
      stato = 2;
      Pubblica(FEED_STATO, COLORE_ORA_ERRATA);
    }
  } else  {
    rtc.setTime(byte(ore), byte(minuti), byte(secondi));        // Se i dati sono validi imposto l' ora
    rtc.setDate(1, 1, 18);
    Pubblica(FEED_DEBUG, String("Ora impostata tramite GPS: " + String(ore) + " h, " + String(minuti) + " m").c_str());
  }
}

void Telegram(String messaggio, int id, bool silenzioso) {
  init_GSM();                                                   // Accendo il modem

  DEBUG_PRINTLN("Telegram() > Iniziamo!");
  if (clientSSL.connect(SERVER, PORT)) {
    DEBUG_PRINTLN("Telegram() > Connesso!");
    DEBUG_PRINTLN("Invio  /bot" +  String(TOKEN) + "/sendMessage?chat_id=" + String(id) + "&text=" + messaggio + "&disable_notification=" + String(silenzioso));
    clientSSL.print("POST ");
    clientSSL.print("/bot" +  String(TOKEN) + "/sendMessage?chat_id=" + String(id) + "&text=" + messaggio + "&disable_notification=" + String(silenzioso));
    clientSSL.println(" HTTP/1.1");
    clientSSL.print("Host: ");
    clientSSL.println(SERVER);
    clientSSL.println("Connection: close");
    clientSSL.println();
    DEBUG_PRINTLN("Telegram() > Inviato!");
  }
  delay(10000);
  DEBUG_PRINTLN("Telegram() > Finito!");
}

bool log_debug(String LOG, bool ln) {
  if (ln) {
    Serial.println(LOG);
  } else {
    Serial.print(LOG);
  }

#ifdef DEBUG_SD
  File file;
  file = SD.open("log.txt", FILE_WRITE);                                      //File in scrittura
  if (file)                                                                   //Se il file è stato aperto correttamente
  {
    file.println(String(rtc.getHours()) + ':' + String(rtc.getMinutes()) + ':' + String(rtc.getSeconds()) + "  -->  " + LOG); //Scrivo su file il numero

    file.close(); //Chiusura file

    return true;
  } else {
    return false;
  }
#endif
}

bool log_debug(int LOG, bool ln) {                                              // Chiamo la funzione precedente modificando il tipo di dato da salvare
  return log_debug(String(LOG), ln);
}

bool log_debug(float LOG, bool ln) {
  return log_debug(String(LOG), ln);
}

bool log_debug(double LOG, bool ln) {
  return log_debug(String(LOG), ln);
}

bool log_debug(long LOG, bool ln) {
  return log_debug(String(LOG), ln);
}

void check_RAM() {
#ifdef RAM
  //Serial.println(F("Free RAM = ")); //F function does the same and is now a built in library, in IDE > 1.0.0
  //Serial.println(freeMemory(), DEC);  // print how much RAM is available.
#else
#endif
}
