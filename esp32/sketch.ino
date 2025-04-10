/**
   PUB
*/

#include "DHTesp.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int DHT_PIN = 15; //Sensor 1
int DHT_PIN2 = 18; //Sensor 2

//Configuración de los tópicos MQTT
#define TOPIC_PUBLISH_TEMPERATURE "temperatura"
#define TOPIC_PUBLISH_HUMIDITY    "humedad"

#define HOST "6.tcp.ngrok.io"
#define PORT 17686

WiFiClient client;


const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";        

static char strTemperature[10] = {0};
static char strHumidity[10] = {0};

char temp[15] = {0};
char humd[15] = {0};

// Variables
WiFiClient espClient;

/* Prototypes */
//float getTemperature(void);
//float getHumidity(void);

char *charTemp;
char *charHumedad;


void initWiFi(void);
void reconnectWiFi(void);

/* Inicializa wifi */
void initWiFi(void)
{
  delay(10);
  Serial.println("------Conexión WI-FI------");
  Serial.print("Conectando con la red: ");
  Serial.println(SSID);
  Serial.println("Espere");

  reconnectWiFi();
}



void reconnectWiFi(void)
{

  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado con exito a la red");
  Serial.print(SSID);
  Serial.println(" IP obtenido: ");
  Serial.println(WiFi.localIP());
}

void dht22(void *pvParameter) {

  int dhtPin = *((int*)pvParameter);
  DHTesp dhtSensor;
  dhtSensor.setup(dhtPin, DHTesp::DHT22);
  

  while (1 ) {
    TempAndHumidity  data = dhtSensor.getTempAndHumidity();
    
    Serial.println("Temp: " + String(data.temperature, 2) + "°C");
    Serial.println("Humidity: " + String(data.humidity, 1) + "%");
    
    //obtencion de temp y humedad
    delay(200);
    sprintf(strTemperature, "%.2f", data.temperature);
    strncpy (temp,strTemperature,5);
    sprintf(strHumidity, "%.2f", data.humidity);
    strncpy (humd,strHumidity,5);
    //formato de envio topico/contenido
    String temperaturaPublish = "nodos/nodo"+ String(dhtPin) + "/" + String(TOPIC_PUBLISH_TEMPERATURE) + "$" + String(temp);
    String humedadPublish = "nodos/nodo"+ String(dhtPin) + "/" + String(TOPIC_PUBLISH_HUMIDITY) + "$" + String(humd);
    
    //conversion char* para enviar al socket
    charTemp = (char*)temperaturaPublish.c_str();
    charHumedad = (char*)humedadPublish.c_str();
    //envio de humedad
    client.connect(HOST,PORT);
    client.write(charHumedad);
    //envio de temperatura
    client.connect(HOST,PORT);
    client.write(charTemp);
    
    vTaskDelay(1100 /portTICK_PERIOD_MS);
  }

}


void setup() {
  Serial.begin(115200);
  initWiFi();
 
  xTaskCreate(&dht22, "DHT22v1", 1024*4, (void*)&DHT_PIN, 5, NULL);
  xTaskCreate(&dht22, "DHT22v2", 1024*4, (void*)&DHT_PIN2, 5, NULL);
  
}


void loop() {
  
}