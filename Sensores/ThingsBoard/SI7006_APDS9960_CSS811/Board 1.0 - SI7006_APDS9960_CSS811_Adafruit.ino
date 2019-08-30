#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>
#include "Adafruit_CCS811.h"
#include "Adafruit_APDS9960.h"

//CONNECTION SETTINGS
const char* ssid = "Depto 601";
const char* password = "17930953kK";


//SENSOR DEFINITIONS
#define Addr_si7006 0x40               //SI7006 TyH
Adafruit_APDS9960 apds;                //APDS-9960   
Adafruit_CCS811 ccs;                   //CSS811 Air Quality
boolean light_sensor_present=false;
boolean TyH_sensor_present=false;
boolean Air_quality_sensor_present=false;


// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "45.236.129.97";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

//Client Settings from server
String client_id,space_id,space_type_id;
  float temp,ctemp,humidity;

//sensors variables
 //APDS getting data
 uint16_t r, g, b, c;
 int co2,tvoc;
//----------------SETUP----------------------------------------
void setup() {
  
  Serial.begin(115200);
  setup_wifi();
  Wire.begin(2,14);
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);

//SENSORS SETUP

//SI7006 TyH Sensor
  Wire.beginTransmission(Addr_si7006);  
  Wire.endTransmission();
  delay(300);
  ctemp=-22;
  humidity=-22;
  
//CSS811 Air Quality Sensor
  Serial.println("Test de CSS811");            
  if(!ccs.begin())
  {
    Serial.println("Error! Chequear cableado");
    Air_quality_sensor_present=false;
    co2=-22;
    tvoc=-22; 
  }
  while(!ccs.available());                         
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
  Air_quality_sensor_present=true;
  
//APDS9960 Light Sensor
 if(!apds.begin())
 {
  Serial.println("failed to initialize device! Please check your wiring.");
  light_sensor_present=false;
  c=-22;
  r=-22;
  g=-22;
  b=-22;
 }
 else Serial.println("Device initialized!");
 apds.enableColor(true);
 light_sensor_present=true;
   
}//end setup

//-------------------LOOP---------------------------------------
// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {
  now = millis();
  
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 30000) {
    lastMeasure = now;
//    if (!client.connected()) {
//    reconnect();
//    }
    if(!client.loop())
    client.connect("IVY_Board");

   //collect Data
   if(light_sensor_present==true)         getRGB();
   if(TyH_sensor_present==true)           getTyH();
   if(Air_quality_sensor_present==true)   getAirQuality();

   //send telemetry
   sendMeasurements();
   clean_variables();
  } 
} //end loop


void transmitPayload(String payload)
{
  // Convert Payload string to c-string and transmit
  char attributes[500];
  payload.toCharArray(attributes, 500);
  client.publish("telemetry", attributes);
}

//---------------------------------------------------------------------------
// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
//  if(topic=="room/lamp"){
//      Serial.print("Changing Room lamp to ");
//      if(messageTemp == "on"){
//        digitalWrite(lamp, HIGH);
//        Serial.print("On");
//      }
//      else if(messageTemp == "off"){
//        digitalWrite(lamp, LOW);
//        Serial.print("Off");
//      }
//  }
  Serial.println();
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
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
//    if (client.connect("room")) {
//      Serial.println("connected");  
//      // Subscribe or resubscribe to a topic
//      // You can subscribe to more topics (to control more LEDs in this example)
//      client.subscribe("room/lamp");
//    } else {
//      Serial.print("failed, rc=");
//      Serial.print(client.state());
//      Serial.println(" try again in 5 seconds");
//      // Wait 5 seconds before retrying
//      delay(5000);
//    }
  }
} //end reconnect

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs


//FUNCTION DEFINITIONS------------------------------------------------------
 void getRGB()
 {

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
 }//end getRGB()
//-----------------------------------------------------
 void getTyH()
 {
  uint8_t data[2] = {0};
  //SI7006 HUMIDITY---------------
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
  humidity  = ((data[0] * 256.0) + data[1]);
  humidity = ((125 * humidity) / 65536.0) - 6;

  //SI7006 TEMPERATURE-------------
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
   temp  = ((data[0] * 256.0) + data[1]);
   ctemp = ((175.72 * temp) / 65536.0) - 46.85;
    
   //Print Values------------
   Serial.println(" ");
   Serial.println("================= Si7006 =================");
   Serial.print("Humedad Relativa : ");
   Serial.print(humidity);
   Serial.println(" % RH");
   Serial.print("Temperatura : ");
   Serial.print(ctemp);
   Serial.println(" C");
   delay(100);

 }//end getTyH()
 
