boolean mqttConnect() {
  unsigned long inizio_MQTT = millis();
  while(!mqtt.connected() && millis()-inizio_MQTT < timeOutMQTT){
    DEBUG_PRINT("mqtt()> Connecting to ");
    DEBUG_PRINT(broker);

  // Connect to MQTT Broker
  //boolean status = mqtt.connect("GsmClientTest");

  // Or, if you want to authenticate MQTT:
  //boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");
    bool status = mqtt.connect("Mqtt_client", ADAFRUIT_USERNAME, AIO_KEY);

    if (status == false) {
      DEBUG_PRINTLN(" fail");
//      return false;
    }
    else {
      DEBUG_PRINTLN(" OK");
    }
//    return mqtt.connected();
  }
  return mqtt.connected();
}


bool Pubblica(String topic, String messaggio){
  DEBUG_PRINTLN("Pubblica() > Pubblico:    \"" + messaggio + "\"      su:   \"" + topic + "\"");
  bool riuscita = false;
  for(int i=0; i<3 && !riuscita; i++){
    if (!mqtt.connected()){
      DEBUG_PRINTLN("Pubblica() > ||||||||||Modem Disconnesso Riconetto!!!!!!!!!");
      init_GSM();
      riuscita = mqttConnect();
    }
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
  return riuscita;
}
