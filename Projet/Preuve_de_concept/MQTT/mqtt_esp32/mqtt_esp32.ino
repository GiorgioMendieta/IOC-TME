#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
// #include "WiFiCredentials.h" // File containing WiFi credentials

// --------------------------------------------------------------------------------------------------------------------
// WIFI credentials
#define WIFI_SSID "Coloc_Apt64"
#define WIFI_PASSWORD "Carbonara2023*"
// MQTT parameters
// #define MQTT_BROKER "192.168.1.133"
#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT 1883
// WiFi parameters
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
// MQTT parameters
const char *mqtt_broker = MQTT_BROKER;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublishTime = 0;
#define MSG_INTERVAL 2000
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Function prototypes
void setup_wifi();
void connect_mqtt();
void callback(char *topic, byte *payload, unsigned int length);

// Connect to WiFi
void setup_wifi()
{
  int count = 0;
  const int max_retries = 60;
  // Small delay to fix some issues with WiFi stability
  delay(10);

  Serial.print("Connecting to SSID: \"");
  Serial.print(ssid);
  Serial.println("\"");
  // Set WiFi mode to station (client)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
    count++;
    if (count >= max_retries)
    {
      Serial.println("Connection failed");
      ESP.restart(); // Reset the ESP after 60 seconds
      return;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Blocking function to connect to MQTT server
// TODO: Adapt to non-blocking version
void connect_mqtt()
{
  // Wait for connection
  while (!client.connected())
  {
    const char *clientId = "ESP32Client-1";

    // First check WiFi connection
    if (WiFi.status() != WL_CONNECTED)
    {
      // if not connected, then first connect to wifi
      setup_wifi();
    }

    Serial.print("Attempting MQTT connection...");

    // Attempt connection
    if (client.connect(clientId))
    {
      Serial.println("Connected to MQTT server!");
      // Publish a message to the topic "/outTopic"
      Serial.println("publish");

      client.publish("esp32/sensor1", "Sensor 1 connecté!");
      client.subscribe("rpi/broadcast");
    }
    else
    {
      Serial.print("Connection to MQTT server failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Function called when a message arrives on any subscribed topic
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  // Do something according to the message and topic
  // Check if a message is received on the topic "rpi/broadcast"
  if (String(topic) == "rpi/broadcast")
  {
    Serial.println("Broadcast message received");
    // Do something with the message
  }
}

// Run setup once
void setup()
{
  Serial.begin(9600);
  setup_wifi();
  Serial.println("Connecting to MQTT broker...");
  client.setServer(mqtt_broker, MQTT_PORT);
  client.setCallback(callback);
}

void loop()
{
  // Check if the client is connected to the server
  if (!client.connected())
  {
    connect_mqtt();
  }
  client.loop();

  // Publish a message every 2 seconds
  unsigned long now = millis();
  if (now - lastPublishTime > MSG_INTERVAL)
  {
    lastPublishTime = now;

    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "salut ma grosse poule #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("/esp32/sensor1", msg);
  }
}