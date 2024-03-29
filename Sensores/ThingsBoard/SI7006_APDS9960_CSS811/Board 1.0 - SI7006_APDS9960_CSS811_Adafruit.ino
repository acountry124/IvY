//lIBRARIES
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "Adafruit_CCS811.h"
#include "Adafruit_APDS9960.h"

//CONNECTION TO THINGSBOARD
#define WIFI_AP "Depto 601"
#define WIFI_PASSWORD "17930953kK"
#define TOKEN "eFTROXN0Jsjsvb1hhOQn"   //TOKEN RECIEVED BY THINGSBOARD TO IDENTIFY THE SENSOR

//DEFINITION SENSORS
#define Addr_si7006 0x40               //SI7006
Adafruit_APDS9960 apds;                //APDS-9960   
Adafruit_CCS811 ccs;                   //CSS811

//SERVER INFO
char thingsboardServer[] = "demo.thingsboard.io";
WiFiClient wifiClient;
PubSubClient client(wifiClient);
int status = WL_IDLE_STATUS;
unsigned long lastSend;

// clock watcher
unsigned long time_to_reset = 3600000;

void setup()
{
  Serial.begin(115200);
  Wire.begin(2,14);

//SENSORS SETUP

//SI7006
  Wire.beginTransmission(Addr_si7006);  
  Wire.endTransmission();
  delay(300);
  
//CSS811
  Serial.println("Test de CSS811");            
  if(!ccs.begin()){
    Serial.println("Error! Chequear cableado");
    while(1);
  }
  while(!ccs.available());                         
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
  
//APDS9960
 if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  else Serial.println("Device initialized!");
  apds.enableColor(true);
   
  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;
} //end setup


void loop()
{
  // if millis() reached time_to_reset (1h) restart ESP
  if(millis() >= time_to_reset) {
    ESP.restart();
  }
  if ( !client.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 10000 ) { // Enviar datos cada 10 segundos
    getData();
    lastSend = millis();
  }
client.loop();
} //end loop


void getData() {

uint8_t data[2] = {0};

  //APDS getting data
  uint16_t r, g, b, c;
  
  //wait for color data to be ready
  while(!apds.colorDataReady()){
    delay(5);
  }

  //get the data and print the different channels
  apds.getColorData(&r, &g, &b, &c);
  Serial.print("Rojo: ");
  Serial.print(r);
  Serial.print(" Verde: ");
  Serial.print(g);
  Serial.print(" Azul: ");
  Serial.print(b);
  Serial.print(" Luz Ambiente: ");
  Serial.println(c);
  Serial.println();
  
  delay(200);
  
//SI7006 HUMIDITY
      Wire.beginTransmission(Addr_si7006);
      Wire.write(0xF5);       // Para cambiar a Esclavo 0xE5
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

//SI7006 TEMPERATURA
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
    
//Print Values
          Serial.println(" ");
          Serial.println("================= Si7006 =================");
          Serial.print("Humedad Relativa : ");
          Serial.print(humidity);
          Serial.println(" % RH");
          Serial.print("Temperatura : ");
          Serial.print(ctemp);
          Serial.println(" C");
          delay(100);

  
  
//CSS811
 if(ccs.available()){                             
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
  


  
//Convert variables for Json
          String temperatura = String(ctemp);
          String humedad = String(humidity);
          String luzAmbiente = String(c);
          String rojo = String(r);
          String verde = String(g);
          String azul = String(b);
          String CO2 = String(ccs.geteCO2());
          String TVCO = String(ccs.getTVOC());

          
          // Prepare a JSON payload string

     String payload1 = "{";
  payload1 += "\"Temperatura\":";      payload1 += temperatura;     payload1 += ", ";
  payload1 += "\"Humedad\":";          payload1 += humedad;         payload1 += ", ";
  payload1 += "\"Luz Ambiente\":";     payload1 += luzAmbiente;
  payload1 += "}";

   String payload2 = "{";
  payload2 += "\"Luz Rojo\":";         payload2 += rojo;             payload2 += ", ";
  payload2 += "\"Luz Verde\":";        payload2 += verde;            payload2 += ", ";
  payload2 += "\"Luz Azul\":";         payload2 += azul; 
  payload2 += "}";

   String payload3 = "{";
  payload3 += "\"CO2\":";              payload3 += CO2;              payload3 += ",";
  payload3 += "\"TVCO\":";             payload3 += TVCO;                
  payload3 += "}";
  

  transmitPayload(payload1);
  transmitPayload(payload2);
  transmitPayload(payload3);

}

void transmitPayload(String payload)
{
  // Convert Payload string to c-string and transmit
  char attributes[500];
  payload.toCharArray(attributes, 500);
  client.publish("v1/devices/me/telemetry", attributes);
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
