/*
Direcciones:
  CSS811    | Calidad del Aire      | 0x5A
  APDS-9960 | Iluminación y RGB     | 0x39
  Si7006    | Temperatura y Humedad | 0x40
  
  */

#include "Adafruit_CCS811.h"

Adafruit_CCS811 ccs;

void setup() {
  Serial.begin(115200);
  Wire.begin(2,14);

  
  Serial.println("Test de CSS811");                 // CSS811
  if(!ccs.begin()){
    Serial.println("Error! Chequear cableado");
    while(1);
  }
  while(!ccs.available());                          //Calibración Sensor
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);



  
}

void loop() {

  if(ccs.available()){                              // CSS811
    float temp = ccs.calculateTemperature();
      Serial.println("Valores CSS811");                
    if(!ccs.readData()){
      Serial.print("CO2: ");
      Serial.print(ccs.geteCO2());
      Serial.print("ppm, TVOC: ");
      Serial.print(ccs.getTVOC());
      Serial.print("ppb   Temp:");
      Serial.println(temp);
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  }
  delay(500);


  


  
}
