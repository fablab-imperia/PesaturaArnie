/*String checksum(String messaggio){                                      // Calcola il checksum
  int i = 1;                                                            // Non funziona con il nuovo GPS
  char check = messaggio[0];
  while (messaggio[i] != '*'){
     check = check^messaggio[i];
     i++;
  }
  int b_int = check%16;
  int a_int = int(check/16);

  String cifre = "0123456789ABCDEF";
  String a = String(cifre[a_int]);
  String b = String(cifre[b_int]);
  
  return(a+b);
}*/

