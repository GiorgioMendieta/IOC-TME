#pragma once

#include <Arduino.h>

enum
{
    EMPTY,
    FULL
};

struct t_mailbox
{
    int state;
    int val;
    // char msg[20];
};