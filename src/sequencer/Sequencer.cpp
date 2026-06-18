/**
 * @file Sequence.cpp
 * @brief Séquenceur de 6 notes.
 */
#include "Sequencer.h"
#include "../base/dac.h"

extern const char *STR_NOTES[12];
extern bool externalClock;
extern bool start_gates;

extern unsigned long lastTimeTick;
extern unsigned long delayBetweenTick;
extern unsigned long tick_num;
extern unsigned long now;

void Sequence::getString(char str[8], byte item) {    
    switch(item) {
        case 0: sprintf(str, "%3d   ", this->values[item]);
                if(this->values[item] == 0) {
                    digitalWrite(SEQ_OUT, LOW);
                    this->gate_open = false;
                }
            break;
        case 6:
            sprintf(str, ":%d   ", this->values[item]);
            break;
        case 7: setProgressString(str, this->values[item]);
            break;
        default:
            if (127 <  this->values[item]) {
                sprintf(str, "----- ");
            } else {
                to_string(str, this->values[item]);
            }
    }
}

void Sequence::play() {
    byte length = this->values[0];
    if(length == 0 || tick_num % this->values[6] != 0) {
        return;
    }
    byte tick = (tick_num / this->values[6]) % length;
    if (start_gates && this->values[tick + 1] <= 127) {
        digitalWrite(SEQ_OUT, HIGH);
        OUT_CV(0, (int) round(4095. * this->values[tick + 1] / NOTE_MAX));
        this->gate_open = true;
    } else if (this->gate_open && 
            now - lastTimeTick >= values[7] * delayBetweenTick / 10) {
        digitalWrite(SEQ_OUT, LOW);
        this->gate_open = false;
    }
}

bool Sequence::on_press_value(byte selectedIndex) {
    if (0 < selectedIndex && selectedIndex < 6) {
        this->values[selectedIndex] += 128; // outside pitch range
    }
    return false;  
}

