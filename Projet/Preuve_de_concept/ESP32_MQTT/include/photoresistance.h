#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include "mqtt.h"
#include "utils.h"

#define PHOTOR_PIN 36 // Photoresistance pin

// Photo resistance task definition
enum photo_levels
{
    DARK = 0,
    DIM = 1,
    BRIGHT = 2
};

struct t_photoresistance
{
    int timer;
    unsigned long period;
    int pin;
    int val;
    int state;
};

void setup_photo(struct t_photoresistance *ctx, int timer, unsigned long period)
{
    ctx->timer = timer;
    ctx->period = period;
    ctx->pin = PHOTOR_PIN;
    ctx->val = 0;
    ctx->state = DARK;
}

void loop_photo(struct t_photoresistance *ctx)
{
    // Wait for the period to elapse
    if (!waitFor(ctx->timer, ctx->period))
        return;

    ctx->val = map(analogRead(ctx->pin), 4095, 0, 0, 100); // Map the value to a 0-100 range
    if (ctx->val < 33)
    {
        ctx->state = DARK;
    }
    else if (ctx->val < 66)
    {
        ctx->state = DIM;
    }
    else
    {
        ctx->state = BRIGHT;
    }

    snprintf(msg, MSG_BUFFER_SIZE, "%ld", ctx->val);
    Serial.print("Published to [esp32/photoVal]: ");
    Serial.println(msg);
    client.publish("esp32/photoVal", msg);

    snprintf(msg, MSG_BUFFER_SIZE, "%ld", ctx->state);
    Serial.print("Published to [esp32/photoState]: ");
    Serial.println(msg);
    client.publish("esp32/photoState", msg);
}