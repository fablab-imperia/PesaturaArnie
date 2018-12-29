#include <RTCZero.h>

RTCZero rtc;
void ISR();

const byte seconds = 0;
const byte minutes = 0;
const byte hours = 0;

const byte day = 28;
const byte month = 6;
const byte year = 18;

void setup() {
  delay(5000);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);
  
  rtc.setAlarmTime(00,00,20);
  
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.attachInterrupt(ISR);

  
}

void loop() {
  rtc.setTime(hours, minutes, seconds);
  rtc.setAlarmTime(00,00,20);
  rtc.standbyMode();
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}

void ISR(){
  digitalWrite(LED_BUILTIN, HIGH);
}
