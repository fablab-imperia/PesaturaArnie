void check_GPS()
{
  digitalWrite(pin_GPS, HIGH);
  delay(5000);
  
  bool fix_Loc = false;
  unsigned long tempoAttivita = millis();
  while ((!gps.available(gpsPort) || !fix_Loc)) {
    if (millis()-tempoAttivita > maxTimeGpsInactived){
      stato = 6;
      mqttConnect();
      DEBUG_PRINT("\nloop()> Pubblico su FEED_STATO valore ");
      DEBUG_PRINTLN(colore_GPS_staccato);
      mqtt.publish(FEED_STATO, colore_GPS_staccato);
      mqtt.publish(FEED_DEBUG, String("GPS danneggiato").c_str());
      
      latitud, longitud, altitudine = 0.0, 0.0, 0.0;
      satelliti, ore, minuti, secondi = 0, 99, 99, 99;
      break;
    }
    DEBUG_PRINT(".");
    fix = gps.read();
    satelliti = fix.satellites;
    
    if (fix.valid.location && fix.valid.time && satelliti > MIN_SAT_CHECK){
      fix_Loc = true;
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
