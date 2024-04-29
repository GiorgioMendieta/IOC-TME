#pragma once

#include <Arduino.h>
#include <PubSubClient.h> // MQTT library
#include <WiFi.h>

#include "credentials.h"
#include "internet.h"

// Used for MQTT messages (payload)
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);
const char *clientId = "ESP32Client-1";

struct t_mqtt
{
    int timer;
    unsigned long period;
};

// Function prototypes
void callback(char *topic, byte *payload, unsigned int length);

void setup_mqtt(struct t_mqtt *ctx, int timer, unsigned long period)
{
    ctx->timer = timer;
    ctx->period = period;

    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(callback);
}

// Blocking function to connect to MQTT server
boolean connect_mqtt()
{
    digitalWrite(LED_BUILTIN, LOW);

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

        if (!client.subscribe("rpi/broadcast"))
        {
            Serial.println("Failed to subscribe to topic");
        }

        client.publish("esp32/photoresistance", "Connected photoresistance!");
    }
    else
    {
        Serial.print("Connection to MQTT server failed, rc=");
        Serial.print(client.state());
        Serial.println(". Trying again in 5 seconds");
    }

    return client.connected();
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

void loop_mqtt(struct t_mqtt *ctx)
{
    // Check if the client is connected to the server
    if (!client.connected())
    {
        // Wait for the period to elapse
        // TODO: Verify if this is the correct way to wait for the period
        if (!waitFor(ctx->timer, ctx->period))
        {
            return;
        }
        connect_mqtt();
    }
    client.loop();
}