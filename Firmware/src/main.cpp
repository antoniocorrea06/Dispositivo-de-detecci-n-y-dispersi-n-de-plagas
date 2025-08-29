
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <NewPing.h> //sensor ultrasonido

// Replace 0 by ID of this current device
const int DEVICE_ID = 2;
const int GROUP_ID = 1;


int test_delay = 1000; // so we don't spam the API
boolean describe_tests = true;
int cont=0;
// Replace 0.0.0.0 by your server local IP (ipconfig [windows] or ifconfig [Linux o MacOS] gets IP assigned to your PC)
String serverName = "http://192.168.92.88:8080";
HTTPClient http;

// Replace WifiName and WifiPassword by your WiFi credentials
#define STASSID "A25 de Paco"    //"Your_Wifi_SSID"
#define STAPSK "pimiento" //"Your_Wifi_PASSWORD"
//#define PIN_LED 17ghg

// MQTT configuration
WiFiClient espClient;
PubSubClient client(espClient);

// Server IP, where de MQTT broker is deployed
const char *MQTT_BROKER_ADRESS = "192.168.92.88"; //poner el de mi router
const uint16_t MQTT_PORT = 1883;
const char* mqtt_topic = "Sensor 1"; // Nuevo topic para suscribirse

// Name for this MQTT client
const char *MQTT_CLIENT_NAME = "ESP_trabajodad"; //este valor depende de la placa, lo suyo es poner el getDeviceId() para no ir cambiando

// callback a ejecutar cuando se recibe un mensaje
// en este ejemplo, muestra por serial el mensaje recibido

String mensaje="";
void OnMqttReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received on ");
  Serial.print(topic);
  Serial.print(": ");

  String content = "";
  for (size_t i = 0; i < length; i++)
  {
    content.concat((char)payload[i]);
  }
  Serial.print(content);
  Serial.println();
  if (content == "OFF" || content == "ON"){
  mensaje = content;

  }
}

// inicia la comunicacion MQTT
// inicia establece el servidor y el callback al recibir un mensaje
void InitMqtt()
{
  client.setServer(MQTT_BROKER_ADRESS, MQTT_PORT);
  client.setCallback(OnMqttReceived);
}
#define TRIG_PIN 5
#define ECHO_PIN 18
#define MAX_DISTANCE 200 // Distancia máxima en cm
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// Definimos los pines de los LEDs
#define LED 27 // Pin del LED
#define BUZZER_PIN 17 // Pin del Buzzer
//#define LED_VERDE 27
//#define LED_NARANJA 26
//#define LED_ROJO 25



