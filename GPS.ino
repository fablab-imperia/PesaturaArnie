
void GPS() {
  
  String stringa = "     ";
  String risultato;
  String check;
  float latitudine = 0.00;
  float longitudine = 0.00;
  char c = Serial1.read();

  while (!(stringa[0] == 'G' && stringa[1] == 'N' && stringa[2] == 'G' && stringa[3] == 'G' && stringa[4] == 'A')){
    stringa = "";
    while (c != '$'){
      if (Serial1.available()){
        c = Serial1.read();
      }
    }

      while (c != '*'){
        if (Serial1.available()){
          c = Serial1.read();
          //Serial.print(c);
          stringa.concat(c);
        }
      }

      check = String(Serial1.read()) + String(Serial1.read());
  }

    //Serial.println(stringa);
    
    if (stringa[0] == 'G' && stringa[1] == 'N' && stringa[2] == 'G' && stringa[3] == 'G' && stringa[4] == 'A' ){    //&& check == checksum(stringa)){
      //Serial.println(stringa);
      latitudine = deg_posizione(trova_valore(2, stringa));
      longitudine = deg_posizione(trova_valore(4, stringa));
      Serial.print("Lat: ");
      Serial.println(latitudine, 15);
      Serial.print("Lon: ");
      Serial.println(longitudine, 15);
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

  Serial.print("Risultato :   ");
  Serial.println(risultato);
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

  casa_trovata = true;
  float Min = 180;
  float MAX = -180;
  
  int validi = 0;                                               // Medio più valori per ottenere una latitudine affidabile

  mqtt.publish(FEED_DEBUG, String("Cerco latitudine casa").c_str());
  
  for(int i=1; i<20; i++){
    GPS();                                                              // Controllo se la posizione GPS è affidabile
    unsigned long inizio_GPS = millis();
    while((latitud == 0.0 || longitud == 0.0) && millis()-inizio_GPS < 600000){
      GPS();
    }

    if (millis()-inizio_GPS < 600000){
      latitudine_casa = latitudine_casa + latitud;
      validi++;
      if (latitud > MAX){
        MAX = latitud;
      }
      if (latitud < Min){
        Min = latitud;
      }
    } else {
      mqtt.publish(FEED_DEBUG, "Errore gps trovare latitudine casa");
    }
  }
  
  if (validi != 0 && abs(MAX-Min)<0.005){
    mqtt.publish(FEED_DEBUG, String("Latitudine casa trovata").c_str());
    latitudine_casa = latitudine_casa/validi;
  } else {
    casa_trovata = false;
    mqtt.publish(FEED_DEBUG, "Errore gps, latitudine non valida");
  }

  validi = 0;                           // Medio più valori per ottenere una longitudine affidabile
  Min = 180;
  MAX = -180;

  mqtt.publish(FEED_DEBUG, String("Cerco longitudine casa").c_str());
  
  for(int i=1; i<20; i++){
    GPS();                                                              // Controllo se la posizione GPS è affidabile
    unsigned long inizio_GPS = millis();
    while((latitud == 0.0 || longitud == 0.0) && millis()-inizio_GPS < 600000){
      GPS();
    }

    if (millis()-inizio_GPS < 600000){
      longitudine_casa = longitudine_casa + longitud;
      validi++;
      if (longitud > MAX){
        MAX = latitud;
      }
      if (longitud < Min){
        Min = latitud;
      }
    } else {
      mqtt.publish(FEED_DEBUG, "Errore gps trovare longitudine casa");
    }
  }
  if (validi != 0 && abs(MAX-Min)<0.005){
    mqtt.publish(FEED_DEBUG, String("Longitudine casa trovata").c_str());
    longitudine_casa = longitudine_casa/validi;
  } else {
    casa_trovata = false;
    mqtt.publish(FEED_DEBUG, "Errore gps, longitudine non valida");
  }
}
