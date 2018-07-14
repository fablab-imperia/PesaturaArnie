#define SerialMon Serial
/*#define TINY_GSM_MODEM_SIM900
#define TINY_GSM_RX_BUFFER 512
#define SMS_TARGET  "+393493140191"
*/
#include "HX711.h"
//#include <TinyGsmClient.h>
#include <SoftwareSerial.h>



SoftwareSerial SIM900(7, 8); // RX, TX

const char apn[]  = "internet.wind";
const char user[] = "";
const char pass[] = "";

HX711 scale;
//TinyGsm modem(SerialAT);

String imei;

void setup() {

  SerialMon.begin(38400);

 // SerialAT.begin(38400);
  delay(3000);

   SIM900.begin(19200);
  // Give time to your GSM shield log on to network
  delay(20000);   
  

 /* 
SerialMon.println("OK0");
 // DBG("Initializing modem...");
  if (!modem.restart()) {
    SerialMon.println("OK4");
    delay(10000);
    return;
  }
SerialMon.println("OK1");
  String modemInfo = modem.getModemInfo();
  DBG("Modem:", modemInfo);


SerialMon.println("OK2");

 // DBG("Waiting for network...");
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

SerialMon.println("OK3");

  imei = modem.getIMEI();

 bool res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
*/
  SerialMon.println("Initializing the scale");
  // parameter "gain" is ommited; the default value 128 is used by the library
  // HX711.DOUT	- pin #A1
  // HX711.PD_SCK	- pin #A0
  scale.begin(A5, A0);

  scale.set_scale(-18200);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();				        // reset the scale to 0


  SerialMon.println("Readings:");
}

void loop() {
  SerialMon.print("one reading:\t");
  SerialMon.print(scale.get_units(), 1);
  SerialMon.print("\t| average:\t");
  SerialMon.println(scale.get_units(20), 1);

  scale.power_down();			        // put the ADC in sleep mode
  delay(500);
  scale.power_up();

  
  // bool res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
  //SerialMon.println(res);

  delay(2000);

SIM900.print("AT+CMGF=1\r"); 
  delay(100);
SerialMon.print("Inizio");
  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("AT + CMGS = \"+393493140191\""); 
  delay(100);
  SerialMon.print("Mezzo");
  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  SIM900.println(scale.get_units(20)); 
  delay(100);

SerialMon.print("Fine");
  // End AT command with a ^Z, ASCII code 26
  SIM900.println((char)26); 
  delay(100);
  SIM900.println();
  // Give module time to send SMS
  delay(60000); 

}
