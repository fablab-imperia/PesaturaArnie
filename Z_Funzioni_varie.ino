/*double split(String stringa, bool n){               Tutto questo non serve più a niente
  String out = "";
  int i = 0;
  Serial.println(n);
  Serial.println(stringa);
  if(n==0){
    Serial.println("hjgdfggdhfhku");
    while(stringa[i] != ','){
      Serial.println(i);
      Serial.println(stringa[i]);
      out = out + stringa[i];
      i++;
    }
  } else {
//    Serial.println("kokkjkjijiji");
    while(stringa[i] != ','){
      Serial.print("I:   ");
      Serial.println(i);
      Serial.print("C:   ");
      Serial.println(stringa[i]);
      Serial.print("S:   ");
      Serial.println(out);
      i++;
    }
    i++;
//    Serial.print("paasssato");
    while(i<stringa.length()){
/*      Serial.print("I:   ");
      Serial.println(i);
      Serial.print("C:   ");
      Serial.println(stringa[i]);
      Serial.print("S:   ");
      Serial.println(out);
      out = out + stringa[i];
      i++;
    }
  }

  return(out.toDouble());
}*/



