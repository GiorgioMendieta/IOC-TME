/*
 Basic MQTT example with Authentication

  - connects to an MQTT server, providing username
    and password
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#include <SPI.h>
//#include <Ethernet.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Update these with values suitable for your network.
/*byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(172, 16, 0, 100);
IPAddress server(172, 16, 0, 2);*/
const char* ssid     = "Livebox-2246";
const char* password = "yA4gXPmJobazc3HXmb";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived");
  Serial.println("");
  Serial.println(topic); 
  for (int i = 0; i < length; i++) { 
    Serial.print((char) payload[i]); 
  }
  Serial.println("");
  display.clearDisplay();
  display.setCursor(0,0);
  //display.println("Message arrived");
  //display.println("");
  //display.println(topic); 
  for (int i = 0; i < length; i++) { 
    display.print((char) payload[i]); 
  }
  display.println("");
  display.display();
}

//EthernetClient ethClient;
//PubSubClient client(server, 1883, callback, ethClient);
WiFiClient espClient; 
PubSubClient client(espClient);

void setup()
{
  //Ethernet.begin(mac, ip);
  // Note - the default maximum packet size is 128 bytes. If the
  // combined length of clientId, username and password exceed this use the
  // following to increase the buffer size:
  // client.setBufferSize(255);
  Serial.begin(9600);
  delay(10);

  //Setup OLED
  Wire.begin(4, 15); // pins SDA , SCL
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64, pour notre ESP32 l'adresse est 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  //display.println("test");
  //display.display();
    
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  display.print("Connecting to ");
  display.println(ssid);
    
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      display.print(".");
      display.display();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Server connecting");
  display.println("WiFi connected");
  display.print("IP address: ");
  display.println(WiFi.localIP());
  display.println("Server connecting");
  display.display();
  delay(1000);

  //Connexion au broker
  client.setServer("192.168.1.95", 1883);
  client.setCallback(callback); 

  while(!client.connected()) {
  if (client.connect("arduinoClient")) {
    Serial.println("connection success");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("connection success");
    display.display();
    client.subscribe("topic");
  } else {
    Serial.println("connection failed");
  }
  delay(500);
  }
}

void loop()
{
  client.loop();
  delay(1000);
}
