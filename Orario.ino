int orario_ore(String stringa) {
  String ora_stringa = "99";
  int ora = 99;

  //stringa = "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
  if (stringa[0] == 'G' && stringa[1] == 'N' && stringa[2] == 'G' && stringa[3] == 'G' && stringa[4] == 'A'){
//    Serial.println(stringa);
    String ora_stringa = trova_valore_ora(1, stringa);
    ora_stringa = String(ora_stringa[0]) + String(ora_stringa[1]);
    ora = ora_stringa.toInt();
    Serial.print("Ore: ");
    Serial.println(ora);
  }

  return(ora);
}







int orario_minuti(String stringa) {
  String minuti_stringa = "99";
  int minuti = 99;

  //stringa = "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
  if (stringa[0] == 'G' && stringa[1] == 'N' && stringa[2] == 'G' && stringa[3] == 'G' && stringa[4] == 'A'){
//    Serial.println(stringa);
    String minuti_stringa = trova_valore_ora(1, stringa);
    minuti_stringa = String(minuti_stringa[2]) + String(minuti_stringa[3]);
    minuti = minuti_stringa.toInt();
    Serial.print("Minuti: ");
    Serial.println(minuti);
  }

  return(minuti);
}








int orario_secondi(String stringa) {
  String secondi_stringa = "99";
  int secondi = 99;

  //stringa = "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
  if (stringa[0] == 'G' && stringa[1] == 'N' && stringa[2] == 'G' && stringa[3] == 'G' && stringa[4] == 'A'){
//    Serial.println(stringa);
    String secondi_stringa = trova_valore_ora(1, stringa);
    secondi_stringa = String(secondi_stringa[4]) + String(secondi_stringa[5]);
    secondi = secondi_stringa.toInt();
    Serial.print("Secondi: ");
    Serial.println(secondi);
  }

  return(secondi);
}




String get_stringa(){
  String stringa = "     ";
  char c = Serial1.read();
  while (!(stringa[0] == 'G' && stringa[1] == 'N' && stringa[2] == 'G' && stringa[3] == 'G' && stringa[4] == 'A')){
    stringa = "";
    while (c != '$'){
      if (Serial1.available()){
        c = Serial1.read();
        timeInactivity = millis();
      }

      if (!Serial1.available() && stato < 6 && (millis() - timeInactivity) > maxTimeGpsInactived){
        stato = 6;
        noGps();
      } else if (Serial1.available() && stato == 6) {
        stato = 1;
      }

      if (stato == 6){
        goto uscita_no_GPS;
      }
    }

    while (c != '*' && stato != 6){
      if (Serial1.available()){
        c = Serial1.read();
        timeInactivity = millis();
        DEBUG_PRINT(c);
        stringa.concat(c);
      }         
        else if (!Serial1.available() && stato < 6 && (millis() - timeInactivity) > maxTimeGpsInactived){
        stato = 6;
        noGps();
      } else if (Serial1.available() && stato == 6) {
        stato = 1;
        
      }
    }

    DEBUG_PRINT("GPS()> Stato:");
    DEBUG_PRINTLN(stato);

    check = String(Serial1.read()) + String(Serial1.read());
    DEBUG_PRINT("Checksum: ");
    DEBUG_PRINT(check);
    DEBUG_PRINT("       Qualità posizione: ");
    DEBUG_PRINT(precisione);
    DEBUG_PRINT("       Numero satelliti: ");
    DEBUG_PRINTLN(n_satelliti);
  }


  uscita_no_GPS:

  /*String check = String(Serial1.read()) + String(Serial1.read());           // Problema con checksum

  Serial.println(stringa);

  if(check == checksum(stringa)){
    return(stringa);
  } else {
    return("GPGGA,,,,,,,,,,,,,,*");                             // Se ci sono errori
  }*/

  return(stringa);
}





String trova_valore_ora(int posizione, String stringa){
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
    risultato = "999999";
  }

  return(risultato);
}
