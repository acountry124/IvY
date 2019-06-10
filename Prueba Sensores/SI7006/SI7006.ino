

// SI7006-A20 I2C 0x40

#include <Wire.h>

#define Addr_si7006 0x40

void setup()
{
  Wire.begin(2,14);
  Serial.begin(115200);

  Wire.beginTransmission(Addr_si7006);
  Wire.endTransmission();
  delay(300);
}

void loop()
{
    uint8_t data[2] = {0};
    
//Cálculo Humedad
    Wire.beginTransmission(Addr_si7006);
    // Para cambiar a Esclavo 0xE5
    Wire.write(0xF5);
    Wire.endTransmission();
    delay(500);
    
    Wire.requestFrom(Addr_si7006, 2);

    if(Wire.available() == 2)
    {
      data[0] = Wire.read();
      data[1] = Wire.read();
    }
    
    float humidity  = ((data[0] * 256.0) + data[1]);
    humidity = ((125 * humidity) / 65536.0) - 6;

//Cálculo Temperatura
    Wire.beginTransmission(Addr_si7006);
    Wire.write(0xF3);
    Wire.endTransmission();
    delay(500);
        Wire.requestFrom(Addr_si7006, 2);
    if(Wire.available() == 2)
    {
      data[0] = Wire.read();
      data[1] = Wire.read();
    }

    float temp  = ((data[0] * 256.0) + data[1]);
    float ctemp = ((175.72 * temp) / 65536.0) - 46.85;
    
//Output
    Serial.print("Humedad Relativa : ");
    Serial.print(humidity);
    Serial.println(" % RH");
    Serial.print("Temperatura : ");
    Serial.print(ctemp);
    Serial.println(" C");
    delay(1000);
}
