# Dispositivo de detección y dispersión de plagas

Este repositorio contiene el código fuente y la documentación de un sistema de detección distribuido para la **detección de plagas**, diseñado para monitorear la proximidad de objetos (plagas) mediante un **sensor ultrasónico** y activar actuadores de alerta (LED y buzzer) en caso de detección.  
El sistema utiliza **HTTP (REST)** y **MQTT** como protocolos de comunicación, permitiendo gestión en tiempo real y control remoto.  

---

##  Características principales

- **Monitoreo de Proximidad**:  
  El sensor ultrasónico **HC-SR04** mide continuamente la distancia a posibles plagas.  
  Si la distancia es inferior a un umbral (20 cm en la configuración actual), se activa un **LED** y un **buzzer**.

- **Activación de Actuadores**:  
  El sistema puede activar o desactivar manualmente los actuadores mediante comandos remotos.  

- **Sistema Distribuido y Escalable**:  
  Cada nodo basado en **ESP32** se conecta a la red WiFi y comunica datos mediante protocolos estándar (REST y MQTT), lo que permite integración en **entornos agrícolas, invernaderos o almacenes**.  

- **Seguridad y Fiabilidad**:  
  El sistema implementa reconexión automática a la red WiFi y al broker MQTT en caso de pérdida de conexión.  

---

##  Tecnologías utilizadas

- **Microcontrolador**: ESP32  
- **Sensor**: Ultrasónico HC-SR04  
- **Actuadores**: LED y buzzer  
- **Protocolos de comunicación**: HTTP (REST) y MQTT  
- **Backend**: API REST para almacenamiento y gestión de datos  
- **Broker MQTT**: Para comunicación ligera en tiempo real
- **Lenguajes de progamación**: Java, C++ y sql.

---

##  Instrucciones de uso

1. Montaje de los sensores de ultrasonido y los buzzers en los puntos deseados incluyendo los leds de cada sensor para saber cual de los montados se activa. 
2. Define el umbral de detección.  
3. Conecta la placa a la red WiFi especificando SSID y contraseña.  
4. Instala y configura el broker MQTT.  
5. Despliega el servidor backend con la API REST para almacenar y consultar los datos.  
6. Suscribe el sistema al *topic* MQTT correspondiente para poder enviar instrucciones de control remoto (“ON”, “OFF”).  

---

##  Estructura del Proyecto

El proyecto se organiza en dos partes principales: **Backend-API** y **Firmware**.

---

###  Backend-API

El **Backend-API** centraliza y almacena la información de los sensores y actuadores.  
Sus funciones principales incluyen:

- **Gestión de Sensores**:  
  - `GET /api/sensor` → Devuelve todos los sensores registrados.  
  - `GET /api/sensor/:idSensor` → Información de un sensor específico.  
  - `GET /api/sensor/last` → Última lectura registrada.  
  - `POST /api/sensor_values` → Envía nueva lectura del sensor.  

- **Gestión de Actuadores**:  
  - `GET /api/actuador` → Devuelve todos los actuadores.  
  - `GET /api/actuador/:idActuador` → Información de un actuador específico.  
  - `POST /api/actuator_states` → Envía estado actual del actuador.  

- **Almacenamiento de Datos**: Guarda lecturas y estados para análisis histórico y auditoría.  

---

###  Firmware

El **Firmware**, ejecutado en la **ESP32**, controla los dispositivos físicos.  
Sus principales tareas son:

- **Captura de Datos**: Lectura continua de la distancia mediante el sensor ultrasónico.  
- **Control Local de Actuadores**: Encendido de LED y buzzer cuando se detecta una plaga.  
- **Recepción de Comandos MQTT**:  
  - `"ON"` → Activa control automático por sensor.  
  - `"OFF"` → Desactiva actuadores independientemente del sensor.  
- **Comunicación con Backend-API**: Envío de lecturas y recepción de órdenes.  
- **Gestión de Conectividad**: Reconexión automática a WiFi y broker MQTT.  

---

##  Conclusión

El sistema de detección de plagas desarrollado es una **solución modular, escalable y de bajo coste**, capaz de operar de forma autónoma con conectividad WiFi.  

Gracias a la combinación de **HTTP (REST)** y **MQTT**, permite control remoto, integración con dashboards de visualización y supervisión en tiempo real.  

Este proyecto constituye un paso hacia la **agricultura de precisión**, donde los recursos se utilizan eficientemente y las decisiones se toman basadas en datos.  

En futuras versiones se podrían integrar:  
- **Cámaras para identificación visual de plagas**,  
- **Inteligencia Artificial para predicción de eventos**,  
- **Actuadores de tratamiento automático** (aspersores, trampas, etc.).

