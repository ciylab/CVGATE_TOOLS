/**
 * @file Euclidean.cpp
 * @brief Gestion des deux séquences euclidiennes.
 */

#include "Euclidean.h"

extern unsigned long lastTimeTick;
extern unsigned long delayBetweenTick;
extern unsigned long tick_num;
extern unsigned long now;
extern bool externalClock;
extern bool start_gates;

void Euclidean::set_value_constraint(byte item) {
    switch(item % 4) {
        case 0:
            if (this->values[item] < this->values[item + 1]) {
                this->values[item] = this->values[item + 1];
            }
            break;
        case 1:
            if (this->values[item] > this->values[item - 1]) {
                this->values[item] = this->values[item - 1];
            }
            break;
        case 2:
            if (this->values[item] > this->values[item - 2]) {
                this->values[item] = this->values[item - 2];
            }
            break;
    }
}

void Euclidean::getString(char str[8], byte item) {
    set_value_constraint(item);
    if (item % 4 == 3) {
        setProgressString(str, this->values[item]);
    } else {
        sprintf(str, "%3d   ", this->values[item]);
    }
    if(item < 4) {
        digitalWrite(EUC1_OUT, LOW);
    } else if(3 < item) {
        digitalWrite(EUC2_OUT, LOW);
    }
    this->gates_open[item / 4] = false;
}

/**
 * On gère les deux sorties de la même façon.
 */
void Euclidean::play(byte drum, byte out) {
    byte length;
    byte tick;
    bool play = false;
    length = this->values[4 * drum + 0];
    if(length != 0) {
        tick = tick_num % length;
        play = (values[4 * drum + 1] * 
        (length - values[4 * drum + 2] + tick)) % length < values[4 * drum + 1];
    }
    if (start_gates) {
        if(play && values[4 * drum + 3] != 0) {
            digitalWrite(out, HIGH);
            this->gates_open[drum] = true;
        }
    } else {
        if (now - lastTimeTick >= values[4 * drum + 3] * delayBetweenTick / MAX_GATE && 
                play && this->gates_open[drum]) {
            digitalWrite(out, LOW);
            this->gates_open[drum] = false;
        }
    }    
}

void Euclidean::play() {
    play(DRUM_1, EUC1_OUT);
    play(DRUM_2, EUC2_OUT);
}
