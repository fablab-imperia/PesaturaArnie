
boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker
  //boolean status = mqtt.connect("GsmClientTest");

  // Or, if you want to authenticate MQTT:
  //boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");
  bool status = mqtt.connect("Mqtt_client", ADAFRUIT_USERNAME, AIO_KEY);

  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" OK");
  return mqtt.connected();
}

void mqtt_init(){

  pinMode(GSM_DTR, OUTPUT);         // Accendi modulo GPRS
  digitalWrite(GSM_DTR, LOW);
  delay(5);

  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);

  Serial.begin(115200);
  delay(10);

  SerialAT.begin(115200);
  delay(3000);
  
  Serial.println("Initializing modem...");
  modem.restart();
  Serial.println("Fatto!");

 // delay(10000);
//  SerialAT.write("AT+CPWROFF");
//  modem.poweroff();
  delay(10000);
  Serial.println("Info");
  String modemInfo = modem.getModemInfo();
  Serial.println("Modem: ");
  Serial.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    while (true);
  }
  SerialMon.println(" OK");

  SerialMon.print("Connecting to "); 
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    while (true);
  }
  SerialMon.println(" OK");

  delay(1000);
  scale.power_up();
  
}
