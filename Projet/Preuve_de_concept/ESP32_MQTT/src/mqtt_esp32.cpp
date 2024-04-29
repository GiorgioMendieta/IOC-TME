#include <Arduino.h>

#include "credentials.h"
#include "screen.h"
#include "photoresistance.h"
#include "mqtt.h"
#include "internet.h"
#include "pushbutton.h"
#include "intercommunication.h"

// --------------------------------------------------------------------------------------------------------------------
// Definitions and global variables
// --------------------------------------------------------------------------------------------------------------------
// Task declarations
struct t_photoresistance Photoresistance;
struct t_screen Screen;
struct t_mqtt Mqtt;
// Mailboxes
struct t_mailbox mb_mqtt = {.state = EMPTY};
// struct t_mailbox mb_wifi = {.state = EMPTY};
struct t_mailbox mb_photor = {.state = EMPTY};

// Run setup once
void setup()
{
  // Initialize the serial port
  Serial.begin(9600);
  // Initialize ESP32 pins
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize push button
  setup_pushbutton();
  // Initialize tasks
  setup_photo(&Photoresistance, TIMER0, 500000);
  setup_screen(&Screen, TIMER1, 1000000); // Refresh display every 1 second
  // Initialize WiFi and MQTT
  setup_wifi(WIFI_SSID, WIFI_PASSWORD);
  setup_mqtt(&Mqtt, TIMER2, 5000000); // Add broker as parameter? Attempt reconnection every 5 seconds
}

// Main loop that runs indefinitely
void loop()
{
  loop_mqtt(&Mqtt, &mb_mqtt);
  loop_photo(&Photoresistance, &mb_photor);
  loop_screen(&Screen, &mb_photor, &mb_mqtt);
}
