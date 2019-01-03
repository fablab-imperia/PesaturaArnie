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