//---------------------------------------------------------
 void getAirQuality()
 {
//CSS811
 if(ccs.available())
 {                             
    float temp = ccs.calculateTemperature();

    Serial.println("Valores CSS811");                
    if(!ccs.readData())
    {
      Serial.print("CO2: ");
      co2=ccs.geteCO2();
      Serial.print(co2);
      Serial.print("ppm, TVOC: ");
      tvoc=ccs.getTVOC();
      Serial.print(tvoc);
      Serial.print("ppb   Temp:");
      Serial.println(temp);
    }
    else
    {
      Serial.println("ERROR!");
      while(1);
    }
  }//end if
 }//end getAirQuality()
 //-------------------------------------------------------
 void sendMeasurements()
 {
////Convert variables for Json
          String temperatura = String(ctemp);
          String humedad = String(humidity);
          String luzAmbiente = String(c);
          String rojo = String(r);
          String verde = String(g);
          String azul = String(b);
          String CO2 = String(co2);
          String TVCO = String(tvoc); 
          String CLIENT_ID = String(client_id);
          String SPACE_ID  = String(space_id);
          String SPACE_TYPE_ID = String(space_type_id);
          String MAC = WiFi.macAddress();
          String MAC2;
          MAC2 += '"';
          MAC2 += MAC;          
          MAC2 += '"';
        //myString.toCharArray(buf, len)
    
        // Prepare a JSON payload string

     String payload1 = "{";
     payload1 += "\"Token\":";            payload1 += MAC2;               payload1 += ", ";  
     payload1 += "\"Client_ID\":";        payload1 += CLIENT_ID;          payload1 += ", ";
     payload1 += "\"Space_ID\":";         payload1 += SPACE_ID;           payload1 += ", ";  
     payload1 += "\"Space_Tipe_ID\":";    payload1 += SPACE_TYPE_ID;      payload1 += ", ";
     payload1 += "\"Temperatura\":";      payload1 += temperatura;        payload1 += ", ";
     payload1 += "\"Humedad\":";          payload1 += humedad;            payload1 += ", ";
     payload1 += "\"Luz Ambiente\":";     payload1 += luzAmbiente;        payload1 += ", ";
     payload1 += "\"Luz Rojo\":";         payload1 += rojo;               payload1 += ", ";
     payload1 += "\"Luz Verde\":";        payload1 += verde;              payload1 += ", ";
     payload1 += "\"Luz Azul\":";         payload1 += azul;               payload1 += ", ";
     payload1 += "\"CO2\":";              payload1 += CO2;                payload1 += ",";
     payload1 += "\"TVCO\":";             payload1 += TVCO; 
     payload1 += "}";

//   String payload2 = "{";
//  payload2 += "\"Luz Rojo\":";         payload2 += rojo;             payload2 += ", ";
//  payload2 += "\"Luz Verde\":";        payload2 += verde;            payload2 += ", ";
//  payload2 += "\"Luz Azul\":";         payload2 += azul; 
//  payload2 += "}";
//
//   String payload3 = "{";
//  payload3 += "\"CO2\":";              payload3 += CO2;              payload3 += ",";
//  payload3 += "\"TVCO\":";             payload3 += TVCO;                
//  payload3 += "}";
//
//   String payload4 = "{";
//   payload4 += "\"Temperature\":";     payload4 += temperatura;                      payload4 += ",";
//   payload4 += "\"Token\":";           payload4 += MAC2;                              payload4 += ", ";  
//   payload4 += "\"Client_ID\":";       payload4 += CLIENT_ID;                        payload4 += ", ";
//   payload4 += "\"Space_ID\":";        payload4 += SPACE_ID;                         payload4 += ", ";  
//   payload4 += "\"Space_Tipe_ID\":";   payload4 += SPACE_TYPE_ID;         
//   payload4 += "}";

  transmitPayload(payload1);
//  transmitPayload(payload2);
//  transmitPayload(payload3);
//  char attr[500];
//  payload4.toCharArray(attr, 500);
//  client.publish("telemetry", attr);


 }//end sendMeasurements()
 //-------------------------------------------------------
 void clean_variables()
 {
  c=-22;
  r=-22;
  g=-22;
  b=-22;
  co2=-22;
  tvoc=-22; 
  ctemp=-22;
  humidity=-22;
  } 
//--------------------------------------------------------------
