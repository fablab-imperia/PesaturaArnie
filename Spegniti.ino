
void spegni_tutto(int ore, int minuti, int secondi){

  /* Change these values to set the current initial date */
  const byte day = 1;
  const byte month = 1;
  const byte year = 18;

  rtc.setDate(day, month, year);


  if(ore == 0 && minuti == 0 && secondi == 0){

    rtc.setAlarmDate(day + 0, month, year);     //------------------------------------------------------ +1

    ore = ore_default;
    minuti = minuti_default;
    secondi = secondi_default;

  } else {
    
    minuti = minuti + int(secondi/60);
    secondi = secondi%60;
    ore = ore + int(minuti/60);
    minuti = minuti%60;

    rtc.setAlarmDate(day + int(ore/24), month, year);

    ore = ore%24;
  }

//  rtc.setAlarmTime(int(secondi/3600)%24+1, int(secondi/60)%60, secondi%60);

  rtc.setAlarmTime(ore, minuti, secondi);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);

  Serial.print("Ore:  ");
  Serial.print(ore);
  Serial.print("  Minuti:  ");
  Serial.print(minuti);
  Serial.print("  Sec:  ");
  Serial.println(secondi);

  Serial.print("Ore attuale:  ");
  Serial.print(rtc.getHours());
  Serial.print("  Minuti attuali:  ");
  Serial.print(rtc.getMinutes());
  Serial.print("  Sec attuale:  ");
  Serial.println(rtc.getSeconds());

  mqtt.publish(FEED_DEBUG, String("Adesso: "+String(rtc.getHours())+":"+String(rtc.getMinutes())+":"+String(rtc.getSeconds())).c_str());
  mqtt.publish(FEED_DEBUG, String("Sveglia: "+String(ore)+":"+String(minuti)+":"+String(secondi)).c_str());

  SerialAT.write("AT+CPWROFF");
  scale.power_down();


  Serial.println("Sleep mode!!!");
  rtc.standbyMode();


  pinMode(GSM_DTR, OUTPUT);         // Accendi modulo GPRS
  digitalWrite(GSM_DTR, LOW);
  delay(5);

  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);

  
  delay(1000);       
  scale.power_up();
  delay(2000);
  
}
