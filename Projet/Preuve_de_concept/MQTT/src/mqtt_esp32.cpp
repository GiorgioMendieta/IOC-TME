#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // MQTT library

#include "credentials.h"
#include "utils.h"
#include "screen.h"
#include "photoresistance.h"
#include "mqtt.h"

// --------------------------------------------------------------------------------------------------------------------
// Definitions and global variables
// WiFiClient espClient;
// PubSubClient client(espClient);

// --------------------------------------------------------------------------------------------------------------------
// Function prototypes
void setup_wifi();
// void connect_mqtt();
// void callback(char *topic, byte *payload, unsigned int length);

// Message publication task

struct Publish_s
{
  int timer;
  unsigned long period;
  char topic[20];
  char msg[40];
};

void setup_Publish(struct Publish_s *ctx, int timer, unsigned long period, const char *topic)
{
  ctx->timer = timer;
  ctx->period = period;
  strcpy(ctx->topic, topic);
  strcpy(ctx->msg, "");
}

void loop_Publish(struct Publish_s *ctx, const char *msg)
{
  // Wait for the period to elapse
  if (!waitFor(ctx->timer, ctx->period))
    return;

  strcpy(ctx->msg, msg);
  client.publish(ctx->topic, ctx->msg);
}

// --------------------------------------------------------------------------------------------------------------------
// Task declarations
struct t_photoresistance Photoresistance;
struct Publish_s Publish;
struct t_screen Oled;

// Connect to WiFi
void setup_wifi(const char *ssid, const char *password)
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
    count++;
    delay(1000);
    if (count >= max_retries)
    {
      Serial.println("Connection failed, restarting...");
      ESP.restart(); // Reset the ESP after 60 seconds
      return;
    }
  }
}

// // Blocking function to connect to MQTT server
// // TODO: Adapt to non-blocking version
// void connect_mqtt()
// {
//   digitalWrite(LED_BUILTIN, LOW);

//   // Wait for connection
//   while (!client.connected())
//   {
//     const char *clientId = "ESP32Client-1";
//     // First check WiFi connection
//     if (WiFi.status() != WL_CONNECTED)
//     {
//       // if not connected, then first connect to wifi
//       setup_wifi(WIFI_SSID, WIFI_PASSWORD);
//     }

//     Serial.println("Attempting MQTT connection...");

//     // Attempt connection
//     if (client.connect(clientId))
//     {
//       Serial.println("Connected to MQTT server!");
//       digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED

//       client.publish("esp32/photoresistance", "Connected photoresistance!");
//       if (!client.subscribe("rpi/broadcast"))
//       {
//         Serial.println("Failed to subscribe to topic");
//       }
//     }
//     else
//     {
//       Serial.print("Connection to MQTT server failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" trying again in 5 seconds");
//       // Wait 5 seconds before retrying
//       delay(5000);
//     }
//   }
// }

// Function called when a message arrives on any subscribed topic
// void callback(char *topic, byte *payload, unsigned int length)
// {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.print("] ");
//   String messageTemp;

//   for (int i = 0; i < length; i++)
//   {
//     Serial.print((char)payload[i]);
//     messageTemp += (char)payload[i];
//   }
//   Serial.println();

//   // Do something according to the message and topic
//   // Check if a message is received on the topic "rpi/broadcast"
//   if (String(topic) == "rpi/broadcast")
//   {
//     Serial.println("Broadcast message received");
//     // Do something with the message
//   }
// }

// Run setup once
void setup()
{
  // Initialize the serial port
  Serial.begin(9600);
  // Initialize pins
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize tasks
  setup_Photo(&Photoresistance, TIMER0, 500000);
  // setup_Publish(&Publish, TIMER1, 2000000, "/esp32/photoresistance");
  setup_Oled(&Oled, TIMER2, 1000000); // Refresh display every 1 second
  // Initialize WiFi and MQTT
  setup_wifi(WIFI_SSID, WIFI_PASSWORD);
  // client.setServer(MQTT_BROKER, MQTT_PORT);
  // client.setCallback(callback);
  setup_mqtt();
}

void loop()
{
  // Check if the client is connected to the server
  // if (!client.connected())
  // {
  //   connect_mqtt();
  // }
  // client.loop();
  loop_mqtt();
  loop_Photo(&Photoresistance);
  loop_Oled(&Oled, Photoresistance.val);
}
