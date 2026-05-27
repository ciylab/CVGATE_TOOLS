/**
 * @file Time.cpp
 * @brief Gestion de l'horloge.
 */
#include "Time.h"
#include <U8x8lib.h>

/**
 * Durée en microsecondes d'une impulsion d'horloge en sortie.
 */
#define CLOCK_PULSE 15000

extern unsigned long lastTimeTick;
extern unsigned long delayBetweenTick;
extern unsigned long tick_num;
extern unsigned long now;
extern bool externalClock; 
extern byte pins[];
extern bool start_gates;
 
void Time::getString(char str[8], byte item) {
    switch(item) {
        case 0: 
            if (this->values[item] == 0) {
                sprintf(str, "INT   ");
                externalClock = false;
                delayBetweenTick = 15000000 / this->values[1];     
            } else {
                sprintf(str, "EXT   ");
                externalClock = true;
            }
            for (byte i = 0; i < 5; i++) {
                digitalWrite(pins[i], LOW);
            }
            break;
        case 1: 
            sprintf(str, "%3d   ", this->values[item]);
            if(!externalClock) {
                delayBetweenTick = 15000000 / this->values[1];
            }
            break;
        case 2: 
            sprintf(str, "x%d   ", this->values[item]);
            this->values[3] = 1;
            break; 
        case 3: 
            sprintf(str, "/%d   ", this->values[item]);
            this->values[2] = 1;
            break; 
        case 4:
            setProgressString(str, this->values[item]);
            break;
        default:    
            break;
    }
}

void Time::play() {
    delayBetweenClock = delayBetweenTick / this->values[2];
    if (start_gates) {
        lastTimeClock = now;
        if(externalClock) {
            this->values[1] = 15000000 / delayBetweenTick;
        }
        if(tick_num % this->values[3] == 0 && no_jump()) {
            digitalWrite(CLOCK_OUT, HIGH);
            this->gate_open = true;
        }
    } else if(now - lastTimeTick >= CLOCK_PULSE && this->gate_open) {
        digitalWrite(CLOCK_OUT, LOW);
        this->gate_open = false;
    } else if(1 < this->values[2]) {
        if(now - lastTimeClock >= delayBetweenClock && 
            !this->mult_gate_open) {
            if(no_jump()) {
                digitalWrite(CLOCK_OUT, HIGH);
                this->mult_gate_open = true;
            }
            lastTimeClock += delayBetweenClock;
        } else if(now - lastTimeClock >= CLOCK_PULSE && 
            this->mult_gate_open) {
            digitalWrite(CLOCK_OUT, LOW);
            this->mult_gate_open = false;
        }
    }

}

bool Time::on_press_value(byte selectedIndex) {
    return selectedIndex == 0;
}
