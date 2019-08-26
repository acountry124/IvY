//lIBRARIES
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "Adafruit_CCS811.h"
#include "Adafruit_APDS9960.h"

//CONNECTION SETTINGS
#define WIFI_AP "Depto 601"
#define WIFI_PASSWORD "17930953kK"
#define TOKEN "jXVhytZL6cV0B0Nw5eZZ"   //TOKEN RECIEVED BY THINGSBOARD TO IDENTIFY THE SENSOR
const char* mqtt_server = "45.236.129.97";

//SENSOR DEFINITIONS
#define Addr_si7006 0x40               //SI7006
Adafruit_APDS9960 apds;                //APDS-9960   
Adafruit_CCS811 ccs;                   //CSS811

//SERVER INFO
//char thingsboardServer[] = "170.239.87.56";
WiFiClient wifiClient;
PubSubClient client(wifiClient);
int status = WL_IDLE_STATUS;
long lastSend=0;//unsigned long lastSend;
long now;
// clock watcher
unsigned long time_to_reset = 3600000;

//-----------------------------------------------------------------------
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
  if(!ccs.begin())
  {
    Serial.println("Error! Chequear cableado");
    while(1);
  }
  while(!ccs.available());                         
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
  
//APDS9960
 if(!apds.begin())
 {
  Serial.println("failed to initialize device! Please check your wiring.");
 }
 else Serial.println("Device initialized!");
 apds.enableColor(true);
   
 client.setServer(mqtt_server, 1883); //to send data to server
 //client.setCallback(callback); //to suscribe a topic
 lastSend = 0;
} //end setup

//----------------------------------------------------------------------------
void loop()
{
  // if millis() reached time_to_reset (1h) restart ESP
  if(millis() >= time_to_reset) 
  {
  ESP.restart();
  }
  if ( !client.connected() ) 
  {
  reconnect();
  }
  if(!client.loop())
    client.connect("room");
  
  now = millis();
  if (now - lastSend > 10000 ) // Send data every 10 seconds
  { 
  getData();
  lastSend = millis();
  }
  
 } //end loop

//FUNCTION DEFINITIONS------------------------------------------------------
void getData() 
 {
 uint8_t data[2] = {0};
  //APDS getting data
  uint16_t r, g, b, c;
  
  //wait for color data to be ready
  while(!apds.colorDataReady())
 {
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
 if(ccs.available())
 {                             
    float temp = ccs.calculateTemperature();
    Serial.println("Valores CSS811");                
    if(!ccs.readData())
    {
      Serial.print("CO2: ");
      Serial.print(ccs.geteCO2());
      Serial.print("ppm, TVOC: ");
      Serial.print(ccs.getTVOC());
      Serial.print("ppb   Temp:");
      Serial.println(temp);
    }
    else
    {
      Serial.println("ERROR!");
      while(1);
    }
  }
  
  
////Convert variables for Json
//          String temperatura = String(ctemp);
//          String humedad = String(humidity);
//          String luzAmbiente = String(c);
//          String rojo = String(r);
//          String verde = String(g);
//          String azul = String(b);
//          String CO2 = String(ccs.geteCO2());
//          String TVCO = String(ccs.getTVOC());

        static char temperatura[7];
        dtostrf(ctemp, 6, 2, temperatura);
        static char humedad[7];
        dtostrf(humidity, 6, 2, humedad);
        static char luzAmbiente[7];
        dtostrf(c, 6, 2,luzAmbiente );
        static char rojo[7];
        dtostrf(r, 6, 2,rojo );
        static char verde[7]; 
        dtostrf(g, 6, 2,verde );
        static char azul[7];
        dtostrf(b, 6, 2,azul );
        static char CO2 [7];
        dtostrf(ccs.geteCO2(), 6, 2,CO2 );
        static char TVCO[7]; 
        dtostrf(ccs.getTVOC(), 6, 2,TVCO );
 //   static char humidityTemp[7];
 //   dtostrf(h, 6, 2, humidityTemp);

    // Publishes values
    client.publish("ivy-001/room/temperature", temperatura);
    client.publish("ivy-001/room/humidity", humedad);
    client.publish("ivy-001/room/light", luzAmbiente);
    client.publish("ivy-001/room/red", rojo);
    client.publish("ivy-001/room/green", verde);
    client.publish("ivy-001/room/blue", azul);
    client.publish("ivy-001/room/CO2", CO2);
    client.publish("ivy-001/room/TVCO", TVCO);


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


void reconnect() 
{
  // Loop until we're reconnected to Wifi AP
  while (!client.connected()) 
    {
    status = WiFi.status();
    if ( status != WL_CONNECTED) 
       {
        WiFi.begin(WIFI_AP, WIFI_PASSWORD);
        while (WiFi.status() != WL_CONNECTED) 
          {
          delay(500);
          Serial.print(".");
          }
        Serial.println("Connected to AP");
        }
// Loop until we're reconnected to Server
  while (!client.connected()) 
     {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (client.connect("ESP8266Client")) {
     You can do it like this:
       if (client.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
      if (client.connect("room")) 
        {
         Serial.println("connected");  
         // Subscribe or resubscribe to a topic
         // You can subscribe to more topics (to control more LEDs in this example)
         //client.subscribe("room/lamp");
         } 
         else 
         {
         Serial.print("failed, rc=");
         Serial.print(client.state());
         Serial.println(" try again in 5 seconds");
         // Wait 5 seconds before retrying
         delay(5000);
         }
   }    
    

  }
}
