#include "HX711.h"

#define TINY_GSM_MODEM_UBLOX

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define SerialMon Serial
#define SerialAT SerialGSM

#define ADAFRUIT_USERNAME  "stefanosemeria"
#define AIO_KEY  "a0254d03dfdf4f35bbfd65874fc7b912"
#define FEED_PATH ADAFRUIT_USERNAME "/feeds/peso/"  

const char apn[]  = "internet.wind";
const char user[] = "";
const char pass[] = "";

const char* broker = "io.adafruit.com";

//const char* topic_peso = "Bilancia/Peso";

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

PubSubClient mqtt(broker, 1883, client);

//PubSubClient mqtt(client);

HX711 scale;

boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker
  //boolean status = mqtt.connect("GsmClientTest");

  // Or, if you want to authenticate MQTT:
  //boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");
  bool status = mqtt.connect(ADAFRUIT_USERNAME, AIO_KEY, "");

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

  SerialMon.begin(115200);
  delay(10);

  SerialAT.begin(115200);
  delay(3000);
  
  SerialMon.println("Initializing modem...");
  modem.restart();
  
  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem: ");
  SerialMon.println(modemInfo);

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
}

void setup() {

  SerialAT.begin(115200);

  mqtt_init();

//  mqtt.setServer(broker, 1883);
  

  SerialMon.println("Initializing the scale");
  // parameter "gain" is ommited; the default value 128 is used by the library
  // HX711.DOUT	- pin #A1
  // HX711.PD_SCK	- pin #A0
  scale.begin(A1, A0);

  scale.set_scale(-18200);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();				        // reset the scale to 0


}

void loop() {

  
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(20), 1);

  scale.power_down();			        // put the ADC in sleep mode
  delay(500);
  scale.power_up();

  
  // bool res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
  //Serial.println(res);

  delay(2000);

  float peso = scale.get_units(20);
  while(!mqtt.connected()){
    mqttConnect();
  }
  
 // mqtt.publish(topic_peso, String(peso).c_str());
  mqtt.publish(FEED_PATH, String(peso).c_str());
  
  delay(10000); 

}
