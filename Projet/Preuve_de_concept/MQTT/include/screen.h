#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

#include "utils.h"

// OLED display parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 16    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Built-in OLED display task
struct t_screen
{
    int timer;            // numéro de timer utilisé par WaitFor
    unsigned long period; // période d'incrémentation du compteur
    unsigned int cpt;     // compteur
    int val;
};

void setup_screen(struct t_screen *Oled, int timer, unsigned long period)
{

    Wire.begin(4, 15); // pins SDA , SCL
    Serial.begin(9600);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    { // Address 0x3D for 128x64, pour notre ESP32 l'adresse est 0x3C
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    Oled->timer = timer;
    Oled->period = period;
    Oled->cpt = 0;
    Oled->val = 0;

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    delay(1000); // Pause for 1 second
    display.clearDisplay();
    display.setRotation(2);
    display.setTextSize(1);
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    // Display message
    display.println("Initialising...");
    display.display();
}

void loop_screen(struct t_screen *ctx, int val)
{
    if (!waitFor(ctx->timer, ctx->period))
        return; // sort s'il y a moins d'une période écoulée

    ctx->cpt++;
    display.clearDisplay();
    // display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0); // Start at top-left corner

    if (WiFi.status() != WL_CONNECTED)
    {
        display.println("WiFi not connected");
    }
    else
    {
        display.print("IP: ");
        display.println(WiFi.localIP());
    }
    display.println("");

    display.print("Photoresistance: ");
    display.print(val, DEC);
    display.println("%");
    display.display();
}