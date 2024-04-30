#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include "mqtt.h"
#include "utils.h"

// Photo resistance task definition
struct t_photoresistance
{
    int timer;
    unsigned long period;
    int pin;
    int val;
};

void setup_photo(struct t_photoresistance *ctx, int timer, unsigned long period, int pin)
{
    ctx->timer = timer;
    ctx->period = period;
    ctx->pin = pin;
    ctx->val = 0;
}

void loop_photo(struct t_photoresistance *ctx, t_mailbox *mb)
{
    // Wait for the period to elapse
    if (!waitFor(ctx->timer, ctx->period))
        return;

    ctx->val = map(analogRead(ctx->pin), 4095, 0, 0, 100); // Map the value to a 0-100 range
    mb->val = ctx->val;

    snprintf(msg, MSG_BUFFER_SIZE, "%ld", ctx->val);
    Serial.print("Published to [esp32/photoVal]: ");
    Serial.println(msg);
    client.publish("esp32/photoVal", msg);
}