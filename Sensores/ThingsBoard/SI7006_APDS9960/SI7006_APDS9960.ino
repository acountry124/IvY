//LIBRERIAS DE SENSORES
#include <Wire.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <SparkFun_APDS9960.h>

#define WIFI_AP "Depto 601"
#define WIFI_PASSWORD "17930953kK"
#define TOKEN "D1PBOtYOJZHVAUMY4W8x"

// DEFINIR PINES DE SENSORES
#define Addr_si7006 0x40

//DEFINIR PINES APDS
SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;

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

// Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 Ok!"));
  } else {
    Serial.println(F("Error APDS-9960"));
  }
  // Start running the APDS-9960 light sensor (no interrupts)
  if ( apds.enableLightSensor(false) ) {
    Serial.println(F("Comenzando Lecturas de Luz"));
  } else {
    Serial.println(F("Error lectura de Luz"));
  }
 
  delay(10);
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;
}

void loop()
{
  if ( !client.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 10000 ) { // Enviar datos cada 10 segundos
    getData();
    lastSend = millis();
  }
//  client.loop();
}


void getData() {

uint8_t data[2] = {0};

 // Read the light levels (ambient, red, green, blue)
  if (  !apds.readAmbientLight(ambient_light) ||
        !apds.readRedLight(red_light) ||
        !apds.readGreenLight(green_light) ||
        !apds.readBlueLight(blue_light) ) {
    Serial.println("Error reading light values");
  } else {
    Serial.println(" ");
    Serial.println("================= APDS 9960 =================");
    Serial.print("Ambiente: ");
    Serial.println(ambient_light);
    Serial.print(" Rojo: ");
    Serial.println(red_light);
    Serial.print(" Verde: ");
    Serial.println(green_light);
    Serial.print(" Azul: ");
    Serial.println(blue_light);
  }
  
  //Cálculo Humedad
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
          Serial.println(" ");
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
          String luzAmbiente = String(ambient_light);
          String rojo = String(red_light);
          String verde = String(green_light);
          String azul = String(blue_light);

          
          // Prepare a JSON payload string

 /*          String payload = "{";
  payload += "\"Temperatura\":"; payload += temperatura; payload += ", ";
  payload += "\"Humedad\":"; payload += humedad; payload += ", ";
  payload += "\"Luz Ambiente\":"; payload += luzAmbiente;
  //payload += "\"Luz Rojo\":"; payload += rojo;  payload += ", ";
  //payload += "\"Luz Verde\":"; payload += verde;  payload += ", ";
 // payload += "\"Luz Azul\":"; payload += azul; 
  payload += "}";
  */

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


  transmitPayload(payload1);
  transmitPayload(payload2);

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
