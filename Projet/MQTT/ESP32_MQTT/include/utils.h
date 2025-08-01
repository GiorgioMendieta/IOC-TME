#pragma once

#include <Arduino.h> // for micros()

// Timer
#define MAX_WAIT_FOR_TIMER 5
#define TIMER0 0
#define TIMER1 1
#define TIMER2 2
#define TIMER3 3
#define TIMER4 4

// --------------------------------------------------------------------------------------------------------------------
// unsigned int waitFor(timer, period)
// Timer pour taches périodiques
// configuration :
//  - MAX_WAIT_FOR_TIMER : nombre maximum de timers utilisés
// arguments :
//  - timer  : numéro de timer entre 0 et MAX_WAIT_FOR_TIMER-1
//  - period : période souhaitée
// retour :
//  - nombre de périodes écoulées depuis le dernier appel
// --------------------------------------------------------------------------------------------------------------------

unsigned int waitFor(int timer, unsigned long period)
{
    static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER]; // il y a autant de timers que de tâches périodiques
    unsigned long newTime = micros() / period;             // numéro de la période modulo 2^32
    int delta = newTime - waitForTimer[timer];             // delta entre la période courante et celle enregistrée
    if (delta < 0)
        delta = 1 + newTime; // en cas de dépassement du nombre de périodes possibles sur 2^32
    if (delta)
        waitForTimer[timer] = newTime; // enregistrement du nouveau numéro de période
    return delta;
}