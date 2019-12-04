void check_GPS()
{
  digitalWrite(pin_GPS, HIGH);
  delay(1000);
  fix_Loc = false;
  unsigned long tempoAttivita = millis();
  while (((!gps.available(gpsPort) || !fix_Loc) && millis()-tempoAttivita < maxTimeGpsInactived+10000)) {
    if (millis()-tempoAttivita > maxTimeGpsInactived){
      if (stato != 6) {
        Pubblica(FEED_STATO, colore_GPS_staccato);
        Pubblica(FEED_DEBUG, String("GPS danneggiato").c_str());
        Pubblica(FEED_DEBUG, String("Cerco una posizione usando la cella GSM").c_str());
        fix_Loc_error = true;
        stato = 6;
        orario_GSM();
        gsm_LOC();
        orario_SET_RTC();
/*        if (latitudine_casa == 0 && longitudine_casa == 0){
          latitudine_casa = latitud;
          longitudine_casa = longitud;
        }*/
        altitudine = 0;
        satelliti = 0;
        DEBUG_PRINTLN("check_GPS> Tempo eccessivo GPS loc con GSM e stato era != 6");
        break;
      }
      else if (stato == 6) {
        DEBUG_PRINTLN("");
        //latitud = lat_GSM;
        //longitud = lon_GSM;
        altitudine = 0;
        satelliti, ore, minuti, secondi = 0, 99, 99, 99;
        DEBUG_PRINTLN_MOBILE(latitud, 7);
        DEBUG_PRINTLN_MOBILE(longitud, 7);
        DEBUG_PRINTLN(altitudine);
        DEBUG_PRINTLN("check_GPS> Tempo eccessivo GPS loc con GSM e stato è 6");
        break;
      }
    }
    fix = gps.read();
    satelliti = fix.satellites;
    
    if (fix.valid.location && fix.valid.time && satelliti > MIN_SAT_CHECK){
      fix_Loc = true;
      fix_Loc_error = false;
      DEBUG_PRINT("Posizione: ");
      DEBUG_PRINT_MOBILE( fix.latitude(), 6 );
      latitud = fix.latitude();
      DEBUG_PRINT( "°, " );
      DEBUG_PRINT_MOBILE( fix.longitude(), 6 );
      longitud = fix.longitude();
      DEBUG_PRINT("°      Altitudine: ");
      DEBUG_PRINT(fix.altitude());
      altitudine = fix.altitude();
      DEBUG_PRINT(" m        Satelliti: ");
      DEBUG_PRINTLN(satelliti);

      if (!orario_settato){
        DEBUG_PRINT(fix.dateTime.hours);
        ore = fix.dateTime.hours;
        DEBUG_PRINT(":");
        DEBUG_PRINT(fix.dateTime.minutes);
        minuti = fix.dateTime.minutes;
        DEBUG_PRINT(":");
        DEBUG_PRINTLN(fix.dateTime.seconds);
        secondi = fix.dateTime.seconds;
        orario_SET_RTC();
        orario_settato = true;
      }
      progressivo++;

      if (stato == 6){
        stato = 1;
        Pubblica(FEED_STATO, colore_ok);
      }
      break;
    }
  }
  
  digitalWrite(pin_GPS, LOW);
}

void gsm_LOC() {
  int accuratezza = 9999;
  if (gprs.status() != 4) {
    DEBUG_PRINTLN("gsm_LOC()> Modem Disconnesso rieseguo la connessione init_GSM");
    init_GSM();  
  }
  DEBUG_PRINTLN("gsm_LOC()> GSM_LOC entrato");
  int tentativo = 0;
  while (!location.available() && !location.available()) {
    //DEBUG_PRINT("|");
    //accuratezza = location.accuracy();
  }
  DEBUG_PRINTLN("gsm_LOC()> GSM_LOC disponibile cerco posizione");
  while (accuratezza > 1000) {
    //DEBUG_PRINT(".");
    if (tentativo > 10000) {
      //DEBUG_PRINT(".");
      tentativo = 0;
    }
    accuratezza = location.accuracy();
    latitud = location.latitude();
    longitud = location.longitude();
    if (location.available()){
      /*DEBUG_PRINT_MOBILE(lat_GSM, 7);
      DEBUG_PRINT(";");
      DEBUG_PRINT_MOBILE(lon_GSM, 7);
      DEBUG_PRINT(";");
      DEBUG_PRINTLN(accuratezza);*/
      if (accuratezza < 1000 && accuratezza != 0) {
        
      }
    }
    tentativo ++;
  }
  DEBUG_PRINTLN("gsm_LOC()> GSM_LOC posizione trovata"); 
  DEBUG_PRINT("Location: Lat");
        DEBUG_PRINT_MOBILE(latitud, 7);
        DEBUG_PRINT(",Lon ");
        DEBUG_PRINTLN_MOBILE(longitud, 7);
        DEBUG_PRINT("Accuracy: +/- ");
        DEBUG_PRINT(accuratezza);
        DEBUG_PRINTLN("m\n");
//        DEBUG_PRINTLN();
        Pubblica(FEED_DEBUG, String("Triangolazione cella COMPLETA").c_str());
}