// Setup
void setup()
{
  Serial.begin(9600);
  //Iniciar conexion con el WIFI
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(STASSID);
  //pinMode(PIN_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);// Correct method for initializing the Buzzer class
  pinMode(TRIG_PIN, OUTPUT);//pin de disparo del sensor
  pinMode(LED, OUTPUT);
  //Implementamos el semaforo
  //pinMode(LED_VERDE,OUTPUT);
  //pinMode(LED_NARANJA,OUTPUT);
  //pinMode(LED_ROJO,OUTPUT);
  //
  pinMode(ECHO_PIN, INPUT);//pin eco del sensor
  /* Explicitly set the ESP32 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  InitMqtt();

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Setup!");
}

// conecta o reconecta al MQTT
// consigue conectar -> suscribe a topic y publica un mensaje
// no -> espera 5 segundos
void ConnectMqtt()
{
  Serial.print("Empezando conexion con el MQTT");
  if (client.connect(MQTT_CLIENT_NAME))
  {
    
    Serial.print("Conectado");
    client.subscribe(mqtt_topic);
  }
  else
  {
    Serial.print("Failed MQTT connection, rc=");
    Serial.print(client.state());
    Serial.println(" intenta de nuevo en 5 segundos");

    delay(5000);
  }
}

// gestiona la comunicación MQTT
// comprueba que el cliente está conectado
// no -> intenta reconectar
// si -> llama al MQTT loop
void HandleMqtt()
{
  if (!client.connected())
  {
    ConnectMqtt();
      }
  client.loop();
  
}

String response;

String serializeSensor(float value, int idSensor, long timestamp)
{
  DynamicJsonDocument doc(2048);
  doc["idSensor"] = idSensor;
  doc["timestamp"] = timestamp;
  doc["value"] = value;
  doc["removed"] = false;
  String output;
  serializeJson(doc, output);
  return output;
}

void deserializeSensor(int httpResponseCode)
{
  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String responseJson = http.getString();
    // allocate the memory for the document
    DynamicJsonDocument doc(ESP.getMaxAllocHeap());

    // parse a JSON array
    DeserializationError error = deserializeJson(doc, responseJson);

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // extract the values
    JsonArray array = doc.as<JsonArray>();
    for (JsonObject sensor : array)
    {
      int id = sensor["id"];
      String status = sensor["status"].as<String>();
      long timestamp = sensor["timestamp"];
      int value = sensor["value"];
      int groupId = sensor["groupId"];
      int nPlaca = sensor["nPlaca"];

      Serial.println((
                         "Sensor: [id: " + String(id) +
                         ", status: " + status +
                         ", timestamp: " + String(timestamp) +
                         ", value: " + String(value) +
                         ", groupId: " + String(groupId) +
                         ", nPlaca: " + String(nPlaca) +
                         "]")
                         .c_str());
    }
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
}
//Serial de los actuadores hay que cambiar 
String serializeActuator(float status, bool statusBinary, int idActuator, long timestamp)
{
  DynamicJsonDocument doc(2048);
  doc["status"] = status;
  doc["statusBinary"] = statusBinary;
  doc["idActuator"] = idActuator;
  doc["timestamp"] = timestamp;
  doc["removed"] = false;
  String output;
  serializeJson(doc, output);
  return output;
}

void deserializeActuator(int httpResponseCode)
{

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String responseJson = http.getString();
    // allocate the memory for the document
    DynamicJsonDocument doc(ESP.getMaxAllocHeap());

    // parse a JSON array
    DeserializationError error = deserializeJson(doc, responseJson);

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // extract the values
    JsonArray array = doc.as<JsonArray>();
    for (JsonObject sensor : array)
    {
    float status = doc["status"];
    int idActuator = doc["idActuator"];
    float value= doc["value"];
    long timestamp = doc["timestamp"];
    int idGroup = doc["idGroup"];
    int nPlaca = doc["nPlaca"];

Serial.println((
                     "Actuador: [status: " + String(status) +
                     ", idActuator: " + String(idActuator) +
                     ", value: " + String(value) +
                     ", timestamp: " + String(timestamp) +
                     ", idGroup: " + String(idGroup) +
                     ", nPlaca: " + String(nPlaca) +
                     "]")
                     .c_str());
    }
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
}
   
void test_response(int httpResponseCode)
{
  delay(test_delay);
  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
}

void describe(char *description)
{
  if (describe_tests)
    Serial.println(description);
}

void GET_tests()
{

  /*describe("Test GET all sensors");
  String serverPath = serverName + "/api/sensor/all*";
  http.begin(serverPath.c_str());
  deserializeDeviceBody(http.GET());*/

  describe("Test GET all sensors with Connection");
  String serverPath = serverName + "/api/sensor";
  http.begin(serverPath.c_str());
  deserializeSensor(http.GET());

  describe("Test GET Sensor by idSensor");
  serverPath = serverName + "/api/sensor/:idSensor";
  http.begin(serverPath.c_str());
  deserializeSensor(http.GET());

  describe("Test GET last Sensor Byid");
  serverPath = serverName + "/api/sensor/last";
  http.begin(serverPath.c_str());
  deserializeSensor(http.GET());
  
  describe("Test GET last Sensor ByGroup");
  serverPath = serverName + "/api/sensor/group/last";
  http.begin(serverPath.c_str());
  deserializeSensor(http.GET());

       // ACTUADORES
  describe("Test GET all actuators");
  serverPath = serverName + "/api/actuador/all";
  http.begin(serverPath.c_str());
  deserializeActuator(http.GET());

  describe("Test GET all actuators with Connection");
  serverPath = serverName + "/api/actuador";
  http.begin(serverPath.c_str());
  deserializeActuator(http.GET());

  describe("Test GET actuador ById");
  serverPath = serverName + "/api/actuador/:idActuador";
  http.begin(serverPath.c_str());
  deserializeActuator(http.GET());
}

void POST_tests()
{
  int valor;
  int estado;
Serial.print(mensaje);
  if (mensaje == "ON") {
    estado = 1;
    valor = 1;
  } else {
    estado = 0;
    valor = 0;
  }

  // --- ACTUADOR ---
  Serial.println("Enviando POST de actuador...");
String actuator_states_body = serializeActuator(estado, false, 1, millis());
  String serverPath = serverName + "/api/actuator_states";
  http.begin(serverPath.c_str());
  Serial.println("Body actuador:");
  Serial.println(actuator_states_body);
  test_response(http.POST(actuator_states_body));

  // --- SENSOR ---
  Serial.println("Enviando POST de sensor...");
String sensor_value_body = serializeSensor(sonar.ping_cm(), 1, millis());
  serverPath = serverName + "/api/sensor_values";
  http.begin(serverPath.c_str());
  Serial.println("Body sensor:");
  Serial.println(sensor_value_body);
  test_response(http.POST(sensor_value_body));
}



 
 

// Run the tests!
void loop()
{
    int distancia = sonar.ping_cm();

  if (distancia > 0 && distancia < 20 && mensaje != "ON") {
    mensaje = "ON";
    client.publish(mqtt_topic, "ON");
    POST_tests();
  } else if (distancia >= 20 && mensaje != "OFF") {
    mensaje = "OFF";
    client.publish(mqtt_topic, "OFF");
    POST_tests();
  }
  if(cont==10){
    cont=0;
    POST_tests();
  }else{
    cont++;
  }
  if(mensaje == "ON"){
    if (sonar.ping_cm() < 20){//por si acaso
      digitalWrite(LED, HIGH);
      digitalWrite(BUZZER_PIN, HIGH); 
    }
    else{
      digitalWrite(LED, LOW);
      digitalWrite(BUZZER_PIN, LOW); // Detiene el tono
    }
  
  }


  HandleMqtt();
}
