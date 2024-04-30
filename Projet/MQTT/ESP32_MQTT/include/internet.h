#pragma once

#include <Arduino.h>
#include <WiFi.h>

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
            ESP.restart(); // Reset the board after 60 seconds
            return;
        }
    }
    Serial.println("Connected to WiFi");
}

void reconnect_wifi(const char *ssid, const char *password)
{
    // Small delay to fix some issues with WiFi stability
    delay(10);

    Serial.print("Reconnecting to SSID: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
}