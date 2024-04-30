#pragma once

// Environment-specific constants

// ---------------------------------------------------
// WIFI credentials
// ---------------------------------------------------
// const char *WIFI_SSID "Livebox-2246";
// const char *WIFI_PASSWORD "yA4gXPmJobazc3HXmb";
const char *WIFI_SSID = "Coloc_Apt64";
const char *WIFI_PASSWORD = "Carbonara2023*";
// const char *WIFI_SSID = "Giorgio Mendieta";
// const char *WIFI_PASSWORD = "tacosdemole";

// ---------------------------------------------------
// MQTT parameters
// ---------------------------------------------------
const char *MQTT_BROKER = "192.168.1.3"; // Raspberry Pi IP address
// const char *MQTT_BROKER = "raspberrypi.local"; // Raspberry Pi IP address
const int MQTT_PORT = 1883;
// TODO: Implement user authentication
const char *MQTT_USER = "test";
const char *MQTT_PASSWORD = "test";