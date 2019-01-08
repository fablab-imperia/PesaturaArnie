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
  DEBUG_PRINTLN("Pubblica()> Controllo connessione MQTT maggiore 0 connesso con problemi minore 0 disconnesso =0 OK ");
  
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
