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

// Update these with values suitable for your network.
/*byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(172, 16, 0, 100);
IPAddress server(172, 16, 0, 2);*/
const char* ssid     = "connexionLT";
const char* password = "thirionlou";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived");
  Serial.println("");
  Serial.println(topic); 
  for (int i = 0; i < length; i++) { 
    Serial.print((char) payload[i]); 
  }
  Serial.println("");
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

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

  Serial.println("Server connecting");
  
  client.setServer("192.168.43.246", 1883);
  client.setCallback(callback); 

  while(!client.connected()) {
  if (client.connect("arduinoClient")) {
    Serial.println("publish");
    client.publish("hello","connection");
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
  client.publish("hello","hello world");
  delay(1000);
}
