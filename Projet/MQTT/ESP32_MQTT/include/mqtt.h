#pragma once

#include <Arduino.h>
#include <PubSubClient.h> // MQTT library
#include <WiFi.h>

#include "credentials.h"
#include "internet.h"
#include "intercommunication.h"

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
    int state; // 1 = connected, 0 = disconnected
};

// Function called when a message arrives on any subscribed topic
void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    String messageTemp;

    for (int i = 0; i < length; i++)
    {
        messageTemp += (char)payload[i];
    }
    Serial.println(messageTemp);

    // Do something according to the message and topic
    if (String(topic) == "rpi/broadcast")
    {
        if (messageTemp == "ledOn")
        {
            Serial.println("Turning ON LED");
            digitalWrite(LED_BUILTIN, HIGH);
        }
        else if (messageTemp == "ledOff")
        {
            Serial.println("Turning OFF LED");
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
}

void setup_mqtt(struct t_mqtt *ctx, int timer, unsigned long period)
{
    ctx->timer = timer;
    ctx->period = period;
    ctx->state = 0;

    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(callback);
}

// Blocking function to connect to MQTT server
boolean connect_mqtt()
{
    Serial.println("Attempting MQTT connection...");

    // Attempt connection
    if (client.connect(clientId))
    {
        Serial.println("Connected to MQTT server!");
        if (!client.subscribe("rpi/broadcast"))
        {
            Serial.println("Failed to subscribe to topic");
        }
    }
    else
    {
        Serial.print("Connection to MQTT server failed, rc=");
        Serial.print(client.state());
        Serial.println(". Trying again in 5 seconds");
    }

    return client.connected();
}

void loop_mqtt(struct t_mqtt *ctx, t_mailbox *mb)
{
    mb->val = 1; // Assume the client is connected
    // Check if the client is connected to the server
    if (!client.connected())
    {
        mb->val = 0; // The client is disconnected
        // Wait for the period to elapse
        if (!waitFor(ctx->timer, ctx->period))
        {
            return;
        }
        else
        {
            // First check WiFi connection
            // TODO: Add mailbox to check if WiFi is connected to speed up the process
            if (WiFi.status() != WL_CONNECTED)
            {
                // if not connected, then first connect to wifi
                reconnect_wifi(WIFI_SSID, WIFI_PASSWORD);
            }
            connect_mqtt();
        }
    }

    client.loop();
}