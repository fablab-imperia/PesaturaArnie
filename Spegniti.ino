
void spegni_tutto(byte ora_sveglia, byte minuti_sveglia, byte secondi_sveglia, byte giorni){

/*
  const byte day = 10;
  const byte month = 10;
  const byte year = 18;

  DEBUG_PRINT("spegni_tutto() > Setto data a ");
  DEBUG_PRINT(day);
  DEBUG_PRINT("/");
  DEBUG_PRINT(month);
  DEBUG_PRINT("/");
  DEBUG_PRINTLN(year);
  
//  rtc.setDate(day, month, year);

  #ifdef DEBUG
    rtc.setAlarmDate(day, month, year);
    
    DEBUG_PRINT("spegni_tutto() > Setto data sveglia a ");
    DEBUG_PRINT(day);
    DEBUG_PRINT("/");
    DEBUG_PRINT(month);
    DEBUG_PRINT("/");
    DEBUG_PRINTLN(year);
  #else
    rtc.setAlarmDate(day + 1, month, year);

    DEBUG_PRINT("spegni_tutto() > Setto data sveglia a ");
    DEBUG_PRINT(day + 1);
    DEBUG_PRINT("/");
    DEBUG_PRINT(month);
    DEBUG_PRINT("/");
    DEBUG_PRINTLN(year);
  #endif
*/
  
//  rtc.setAlarmTime(ora_sveglia, minuti_sveglia, secondi_sveglia);
  
//  rtc.enableAlarm(rtc.MATCH_HHMMSS);

  DEBUG_PRINT("spegni_tutto() > Setto ora sveglia a ");
  DEBUG_PRINT(ora_sveglia);
  DEBUG_PRINT(":");
  DEBUG_PRINT(minuti_sveglia);
  DEBUG_PRINT(":");
  DEBUG_PRINTLN(secondi_sveglia);

  DEBUG_PRINT("spegni_tutto() > Ora attuale ");
  DEBUG_PRINT(rtc.getHours());
  DEBUG_PRINT(":");
  DEBUG_PRINT(rtc.getMinutes());
  DEBUG_PRINT(":");
  DEBUG_PRINTLN(rtc.getSeconds());

//  mqtt.publish(FEED_DEBUG, String("Adesso: "+String(rtc.getHours())+":"+String(rtc.getMinutes())+":"+String(rtc.getSeconds())).c_str());
//  mqtt.publish(FEED_DEBUG, String("Sveglia: "+String(ore)+":"+String(minuti)+":"+String(secondi)).c_str());

  SerialAT.write("AT+CPWROFF");
  scale.power_down();


  DEBUG_PRINTLN("Sleep!!!");
//  rtc.standbyMode();

/*
  long tempo = domani*24*60*60 + (ora_sveglia-rtc.getHours())*60*60 + (minuti_sveglia-rtc.getMinutes())*60 + (secondi_sveglia-rtc.getSeconds());

  if (tempo < 0){
    domani = 1;
    tempo = domani*24*60*60 + (ora_sveglia-rtc.getHours())*60*60 + (minuti_sveglia-rtc.getMinutes())*60 + (secondi_sveglia-rtc.getSeconds());
  }

  arnia_sollevata = false;

  unsigned long inizio_sleep = millis();
  while (tempo*1000 >= millis() - inizio_sleep){
    DEBUG_PRINTLN("spegni_tutto() > Aspetto");
    delay(10000);

    if (arnia_sollevata){
      allarme();
    }
  }
*/

  rtc.setDate(1, 1, 18);
  
  while (rtc.getDay()<giorni+1 || rtc.getHours()<ora_sveglia || rtc.getMinutes()<minuti_sveglia || rtc.getSeconds()<secondi_sveglia){
    DEBUG_PRINTLN("spegni_tutto() > Aspetto");
    delay(10000);

    if (arnia_sollevata){
      allarme();
    }
  }

  
}




void riaccendi_tutto(){  
  pinMode(GSM_DTR, OUTPUT);                 // Accendi modulo GPRS
  digitalWrite(GSM_DTR, LOW);
  delay(5);

  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);

  
  delay(1000);       
  scale.power_up();
  delay(2000);

  mqtt_init();
}



void sollevata(){
  arnia_sollevata = true;
}
