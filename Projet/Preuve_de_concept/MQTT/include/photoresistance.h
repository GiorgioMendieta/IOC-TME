#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include "mqtt.h"
#include "utils.h"

#define PHOTOR_PIN 36

// Photo resistance task definition

struct t_photoresistance
{
    int timer;
    unsigned long period;
    int val;
    int pin;
};

void setup_photo(struct t_photoresistance *ctx, int timer, unsigned long period)
{
    ctx->timer = timer;
    ctx->period = period;
    ctx->val = 0;
    ctx->pin = PHOTOR_PIN;
}

void loop_photo(struct t_photoresistance *ctx)
{
    const char *photo_topic = "/esp32/photoresistance";
    // Wait for the period to elapse
    if (!waitFor(ctx->timer, ctx->period))
        return;

    ctx->val = map(analogRead(ctx->pin), 4095, 0, 0, 100); // Map the value to a 0-100 range
    snprintf(msg, MSG_BUFFER_SIZE, "Value: %ld", ctx->val);
    Serial.print("Message published to [");
    Serial.print(photo_topic);
    Serial.print("]: ");
    Serial.println(msg);
    client.publish(photo_topic, msg);
}