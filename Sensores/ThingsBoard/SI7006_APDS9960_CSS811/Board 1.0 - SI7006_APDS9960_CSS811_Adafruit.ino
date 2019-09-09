#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Adafruit_CCS811.h"
#include "Adafruit_APDS9960.h"

//CONNECTION SETTINGS
const char* ssid = "Depto 601";
const char* password = "17930953kK";
//const char* ssid = "Wireless";
//const char* password = "elbosque1122";


//SENSOR DEFINITIONS
#define Addr_si7006 0x40               //SI7006 TyH
Adafruit_APDS9960 apds;                //APDS-9960   
Adafruit_CCS811 ccs;                   //CSS811 Air Quality
boolean light_sensor_present=false;//APDS9960 Light Sensor
boolean TyH_sensor_present=false; //SI7006
boolean Air_quality_sensor_present=false; //CSS811 Air Quality Sensor


// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "45.236.129.97";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Timers auxiliar variables
long now=0;
long lastMeasure = 40000;

//Client Settings from server
String client_id,space_id,space_type_id,MAC;
float temp,ctemp,humidity;
String warm_pwm_lamp_topic,cold_pwm_lamp_topic;
//sensors variables
 //APDS getting data
 uint16_t r, g, b, c;
 int co2,tvoc;

// Lamp - LED - GPIO 4 = D2 on ESP-12E NodeMCU board
const int warm_pwm_lamp_pin = 12; //D6
const int cold_pwm_lamp_pin = 13; //D7

//----------------SETUP----------------------------------------
void setup() {
  lastMeasure = millis();
  Serial.begin(115200);
  setup_wifi();
  Wire.begin(2,14);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //GPIO Setup
//  pinMode(lamp1, OUTPUT);
  pinMode(warm_pwm_lamp_pin, OUTPUT);
  pinMode(cold_pwm_lamp_pin, OUTPUT);
 
  //SENSORS SETUP

  //SI7006 TyH Sensor
  if( TyH_sensor_present==true)
    {
    Wire.beginTransmission(Addr_si7006);  
    Wire.endTransmission();
    delay(300);
    }
  else
    { 
     ctemp=-22;
     humidity=-22;
    }
  //CSS811 Air Quality Sensor
  if(Air_quality_sensor_present==true)
    {
     Serial.println("Test de CSS811");            
     if(!ccs.begin())
      {
      Serial.println("Error! Chequear cableado");
      Air_quality_sensor_present=false;
      co2=-22;
      tvoc=-22; 
      }
      else
      {
      while(!ccs.available());                         
      float temp = ccs.calculateTemperature();
      ccs.setTempOffset(temp - 25.0);
      Air_quality_sensor_present=true;
      }
     }  
  //APDS9960 Light Sensor
  if(light_sensor_present==true)
  {
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
   }//end if
}//end setup

//-------------------LOOP---------------------------------------
// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {
    now = millis();

    if(!client.loop())
    {
    client.connect("IVY_Board3");
    }
      if (!client.connected()) {
    reconnect();
    }
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 30000) 
    {


   lastMeasure = millis();
   //collect Data
   if(light_sensor_present==true)      
   getRGB();
   if(TyH_sensor_present==true)           
   getTyH();
   if(Air_quality_sensor_present==true)  
   getAirQuality();

   //send telemetry

    // clean_variables();
    //sendMeasurements();
  } 
} //end loop

//----------------------------------------------------------------------------
void transmitPayload(String payload)
{
  // Convert Payload string to c-string and transmit
  char attributes[3000];
  payload.toCharArray(attributes, 3000);
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
  MAC = WiFi.macAddress();
  Serial.print("MAC: ");
  Serial.println(MAC);
} //end setup wifi

