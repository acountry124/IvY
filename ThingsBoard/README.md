# MQTT ThingsBoard

## Direcciones I2C:

  CSS811    | Calidad del Aire      | 0x5A
  APDS-9960 | Iluminación y RGB     | 0x39
  Si7006    | Temperatura y Humedad | 0x40
  

## ERROR ACTUAL:
Todas los archivos funcionan bien menos "SI7006_ADPS9960_CSS811", el error en Arduino IDE es:


In file included from /Users/juanrivera/Google Drive/NuLight V2/IvY/Desarrollo/Arduino/MQTT_Thingsboard_SI7006_APDS9960_CSS811/MQTT_Thingsboard_SI7006_APDS9960_CSS811.ino:5:0:
/Users/juanrivera/Documents/Arduino/libraries/APDS9960_ESP8266/src/SparkFun_APDS9960.h:31:33: error: expected unqualified-id before numeric constant
 #define ERROR                   0xFF
                                 ^
/Users/juanrivera/Documents/Arduino/libraries/Adafruit_CCS811-master/Adafruit_CCS811.h:143:21: note: in expansion of macro 'ERROR'
             uint8_t ERROR: 1;
                     ^
In file included from /Users/juanrivera/Google Drive/NuLight V2/IvY/Desarrollo/Arduino/MQTT_Thingsboard_SI7006_APDS9960_CSS811/MQTT_Thingsboard_SI7006_APDS9960_CSS811.ino:6:0:
/Users/juanrivera/Documents/Arduino/libraries/Adafruit_CCS811-master/Adafruit_CCS811.h: In member function 'void Adafruit_CCS811::status::set(uint8_t)':
/Users/juanrivera/Documents/Arduino/libraries/Adafruit_CCS811-master/Adafruit_CCS811.h:161:20: error: lvalue required as left operand of assignment
              ERROR = data & 0x01;
                    ^
Se encontraron varias bibliotecas para "Adafruit_CCS811.h"
Usado: /Users/juanrivera/Documents/Arduino/libraries/Adafruit_CCS811-master
 No usado: /Users/juanrivera/Documents/Arduino/libraries/CSS811
exit status 1
Error compilando para la tarjeta NodeMCU 1.0 (ESP-12E Module).



