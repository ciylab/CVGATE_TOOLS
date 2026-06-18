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
byte Random::count[NUM_SCALE][12] = {
    { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, },
    { 1,  1,  2,  2,  3,  4,  4,  5,  5,  6,  6,  7, },
    { 1,  1,  2,  2,  3,  3,  3,  4,  4,  5,  5,  5, },
    { 1,  1,  2,  3,  3,  4,  4,  5,  6,  6,  6,  7, },
};

/**
 * Tableaux de 0 et 1 suivant que la note est dans la gamme
 * ou pas.
 */
byte Random::scales[NUM_SCALE][12] = {
    { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, },
    { 1,  0,  1,  0,  1,  1,  0,  1,  0,  1,  0,  1, },
    { 1,  0,  1,  0,  1,  0,  0,  1,  0,  1,  0,  0, },
    { 1,  0,  1,  1,  0,  1,  0,  1,  1,  0,  0,  1, },
};

/**
 * Rang des notes de la gamme.
 */
byte Random::pitchs[NUM_SCALE][12] = { // i-th pitch in C
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
 * @brief Retourne un nombre de notes.
 *
 * Pour une gamme donnée et une hauteur donnée, retourne
 * le nombre de notes inférieure ou égale à cette hauteur
 * dans cette gamme.
 */
byte Random::get_count(byte scale, byte note) {
    // lower pitch 
    return count[scale][11] * (note / 12) + 
        count[scale][note % 12] -
        scales[scale][note % 12];
}

/**
 * @brief Retourne une hauteur de note.
 *
 * Pour une gamme donnée et un rang donné, retourne
 * la hauteur correspondante.
 */
byte Random::get_pitch(byte scale, byte rank) {
    return 12 * (rank / scale_size[scale]) + 
        pitchs[scale][rank % scale_size[scale]];  
}

/**
 * @brief Retourne une hauteur de note aléatoire.
 *
 * Pour une gamme donnée et une tonalité donnée, retourne
 * une hauteur aléatoire comprise entre min (inclus) 
 * et max (exclus) pour un tirage uniforme.
 */
byte Random::get_rand_note(byte scale, byte tone, byte min, byte max) {
    byte a = get_count(scale, min - tone + 12);
    byte b = get_count(scale, max - tone + 12);
    if (a == b) {
        return 0;
    }
    // rank in the C range with uniform distribution
    byte r = random(a, b);
    // pitch in tone
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
        OUT_CV(1, (int) round(4095. * pitch / NOTE_MAX));
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
