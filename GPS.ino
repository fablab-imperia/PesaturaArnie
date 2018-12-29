void GPS() {
  
  String stringa = get_stringa();
  String risultato;
  float latitudine = 0.00;
  float longitudine = 0.00;

    //Serial.println(stringa);
    
    if (stringa[0] == 'G' && stringa[1] == 'N' && stringa[2] == 'G' && stringa[3] == 'G' && stringa[4] == 'A' && stato!=6){    //&& check == checksum(stringa)){
      //Serial.println(stringa);
      latitudine = deg_posizione(trova_valore(2, stringa));
      longitudine = deg_posizione(trova_valore(4, stringa));

      if (trova_valore(3, stringa) == "S"){
        latitudine = -1*latitudine;
      }
      if (trova_valore(5, stringa) == "W"){                                                             // W ???
        longitudine == -1*longitudine;
      }

      n_satelliti = trova_valore(7, stringa).toInt();
      precisione = trova_valore(8, stringa).toInt();                                                    // 1-2 Eccellente, 2-5 Buono, 5-10 Medio, 10-20 Bassa, >20 Molto bassa
    }

  latitud = latitudine;
  longitud = longitudine;
  

}

String trova_valore(int posizione, String stringa){
  int virgole = 0;
  int carattere = 0;
  String risultato = ""; 
  while (virgole != posizione && carattere < stringa.length()){
    if (stringa[carattere] == ','){
      virgole++;
    }
    carattere++;
  }
  while (stringa[carattere] != ',' && carattere < stringa.length()){
    risultato.concat(stringa[carattere]);
    carattere++;
  }

  if (risultato.length() == 0){
    risultato = "0";
  }

 
  return(risultato);
}



float deg_posizione(String input){
  float valore = input.toFloat();
  int gradi = int(valore/100);
  float primi = valore - (gradi*100);
  float posizione = gradi + (primi/60);

  return(posizione);
}



void trova_casa(){
  int validi = 0;
  casa_trovata = true;
  float Min_lat = 180;
  float MAX_lat = -180;
  float Min_long = 180;
  float MAX_long = -180;
  

  mqtt.publish(FEED_DEBUG, String("Cerco posizione casa").c_str());

  DEBUG_PRINTLN("trova_casa()> Raccolgo 20 posizioni GPS per calcolo casa...");
  
  for(int i=1; i<20; i++)
  {
    // Controllo se la posizione GPS è affidabile
    unsigned long inizio_GPS = millis();
    while((latitud == 0.0 || longitud == 0.0) && millis()-inizio_GPS < 300000 && stato != 6){
      GPS();
    }
    if (latitud != 0.0 && longitud != 0.0){
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
  
  if (abs(MAX_lat-Min_lat)<0.005 && validi >= 5){
    latitudine_casa = latitudine_casa/validi;
    DEBUG_PRINT("trova_casa()> Latitudine casa trovata:  ");
    DEBUG_PRINTLN(latitudine_casa);
    mqtt.publish(FEED_DEBUG, String("Latitudine casa trovata").c_str());  
  } else {
    casa_trovata = false;
    DEBUG_PRINTLN("Errore gps, latitudine non valida");
    mqtt.publish(FEED_DEBUG, "Errore gps, latitudine non valida");
  }

  if (abs(MAX_long-Min_long)<0.005 && validi >= 5){
    longitudine_casa = longitudine_casa/validi;
    DEBUG_PRINT("trova_casa()> Longitudine casa trovata:  ");
    DEBUG_PRINTLN(longitudine_casa);
    mqtt.publish(FEED_DEBUG, String("Longitudine casa trovata").c_str());  
  } else {
    casa_trovata = false;
    DEBUG_PRINTLN("Errore gps, longitudine non valida");
    mqtt.publish(FEED_DEBUG, "Errore gps, longitudine non valida");
  }

}

void noGps(){
  unsigned long inizio_MQTT = millis();
  while(!mqtt.connected() && millis()-inizio_MQTT < 300000){
    DEBUG_PRINTLN("loop()> chiamo mqttConnect...");
    mqttConnect();

    if (mqtt.connected()){
      DEBUG_PRINTLN("loop()> Connessione MQTT riuscita...");
    } else {
      DEBUG_PRINTLN("loop()> mqttConnect fallito: ritento...");
    }
  }
  
  mqtt.publish(FEED_STATO, colore_GPS_staccato);
  //delay(5000);
  mqtt.publish(FEED_DEBUG, String("Collegamento con gps danneggiato").c_str());
  DEBUG_PRINTLN("GPS()> Il collegamento con il gps è danneggiato");
  }
