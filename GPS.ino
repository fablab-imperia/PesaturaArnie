
String GPS(int mode) {            // mode 1=lat 2=long 3=stringa completa
  String stringa = "";
  String risultato;
  float latitudine = 0.00;
  float longitudine = 0.00;
  char c = Serial1.read();
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

    String check = String(Serial1.read()) + String(Serial1.read());

//    Serial.println(stringa);
    
    if (stringa[0] == 'G' && stringa[1] == 'P' && stringa[2] == 'G' && stringa[3] == 'G' && stringa[4] == 'A' && check == checksum(stringa)){
      Serial.println(stringa);
      latitudine = deg_posizione(trova_valore(2, stringa));
      longitudine = deg_posizione(trova_valore(4, stringa));
      Serial.print("Lat: ");
      Serial.println(latitudine, 15);
      Serial.print("Lon: ");
      Serial.println(longitudine, 15);
    }

  if(mode == 1){
    risultato = String(latitudine, 15);
  } else if(mode == 2) {
    risultato = String(longitudine, 15);
  } else {  
    risultato = String(latitudine, 15) + "," + String(longitudine, 15);
  }
  Serial.println(risultato);

  return(risultato);

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

  casa_trovata = true;
  
  int validi = 0;                                               // Medio più valori per ottenere una latitudine affidabile
  for(int i=1; i<20; i++){
    String posizione = GPS(1);
    unsigned long inizio_GPS = millis();
    while(posizione == "0.000000000000000" && millis()-inizio_GPS < 600000){
      posizione = GPS(1);
    }

    if (millis()-inizio_GPS < 600000){
      latitudine_casa = latitudine_casa + posizione.toDouble();
      validi++;
    } else {
      mqtt.publish(FEED_DEBUG, "Errore gps trovare latitudine casa");
    }
  }
  if (validi != 0){
    latitudine_casa = latitudine_casa/validi;
  } else {
    casa_trovata = false;
    mqtt.publish(FEED_DEBUG, "Errore gps nessuna latitudine valida");
  }

  validi = 0;                                                   // Medio più valori per ottenere una longitudine affidabile
  for(int i=1; i<20; i++){
    String posizione = GPS(2);
    unsigned long inizio_GPS = millis();
    while(posizione == "0.000000000000000" && millis()-inizio_GPS < 600000){
      posizione = GPS(2);
    }

    if (millis()-inizio_GPS < 600000){
      longitudine_casa = longitudine_casa + posizione.toDouble();
      validi++;
    } else {
      mqtt.publish(FEED_DEBUG, "Errore gps trovare longitudine casa");
    }
  }
  if (validi != 0){
    longitudine_casa = longitudine_casa/validi;
  } else {
    casa_trovata = false;
    mqtt.publish(FEED_DEBUG, "Errore gps nessuna longitudine valida");
  }
}

