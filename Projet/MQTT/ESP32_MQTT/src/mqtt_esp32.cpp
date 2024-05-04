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
// Pins
#define PB_PIN 23     // Push button pin
#define PHOTOR_PIN 36 // Photoresistance pin
// Task declarations
struct t_photoresistance Photoresistance;
struct t_screen Screen;
struct t_mqtt Mqtt;
// Mailboxes
struct t_mailbox mb_conn = {.state = EMPTY};
struct t_mailbox mb_photor = {.state = EMPTY};

// Run setup once
void setup()
{
  // Initialize the serial port
  Serial.begin(115200);
  // Initialize ESP32 pins
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize push button
  setup_pushbutton(PB_PIN);
  // Initialize tasks
  setup_photo(&Photoresistance, TIMER0, 30000000, PHOTOR_PIN);
  setup_screen(&Screen, TIMER1, 1000000); // Refresh display every 1 second
  // Initialize WiFi and MQTT
  setup_wifi(WIFI_SSID, WIFI_PASSWORD);
  setup_mqtt(&Mqtt, TIMER2, 5000000); // Add broker as parameter? Attempt reconnection every 5 seconds
}

// Main loop that runs indefinitely
void loop()
{
  loop_mqtt(&Mqtt, &mb_conn);
  loop_photo(&Photoresistance, &mb_photor);
  loop_screen(&Screen);
}
