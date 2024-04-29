/**
 * @file pushbutton.h
 * @brief Header file for push button functionality.
 */

#pragma once

#include <Arduino.h>

#define PB_PIN 23 // Push button pin

// INFO: Push button is not soldered properly on the board!!

// ISR for push button
void IRAM_ATTR isr_pb()
{
    client.publish("esp32/pushbutton", "1");
}

void setup_pushbutton()
{
    pinMode(PB_PIN, INPUT_PULLUP);
    attachInterrupt(PB_PIN, isr_pb, RISING);
}