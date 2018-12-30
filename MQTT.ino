
boolean mqttConnect() {
  unsigned long inizio_MQTT = millis();
  while(!mqtt.connected() && millis()-inizio_MQTT < timeOutMQTT){
    DEBUG_PRINT("Connecting to ");
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
    DEBUG_PRINTLN(" OK");
//    return mqtt.connected();
  }
  return mqtt.connected();
}

void mqtt_init(){

  DEBUG_PRINTLN("mqtt_init()> Accendo Modulo GPRS");
  pinMode(GSM_DTR, OUTPUT);         // Accendi modulo GPRS
  digitalWrite(GSM_DTR, LOW);
  delay(5);

  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);

  Serial.begin(115200);
  delay(10);

//  SerialAT.begin(115200);
  SerialAT.begin(9600);
  delay(3000);
  
  DEBUG_PRINTLN("mqtt_init()> Inizializzo Modem");
  modem.restart();

 // delay(10000);
//  SerialAT.write("AT+CPWROFF");
//  modem.poweroff();
  delay(10000);

  String modemInfo = modem.getModemInfo();
  DEBUG_PRINT("mqtt_init()>  Modello Modem: ");
  DEBUG_PRINTLN(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  DEBUG_PRINTLN("mqtt_init()> Aggancio la rete cellulare...");
  if (!modem.waitForNetwork()) {
    DEBUG_PRINTLN("mqtt_init()> ERRORE non trovo rete cellulare... ");
    return;
  }

  if (modem.isNetworkConnected()) {
    DEBUG_PRINTLN("mqtt_init()> Rete cellulare OK");
  }
  
  
  DEBUG_PRINTLN("mqtt_init()> Tento connessione ad APN GPRS ");
  DEBUG_PRINT(apn);
  DEBUG_PRINT(" con utente ");
  DEBUG_PRINT(user);
  DEBUG_PRINT(" e pwd ");
  DEBUG_PRINTLN(user); 

  if (!modem.gprsConnect(apn, user, pass)) {
    DEBUG_PRINTLN("mqtt_init()> ERRORE fallita connessione APN GPRS");
  }

  DEBUG_PRINTLN("mqtt_init()> INFORMAZIONI RECUPERATE DA RETE CELLULARE");
  String cop = modem.getOperator();
  
  DEBUG_PRINT("mqtt_init()> OPERATORE ");
  DEBUG_PRINTLN(cop);
  
  IPAddress local = modem.localIP();
  
  DEBUG_PRINT("mqtt_init()> Indirizzo IP ");
  DEBUG_PRINTLN(local);

  int csq = modem.getSignalQuality();
  DEBUG_PRINT("mqtt_init()> Segnale:  ");
  DEBUG_PRINTLN(csq);

  delay(1000);

  DEBUG_PRINTLN("mqtt_init()> accensione bilancia...");
  scale.power_up();
  
}
