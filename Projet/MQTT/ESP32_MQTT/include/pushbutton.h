/**
 * @file pushbutton.h
 * @brief Header file for push button functionality.
 */

#pragma once

#include <Arduino.h>

// INFO: Push button is not soldered properly on the board!!

// ISR for push button
void IRAM_ATTR isr_pb()
{
    // TODO: Change this for something more useful
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    client.publish("esp32/pushbutton", "1");
}

void setup_pushbutton(int pin)
{
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(pin, isr_pb, RISING);
}