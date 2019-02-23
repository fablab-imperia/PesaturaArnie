float livello_batteria(){                                                           // Controlla lo stato della batteria...
  float min_batteria = 3.6;                                                         // La tensione massima della batteria
  float max_batteria = 4.2;                                                         // La tensione minima della batteria
  tens = analogRead(ADC_BATTERY)* (4.3 / 1023.0);                             // Leggo la tensione della batteria
  
  
//  DEBUG_PRINTLN(100*(val-max_batteria)/(max_batteria-min_batteria)+100);
  return (100*(tens-max_batteria)/(max_batteria-min_batteria)+100);                  // Ritorna la percentuale della batteria, considerando
}                                                                                   // 0% min_batteria e 100% max_batteria
