void check_GPS()
{
  digitalWrite(pin_GPS, HIGH);
  delay(1000);
  unsigned long tempoAttivita = millis();
  while ((!gps.available(gpsPort) || !fix_Loc)) {
    if (millis()-tempoAttivita > maxTimeGpsInactived){
      if (stato != 6) {
        mqttConnect();
        DEBUG_PRINT("\nloop()> Pubblico su FEED_STATO valore ");
        DEBUG_PRINTLN(colore_GPS_staccato);
        mqtt.publish(FEED_STATO, colore_GPS_staccato);
        mqtt.publish(FEED_DEBUG, String("GPS danneggiato").c_str());
        mqtt.publish(FEED_DEBUG, String("Cerco una posizione usando la cella GSM").c_str());
        fix_Loc_error = true;
        stato = 6;
        orario_GSM();
        gsm_LOC();
        orario_SET_RTC();
        latitud = lat_GSM;
        longitud = lon_GSM;
        altitudine = 0;
        satelliti = 0;
        DEBUG_PRINTLN_MOBILE(latitud, 7);
        DEBUG_PRINTLN_MOBILE(longitud, 7);
        DEBUG_PRINTLN(altitudine);
        break;
      }
      else if (stato == 6) {
        DEBUG_PRINTLN("/");
        latitud = lat_GSM;
        longitud = lon_GSM;
        altitudine = 0;
        satelliti, ore, minuti, secondi = 0, 99, 99, 99;
        DEBUG_PRINTLN_MOBILE(latitud, 7);
        DEBUG_PRINTLN_MOBILE(longitud, 7);
        DEBUG_PRINTLN(altitudine);
        break;
      }
    }
    //else DEBUG_PRINT("*");
    
    //DEBUG_PRINT(".");
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
      DEBUG_PRINTLN_MOBILE( fix.altitude(), 2 );
      altitudine = fix.altitude();
      DEBUG_PRINT("m,    Satelliti: ");
      DEBUG_PRINTLN(satelliti);
      DEBUG_PRINT(fix.dateTime.hours);
      ore = fix.dateTime.hours;
      DEBUG_PRINT(":");
      DEBUG_PRINT(fix.dateTime.minutes);
      minuti = fix.dateTime.minutes;
      DEBUG_PRINT(":");
      DEBUG_PRINTLN(fix.dateTime.seconds);
      secondi = fix.dateTime.seconds;
      orario_SET_RTC();
      progressivo++;

      if (stato == 6){
        stato = 1;
        mqttConnect();
        DEBUG_PRINT("loop()> Pubblico su FEED_STATO valore ");
        DEBUG_PRINTLN(colore_ok);
        mqtt.publish(FEED_STATO, colore_ok);
      }
      break;
    }
  }
  
  digitalWrite(pin_GPS, LOW);
}

void gsm_LOC() {
  
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
    lat_GSM = location.latitude();
    lon_GSM = location.longitude();
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
        DEBUG_PRINT_MOBILE(lat_GSM, 7);
        DEBUG_PRINT(",Lon ");
        DEBUG_PRINTLN_MOBILE(lon_GSM, 7);
        DEBUG_PRINT("Accuracy: +/- ");
        DEBUG_PRINT(accuratezza);
        DEBUG_PRINTLN("m");
        DEBUG_PRINTLN();
        mqtt.publish(FEED_DEBUG, String("Triangolazione cella COMPLETA").c_str());
}

void orario_GSM()
{
  DEBUG_PRINTLN("orario_GSM()> contatto il server NTP per l'orario UTC");
  //mqtt.publish(FEED_DEBUG, "orario_GSM()> contatto il server NTP per l'orario UTC");
  long int epoch = gsmAccess.getTime();
  DEBUG_PRINTLN("orario_GSM()> start");
  DEBUG_PRINT("Unix time = ");
  DEBUG_PRINTLN(epoch);
  rtc.setEpoch(epoch);
  //mqtt.publish(FEED_DEBUG, "orario_GSM()> Server NTP contattato ora rilevata");
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