//------------------------------------------------------------------------------
// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  char bufferA[10];
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
    bufferA[i]= (char)message[i];
  }
  Serial.println();
  int bufferB = (bufferA[3] - '0') * 100 + (bufferA[1] - '0') * 10 + (bufferA[2] - '0') * 1;

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="room/lamp"){
      Serial.print("Changing Room lamp to ");
      if(messageTemp == "on"){
       // digitalWrite(lamp1, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
       // digitalWrite(lamp1, LOW);
        Serial.print("Off");
      }
      }
   if(topic==cold_pwm_lamp_topic)
      {
       Serial.print("\nSetting new PWM Value...\n");
       int auxx=messageTemp.toInt(); 
       Serial.print(auxx);
       analogWrite(cold_pwm_lamp_pin, auxx);
//       delay(2000);
//       analogWrite(cold_pwm_lamp_pin, bufferB);
      } 
   if(topic==warm_pwm_lamp_topic)
      {
       Serial.print("\nSetting new PWM Value...\n"); 
       int auxx=messageTemp.toInt(); 
       Serial.print(auxx);
       analogWrite(warm_pwm_lamp_pin, auxx);
      } 
  Serial.println();
} //end void callback
//--------------------------------------------------------------------------------------
// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
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
    if (client.connect("IVY_Board3")) {
     // Serial.println("connected. Subscribing...."); 
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      String aux;      
      char aux_char[500];
      aux = MAC; aux +="/Lamp_1"; 
      aux.toCharArray(aux_char,500);   
      client.subscribe(aux_char);
      warm_pwm_lamp_topic = MAC;  warm_pwm_lamp_topic +="/Warm_PWM";
      warm_pwm_lamp_topic.toCharArray(aux_char,500); 
      client.subscribe(aux_char);
      cold_pwm_lamp_topic = MAC;  cold_pwm_lamp_topic +="/Cold_PWM"; 
      cold_pwm_lamp_topic.toCharArray(aux_char,500);
      client.subscribe(aux_char);
     // Serial.println("\nSubscribed to all topics.\n"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  } //end while
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
   Serial.println("\n=================");
   Serial.print("Node Token/MAC: ");
   Serial.print(MAC);
   Serial.print("=================\n");
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
          String luz_ambiente = String(c);
          String rojo = String(r);
          String verde = String(g);
          String azul = String(b);
          String CO2 = String(co2);
          String TVCO = String(tvoc); 
          String CLIENT_ID = String(client_id);
          String SPACE_ID  = String(space_id);
          String SPACE_TYPE_ID = String(space_type_id);
          String MAC2;
          MAC2 += '"';
          MAC2 += MAC;          
          MAC2 += '"';
        //myString.toCharArray(buf, len)
    
        // Prepare a JSON payload string
 
     String payload1 = "{";
     payload1 += "\"Token\":";            payload1 += MAC2;               payload1 += ",";  
//     payload1 += "\"Client_ID\":";        payload1 += CLIENT_ID;          payload1 += ",";
//   payload1 += "\"Space_Tipe_ID\":";    payload1 += SPACE_TYPE_ID;      payload1 += ",";
//     payload1 += "\"Space_ID\":";         payload1 += SPACE_ID;           payload1 += ",";    
     payload1 += "\"Temperature\":";      payload1 += temperatura;        payload1 += ",";     
     payload1 += "\"Humidity\":";         payload1 += humedad;          
     payload1 += "}"; 

    char attr[500];
    payload1.toCharArray(attr,500);
    client.publish("t&h", attr);

     String payload2 = "{";
     payload2 += "\"Token\":";            payload2 += MAC2;               payload2 += ",";  
//     payload2 += "\"Client_ID\":";        payload2 += CLIENT_ID;          payload2 += ",";
//     payload2 += "\"Space_Tipe_ID\":";    payload2 += SPACE_TYPE_ID;      payload2 += ",";
//     payload2 += "\"Space_ID\":";         payload2 += SPACE_ID;           payload2 += ","; 
     payload2 += "\"Luz_Ambiente\":";     payload2 += luz_ambiente;       payload2 += ",";   
     payload2 += "\"Luz_Rojo\":";         payload2 += rojo;               payload2 += ",";
     payload2 += "\"Luz_Verde\":";        payload2 += verde;              payload2 += ",";
     payload2 += "\"Luz_Azul\":";         payload2 += azul;           
     payload2 += "}"; 

    payload2.toCharArray(attr,500);
    client.publish("Light", attr);
    
     String payload3 = "{";
     payload3 += "\"Token\":";            payload3 += MAC2;               payload3 += ",";  
 //    payload3 += "\"Client_ID\":";        payload3 += CLIENT_ID;          payload3 += ",";
//     payload3 += "\"Space_Tipe_ID\":";    payload3 += SPACE_TYPE_ID;      payload3 += ",";
//     payload3 += "\"Space_ID\":";         payload3 += SPACE_ID;           payload3 += ","; 
     payload3 += "\"CO2\":";              payload3 += CO2;                payload3 += ",";
     payload3 += "\"TVCO\":";             payload3 += TVCO;                
     payload3 += "}"; 
   
    payload3.toCharArray(attr,500);
    client.publish("Ambient", attr);

//  transmitPayload(payload1);
//  transmitPayload(payload2);
//  transmitPayload(payload3);
//  char attr[500];
//  payload4.toCharArray(attr, 500);
//  client.publish("", attr);


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
