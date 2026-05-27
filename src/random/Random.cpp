/**
 * @file Random.cpp
 * @brief Gestion du générateur aléatoire.
 */
#include "Random.h"
#include "../base/dac.h"

extern const char *STR_NOTES[12];
/**
 * Texte pour les gammes.
 */
const char *STR_SCALES[NUM_SCALE] = 
    {"CHROMA", "MAJOR ", "PENTA ", "HARMON"};

/**
 * Nombre de notes dont la hauteur est inférieure
 * ou égale au rang.
 */
byte Random::count[NUM_SCALE][12];

/**
 * Tableaux de 0 et 1 suivant que la note est dans la gamme
 * ou pas.
 */
byte Random::scales[NUM_SCALE][12];

/**
 * Rang des notes de la gamme.
 */
byte Random::pitchs[NUM_SCALE][12] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, // chromatic
    {0, 2, 4, 5, 7, 9, 11},                 // major
    {0, 2, 4, 7, 9},                        // pentatonic
    {0, 2, 3, 5, 7, 8, 11},                 // harmonic
};

/**
 * Taille des gammes.
 */
byte Random::scale_size[NUM_SCALE] = {12, 7, 5, 7};

extern bool externalClock;
extern unsigned long lastTimeTick;
extern unsigned long delayBetweenTick;
extern unsigned long tick_num;
extern unsigned long now;
extern bool start_gates;

void Random::getString(char str[8], byte item) {
    switch(item) {
        case 0:
            sprintf(str, "%3d   ", this->values[item]);
            if(this->values[item] == 0) {
                digitalWrite(TM_OUT, LOW);
                this->gate_open = false;
            }
            break;
        case SELECT_MIN: 
            to_string(str, this->values[item]);
            break;
        case 2: 
            sprintf(str, "%2s    ", STR_NOTES[this->values[item]]);
            break;
        case 3:
            setProgressString(str, 2 * this->values[item]);
            break;
        case 4: 
            if (this->values[item] == 0) {
                sprintf(str, "OFF   ");
            } else {
                sprintf(str, "ON    ");               
            }
            break;    
        case SELECT_MAX: 
            to_string(str, this->values[item]);
            break;    
        case SELECT_SCALE: 
            sprintf(str, "%s", STR_SCALES[this->values[item]]);
            break;
        case SELECT_PROBA: 
            setProgressString(str, this->values[item]);
            break;
        default:
            break;
    }
}

void Random::set_value_constraint(byte item) {
    switch(item) {
        case SELECT_LEN:
            if (this->values[item] > this->length) {
                if (this->values[4]) { // freeze
                    this->values[item] = this->length;
                } else {
                    this->length = this->values[item];
                }
            } 
            break;
        case SELECT_MIN:
            if (this->values[item] > this->values[SELECT_MAX]) {
                this->values[item] = this->values[SELECT_MAX];
            }
            break;
        case SELECT_MAX:
            if (this->values[item] < this->values[SELECT_MIN]) {
                this->values[item] = this->values[SELECT_MIN];
            }
            break;
    }
}

/**
 * Alimentation des tableaux scales et count.
 */
void Random::init_arrays() {
    for(int scale = 0; scale < NUM_SCALE; scale++) {
        for(int pitch = 0; pitch < 12; pitch++) {
            scales[scale][pitch] = 0;
        }
        for(int i = 0; i < scale_size[scale]; i++) {
            scales[scale][pitchs[scale][i]] = 1;
        }
        count[scale][0] = 1;
        for(int i = 1; i < 12; i++) {
            count[scale][i] = count[scale][i - 1] + scales[scale][i];
        }
    }
}

/**
 * Fonction qui retourne le nombre de notes inférieures
 * ou égales à **note** dans la gamme **scale**.
 */
byte Random::get_count(byte scale, byte note) {
    // lower or equal pitch 
    return count[scale][11] * (note / 12) + 
        count[scale][note % 12] -
        scales[scale][note % 12];
}

/**
 * Fonction qui retourne la hauteur de la note en fonction de
 * son rang.
 */
byte Random::get_pitch(byte scale, byte rank) {
    return 12 * (rank / scale_size[scale]) + 
        pitchs[scale][rank % scale_size[scale]];  
}

/**
 * Fonction qui retourne une hauteur aléatoire uniformément
 * entre **min** inclus et **max** exclus.
 */
byte Random::get_rand_note(byte scale, byte tone, byte min, byte max) {
    byte a = get_count(scale, min - tone + 12);
    byte b = get_count(scale, max - tone + 12);
    if (a == b) {
        return 0;
    }
    byte r = random(a, b);
    byte note = tone + get_pitch(scale, r);
    if(max <= note) {
        note -= 12;
    }
    return note;
}

int Random::randomize() {
    int delta[] = {-12, -5, 0, 7, 12};
    if(random(10) < this->values[SELECT_PROBA]) {
        return delta[random(5)];
    }
    return 0;
}

/**
 * Gestion des notes jouées.
 */
void Random::play() {
    byte length = this->values[0];
    byte pitch;
    if(length == 0) {
        return;
    }
    byte tick = tick_num % length; // beat number (16 beats by bar)
    if (start_gates) {        // good time to play note
        if (this->values[4] == 0) {  // not frozen
        // new random note
            notes[tick] = get_rand_note(this->values[SELECT_SCALE], 
                this->values[2], this->values[SELECT_MIN], 
                this->values[SELECT_MAX]);
        }
        if (notes[tick] < this->values[SELECT_MIN] || 
            this->values[SELECT_MAX] <= notes[tick]) { // not in range
            return;
        }
        digitalWrite(TM_OUT, HIGH);
        pitch = notes[tick];
        if(this->values[4]) {
            pitch = (60 + pitch + randomize()) % 60;
        }
        OUT_CV(1, (int) round(4095. * pitch / 60));
        this->gate_open = true;
    } else if (this->gate_open && now - lastTimeTick >= 
            this->values[3] * delayBetweenTick / 8) {
        digitalWrite(TM_OUT, LOW);
        this->gate_open = false; 
    }
}    

bool Random::on_press_value(byte selectedIndex) {
    switch(selectedIndex) {
        case 4:
            return true;
        case 5:
            this->values[selectedIndex] = 0;
            return false;
        case 6:
            this->values[selectedIndex] = NOTE_MAX;
            return false;
    }
    return false;
}
