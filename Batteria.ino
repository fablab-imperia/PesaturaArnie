float livello_batteria(){                                                           // Controlla lo stato della batteria...
  float min_batteria = 3.2;                                                           // Il valore max della batteria (tra 0 e 1023)
  float max_batteria = 4.4;                                                           // Il valore max della batteria (tra 0 e 1023)
  float val = analogRead(ADC_BATTERY)* (4.3 / 1023.0);                                                         // La batteria è collegata al pin analogico A3
  
  Serial.println(val);
//  Serial.println(100*(val-max_batteria)/(max_batteria-min_batteria)+100);
  return (100*(val-max_batteria)/(max_batteria-min_batteria)+100);                  // Ritorna la percentuale della batteria, considerando
}                                                                                   // 0% min_batteria e 100% max_batteria
