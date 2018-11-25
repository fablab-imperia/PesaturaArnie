void allarme_GPS(){                                                                                         // Se l'arnia viene spostata...
  mqtt.publish(FEED_STATO, colore_allarme_gps);
  int controllo = 120;
  bool spostata = true;
  while(spostata){
    mqtt.publish(FEED_DEBUG, "Allarme GPS");
    GPS();                                                              // Controllo se la posizione GPS è affidabile
    unsigned long inizio_GPS = millis();
    while((latitud == 0.0 || longitud == 0.0) && millis()-inizio_GPS < 600000){
      GPS();
    }

    while(!mqtt.connected()){                                                                               // Se non sono connesso al boroker
      mqttConnect();                                                                                        // mi connetto
    }

    if (millis()-inizio_GPS < 300000){                                                                      // Se è affidabile la pubblico
      mqtt.publish(FEED_POSIZIONE, String("1,"+String(latitud, 15)+","+String(longitud, 15)+",2").c_str());
      if(abs(latitudine_casa-latitud)<0.005 && abs(longitudine_casa-longitud)<0.005){    // Se l'arnia è a posto
        controllo--;                                                                                        // controllo alre 2-1 volte
      } else {
        controllo = 120;                                                                                      // altrimenti continuo
        delay(60000);
      }
    }

    if (controllo == 0){spostata = false;}                                                        // Se per 120 volte consecutive la posizione è quella iniziale ritorno nel loop

  }
}

