/*
 Basic MQTT example with Authentication

  - connects to an MQTT server, providing username and password
  - publishes "hello world" to the topic "/outTopic"
  - subscribes to the topic "/inTopic"
*/

#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "WiFiCredentials.h" // File containing WiFi credentials

// WiFi parameters
const char *ssid = SSID;
const char *password = PASSWORD;
// MQTT parameters
const char *mqtt_broker = MQTT_BROKER;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long timeSinceLastMsg = 0;
#define MSG_INTERVAL 2000
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Connect to WiFi
void wifi_setup()
{
  // Small delay to fix some issues with WiFi stability
  delay(10);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Set WiFi mode to station (client)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("");
  Serial.println("WiFi connected");
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
    Serial.print("Device disconnected from MQTT server. Trying to reconnect...");
    String clientId = "arduinoClient";

    // Attempt connection
    if (client.connect(clientId))
    {
      Serial.println("Connected to MQTT server!");
      // Publish a message to the topic "/outTopic"
      Serial.println("publish");
      client.publish("/outTopic", "salut ma grosse poule reconnectée");
      // Subscribe to the topic "/inTopic"
      client.subscribe("/inTopic");
    }
    else
    {
      Serial.println("Connection to MQTT server failed");
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Function called when a message arrives
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Do something according to the message
}

// Run setup once
void setup()
{
  Serial.begin(9600);
  wifi_setup();
  Serial.println("Connecting to MQTT broker...");
  client.setServer(mqtt_broker, 1883);
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
  if (now - lastMsg > MSG_INTERVAL)
  {
    lastMsg = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "salut ma grosse poule #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("/outTopic", msg);
  }
}
