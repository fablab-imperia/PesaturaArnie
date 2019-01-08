boolean mqttConnect() {
  unsigned long inizio_MQTT = millis();
  while(!mqtt.connected() && millis()-inizio_MQTT < timeOutMQTT){
    DEBUG_PRINT("mqtt()> Connecting to ");
    DEBUG_PRINT(broker);

    bool status = mqtt.connect("Mqtt_client", ADAFRUIT_USERNAME, AIO_KEY);

    if (status == false) {
      DEBUG_PRINTLN(" fail");
    }
    else {
      DEBUG_PRINTLN(" OK");
    }
  }
  return mqtt.connected();
}


bool Pubblica(String topic, String messaggio){
  check_RAM();
  int is = gsmAccess.status();
  DEBUG_PRINT("IS Access alive   ");
  DEBUG_PRINT(is);
  DEBUG_PRINTLN("");
  //DEBUG_PRINTLN("Pubblica()> Controllo connessione MQTT maggiore 0 connesso con problemi minore 0 disconnesso =0 OK ");
  
  DEBUG_PRINTLN("Pubblica() > Pubblico:    \"" + messaggio + "\"      su:   \"" + topic + "\"");
  bool riuscita = false;
  for(int i=0; i<3 && !riuscita; i++){
    DEBUG_PRINTLN("Pubblica()> Test Connessione");
    if (gsmAccess.status() != 3) init_GSM();
    mqttConnect();
    DEBUG_PRINT("IS Access alive   ");
    DEBUG_PRINT(gsmAccess.status());
    DEBUG_PRINTLN("");
    int x = gprs.ping("www.google.com");
    DEBUG_PRINT("Ping google in   ");
    DEBUG_PRINT(x);
    DEBUG_PRINTLN("  mS");
    DEBUG_PRINTLN("Pubblica()> Modem Connesso MQTT Connesso-------------------------------");
    riuscita = mqtt.publish(topic.c_str(), messaggio.c_str());
    DEBUG_PRINT("Pubblica() > " + String(i+1) + "° tentativo completato: ");
    if (riuscita){
      DEBUG_PRINTLN("RIUSCITO!");
    } else {
      DEBUG_PRINTLN("FALLITO!");
    }
  }
  if (riuscita){
    DEBUG_PRINTLN("Pubblica() > !!!!!!!!!!! INVIO RIUSCITO !!!!!!!!!!!");
  } else {
    DEBUG_PRINTLN("Pubblica() > !!!!!!!!!!! INVIO FALLITO !!!!!!!!!!!");
  }
  DEBUG_PRINTLN("Pubblica()> Disconnetto");
  
  DEBUG_PRINT("IS Access alive   ");
  DEBUG_PRINT(gsmAccess.status());
  DEBUG_PRINTLN("");
  check_RAM();
  DEBUG_PRINTLN("");
  return riuscita;
}

bool PubblicaLoop(String Batteria, String feed_Batteria, String Peso, String feed_Peso, String Posizione, String feed_Posizione, String Stato, String feed_Stato, String Messaggio, String feed_Debug){
  check_RAM();
  int is = gsmAccess.status();
  DEBUG_PRINTLN("PubblicaTutto()> Check Connessione: 3 OK ");
  DEBUG_PRINT("IS Access alive   ");
  DEBUG_PRINT(is);
  DEBUG_PRINTLN("");
  
  DEBUG_PRINTLN("Pubblica() > Pubblico:Batteria: " + Batteria);
  DEBUG_PRINTLN("Pubblica() > Pubblico:Peso: " + Peso);
  DEBUG_PRINTLN("Pubblica() > Pubblico:Posizione: " + Posizione);
  DEBUG_PRINTLN("Pubblica() > Pubblico:Stato: " + Stato);
  DEBUG_PRINTLN("Pubblica() > Pubblico:Debug: " + Messaggio);
  bool riuscita_batteria = false;
  bool riuscita_peso = false;
  bool riuscita_posizione = false;
  bool riuscita_stato = false;
  bool riuscita_debug = false;
  bool riuscita_trasmissione = false;
  
  for(int i=0; i<3 && !riuscita_trasmissione; i++){
    DEBUG_PRINTLN("Pubblica()> Test Connessione");
    if (gsmAccess.status() != 3) init_GSM();
    mqttConnect();
    DEBUG_PRINT("IS Access alive   ");
    DEBUG_PRINT(gsmAccess.status());
    DEBUG_PRINTLN("");
    int x = gprs.ping("www.google.com");
    DEBUG_PRINT("Ping google in   ");
    DEBUG_PRINT(x);
    DEBUG_PRINTLN("  mS");
    DEBUG_PRINTLN("Pubblica()> Modem Connesso MQTT Connesso-------------------------------");
    riuscita_batteria = mqtt.publish(FEED_BATTERIA, Batteria.c_str());
    riuscita_peso = mqtt.publish(FEED_PESO, Peso.c_str());
    riuscita_posizione = mqtt.publish(FEED_POSIZIONE, Posizione.c_str());
    riuscita_stato = mqtt.publish(FEED_STATO, Stato.c_str());
    riuscita_debug = mqtt.publish(FEED_DEBUG, Messaggio.c_str());
    DEBUG_PRINT("Pubblica() > " + String(i+1) + "° tentativo completato: ");
    if (riuscita_batteria && riuscita_peso && riuscita_posizione && riuscita_stato && riuscita_debug){
      DEBUG_PRINTLN("RIUSCITO!");
      riuscita_trasmissione = true;
    } else {
      DEBUG_PRINTLN("FALLITO!");
      riuscita_trasmissione = false;
    }
  }
  if (riuscita_trasmissione){
    DEBUG_PRINTLN("Pubblica() > !!!!!!!!!!! INVIO RIUSCITO !!!!!!!!!!!");
  } else {
    DEBUG_PRINTLN("Pubblica() > !!!!!!!!!!! INVIO FALLITO !!!!!!!!!!!");
  }
  DEBUG_PRINTLN("Pubblica()> Disconnetto");
  
  DEBUG_PRINT("IS Access alive   ");
  DEBUG_PRINT(gsmAccess.status());
  DEBUG_PRINTLN("");
  check_RAM();
  DEBUG_PRINTLN("");
  return riuscita_trasmissione;
}
