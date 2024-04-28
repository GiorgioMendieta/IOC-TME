#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include "mqtt.h"
#include "utils.h"

#define PHOTOR_PIN 36

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
    const char *photo_topic1 = "esp32/photoVal";
    const char *photo_topic2 = "esp32/photoState";
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

    snprintf(msg, MSG_BUFFER_SIZE, "Value: %ld", ctx->val);
    // Serial.print("Message published to [");
    // Serial.print(photo_topic1);
    // Serial.print("]: ");
    // Serial.println(msg);
    client.publish(photo_topic1, msg);

    snprintf(msg, MSG_BUFFER_SIZE, "State: %ld", ctx->state);
    Serial.print("Message published to [");
    Serial.print(photo_topic2);
    Serial.print("]: ");
    Serial.println(msg);
    client.publish(photo_topic2, msg);
}