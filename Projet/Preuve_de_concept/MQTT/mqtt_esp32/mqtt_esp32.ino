#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // MQTT library
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --------------------------------------------------------------------------------------------------------------------
// Definitions and global variables
// Pins
#define PHOTOR_PIN 36
// Timer
#define MAX_WAIT_FOR_TIMER 5
#define TIMER0 0
#define TIMER1 1
#define TIMER2 2
#define TIMER3 3
#define TIMER4 4
// OLED display parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 16    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// WiFi parameters
const char *WIFI_SSID = "Coloc_Apt64";
const char *WIFI_PASSWORD = "Carbonara2023*";
WiFiClient espClient;
// MQTT parameters
const char *MQTT_BROKER = "test.mosquitto.org"; // TODO: Change to the IP address of your MQTT broker
const int MQTT_PORT = 1883;
PubSubClient client(espClient);

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

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
  const char *photo_topic = "/esp32/photoresistance";
  // Wait for the period to elapse
  if (!waitFor(ctx->timer, ctx->period))
    return;

  ctx->val = map(analogRead(ctx->pin), 4095, 0, 0, 100); // Map the value to a 0-100 range
  snprintf(msg, MSG_BUFFER_SIZE, "Value: %ld", ctx->val);
  Serial.print("Message published to [");
  Serial.print(photo_topic);
  Serial.print("]: ");
  Serial.println(msg);
  client.publish(photo_topic, msg);
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

// Built-in OLED display task
struct Oled_s
{
  int timer;            // numéro de timer utilisé par WaitFor
  unsigned int cpt;     // compteur
  unsigned long period; // période d'incrémentation du compteur
  int val;
};

void setup_Oled(struct Oled_s *Oled, int timer, unsigned long period)
{

  Wire.begin(4, 15); // pins SDA , SCL
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  { // Address 0x3D for 128x64, pour notre ESP32 l'adresse est 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  Oled->timer = timer;
  Oled->period = period;
  Oled->cpt = 0;
  Oled->val = 0;

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000); // Pause for 1 second
  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(1);
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  // Display message
  display.println("Initialising...");
  display.display();
}

void loop_Oled(struct Oled_s *ctx, struct Photoresistance_s *pr)
{
  if (!waitFor(ctx->timer, ctx->period))
    return; // sort s'il y a moins d'une période écoulée

  ctx->cpt++;
  display.clearDisplay();
  // display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0); // Start at top-left corner

  if (WiFi.status() != WL_CONNECTED)
  {
    display.println("WiFi not connected");
  }
  else
  {
    display.print("SSID: ");
    display.println(WIFI_SSID);
    display.print("IP: ");
    display.println(WiFi.localIP());
  }
  display.println("");

  display.println("Photoresistance: ");
  display.print(pr->val, DEC);
  display.println("%");
  display.display();
}

// --------------------------------------------------------------------------------------------------------------------
// Task declarations
struct Photoresistance_s Photoresistance;
struct Publish_s Publish;
struct Oled_s Oled;

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
      setup_wifi(WIFI_SSID, WIFI_PASSWORD);
    }

    Serial.println("Attempting MQTT connection...");

    // Attempt connection
    if (client.connect(clientId))
    {
      Serial.println("Connected to MQTT server!");
      digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED

      client.publish("esp32/photoresistance", "Connected photoresistance!");
      if (!client.subscribe("rpi/broadcast"))
      {
        Serial.println("Failed to subscribe to topic");
      }
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
  // Initialize tasks
  setup_Photo(&Photoresistance, TIMER0, 500000);
  // setup_Publish(&Publish, TIMER1, 2000000, "/esp32/photoresistance");
  setup_Oled(&Oled, TIMER2, 1000000); // Refresh display every 1 second
  // Initialize WiFi and MQTT
  setup_wifi(WIFI_SSID, WIFI_PASSWORD);
  client.setServer(MQTT_BROKER, MQTT_PORT);
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
  loop_Photo(&Photoresistance);
  loop_Oled(&Oled, &Photoresistance);
}
