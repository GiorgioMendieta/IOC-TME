#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --------------------------------------------------------------------------------------------------------------------
// Definitions
// WIFI credentials
#define WIFI_SSID "Coloc_Apt64"
#define WIFI_PASSWORD "Carbonara2023*"
// MQTT parameters
// #define MQTT_BROKER "192.168.1.133"
#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT 1883
// Pins
#define PHOTOR_PIN 36
// Timer
#define MAX_WAIT_FOR_TIMER 5
#define TIMER0 0
#define TIMER1 1
#define TIMER2 2
#define TIMER3 3
#define TIMER4 4
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

// --------------------------------------------------------------------------------------------------------------------
// Function prototypes
void setup_wifi();
void connect_mqtt();
void callback(char *topic, byte *payload, unsigned int length);
unsigned int waitFor(int timer, unsigned long period);
void setup_Lum(struct Photoresistance_s *ctx, int timer, unsigned long period);
void loop_Lum(struct Photoresistance_s *ctx);

// --------------------------------------------------------------------------------------------------------------------
// unsigned int waitFor(timer, period)
// Timer pour taches périodiques
// configuration :
//  - MAX_WAIT_FOR_TIMER : nombre maximum de timers utilisés
// arguments :
//  - timer  : numéro de timer entre 0 et MAX_WAIT_FOR_TIMER-1
//  - period : période souhaitée
// retour :
//  - nombre de périodes écoulées depuis le dernier appel
// --------------------------------------------------------------------------------------------------------------------
unsigned int waitFor(int timer, unsigned long period)
{
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER]; // il y a autant de timers que de tâches périodiques
  unsigned long newTime = micros() / period;             // numéro de la période modulo 2^32
  int delta = newTime - waitForTimer[timer];             // delta entre la période courante et celle enregistrée
  if (delta < 0)
    delta = 1 + newTime; // en cas de dépassement du nombre de périodes possibles sur 2^32
  if (delta)
    waitForTimer[timer] = newTime; // enregistrement du nouveau numéro de période
  return delta;
}

// --------------------------------------------------------------------------------------------------------------------
// Photo resistance task definition

struct Photoresistance_s
{
  int timer;
  unsigned long period;
  int val;
  int pin;
};

void setup_Photo(struct Photoresistance_s *ctx, int timer, unsigned long period)
{
  ctx->timer = timer;
  ctx->period = period;
  ctx->val = 0;
  ctx->pin = PHOTOR_PIN;
}

void loop_Photo(struct Photoresistance_s *ctx)
{
  // Wait for the period to elapse
  if (!waitFor(ctx->timer, ctx->period))
    return;

  ctx->val = map(analogRead(ctx->pin), 4095, 0, 0, 100); // Map the value to a 0-100 range
  snprintf(msg, MSG_BUFFER_SIZE, "Photoresistance value: %ld", ctx->val);
  Serial.print("Message published: ");
  Serial.println(msg);
  client.publish("/esp32/photoresistance", msg);
}

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
struct Photoresistance_s Photoresistance;
struct Publish_s Publish;

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
      Serial.println("Connection failed, restarting...");
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
  digitalWrite(LED_BUILTIN, LOW);

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

    Serial.println("Attempting MQTT connection...");

    // Attempt connection
    if (client.connect(clientId))
    {
      Serial.println("Connected to MQTT server!");
      digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED

      client.publish("esp32/photoresistance", "Connected photoresistance!");
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
  // Initialize the serial port
  Serial.begin(9600);
  // Initialize pins
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize WiFi and MQTT
  setup_wifi();
  client.setServer(mqtt_broker, MQTT_PORT);
  client.setCallback(callback);
  // Initialize tasks
  setup_Photo(&Photoresistance, TIMER0, 500000);
  // setup_Publish(&Publish, TIMER1, 2000000, "/esp32/photoresistance");
}

void loop()
{
  // Check if the client is connected to the server
  if (!client.connected())
  {
    connect_mqtt();
  }
  client.loop();
  loop_Photo(&Photoresistance);

  // Publish a message every 2 seconds
  // unsigned long now = millis();
  // if (now - lastPublishTime > MSG_INTERVAL)
  // {
  //   lastPublishTime = now;

  //   ++value;
  //   snprintf(msg, MSG_BUFFER_SIZE, "Photoresistance value: %ld", Photoresistance.val);
  //   Serial.print("Message published: ");
  //   Serial.println(msg);
  //   client.publish("/esp32/photoresistance", msg);
  // }
}
