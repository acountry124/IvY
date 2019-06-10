//LIBRERIAS DE SENSORES
#include <Wire.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define WIFI_AP "Depto 601"
#define WIFI_PASSWORD "17930953kK"
#define TOKEN "0KIMQZJUvqWvpQAi4shh"

// DEFINIR PINES DE SENSORES
#define Addr_si7006 0x40

char thingsboardServer[] = "170.239.87.56";
WiFiClient wifiClient;

// INICIALIZAR SENSORES
PubSubClient client(wifiClient);
int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup()
{
  Serial.begin(115200);
  Wire.begin(2,14);

  //SENSORES
//Inicio SI7006
  Wire.beginTransmission(Addr_si7006);
  Wire.endTransmission();
  delay(300);

  delay(10);
  InitWiFi();
  client.setServer( thingsboardServer, 1883);
  lastSend = 0;
}

void loop()
{
  if ( !client.connected() ) {
    reconnect();
  }
  if ( millis() - lastSend > 10000 ) { // Envia datos cada 10 segundos
    getSI7006();
    lastSend = millis();
  }
  client.loop();
}


void getSI7006() {

uint8_t data[2] = {0};
  //Cálculo Humedad
      Wire.beginTransmission(Addr_si7006);
      Wire.write(0xF5); // Para cambiar a Esclavo 0xE5  
      Wire.endTransmission();
      delay(100);
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
          Serial.println("================= Si7006 =================");
          Serial.print("Humedad Relativa : ");
          Serial.print(humidity);
          Serial.println(" % RH");
          Serial.print("Temperatura : ");
          Serial.print(ctemp);
          Serial.println(" C");
          delay(100);


//conversión a Thingsboard
          String temperatura = String(ctemp);
          String humedad = String(humidity);
          
          // Prepare a JSON payload string
          String payload = "{";
          payload += "\"temperatura\":"; payload += temperatura; payload += ",";
          payload += "\"humedad\":"; payload += humedad; payload += "}";
        
          // Send payload
          char attributes[1000];
          payload.toCharArray( attributes, 1000 );
          client.publish( "v1/devices/me/telemetry", attributes );
          Serial.println( attributes );
}





    

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}
