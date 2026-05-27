/**
 * @file Euclidean.h
 * @brief Fonctions pour le générateur euclidiens.
 */
#ifndef EUCLIDEAN_H
#define EUCLIDEAN_H
#include <avr/pgmspace.h>
#include "../base/Module.h"

#if defined(NANO)
#define EUC1_OUT 1
#define EUC2_OUT 0
#elif defined(EVERY)
#define EUC1_OUT 1
#define EUC2_OUT 0
#elif defined(THINARY)
#define EUC1_OUT 1
#define EUC2_OUT A6
#endif

#define DRUM_1 0
#define DRUM_2 1

class Euclidean : public Module
{
  public:
    bool second_gate_open; /**<Par défaut il n'y a qu'un gate par module */
    bool gates_open[2];
    /**
     * Redéfinition des valeurs par défaut.
     */
    Euclidean() {
        values[0] = 16;
        values[1] = 4;
        values[3] = 4;
        min[3] = 1;
        max[3] = MAX_GATE - 1;
        
        values[4] = 16;
        values[5] = 4;
        values[6] = 2;
        values[7] = 4;
        min[7] = 1;
        max[7] = MAX_GATE - 1;
    }
    void getString(char str[8], byte item) override;
    void set_value_constraint(byte item) override;
    void play(byte drum, byte out);
    void play() override;
    bool on_press_value(byte selectedIndex) {return false;};
};
#endif
