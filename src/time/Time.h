/**
 * @file Time.h
 * @brief Initialisation et gestion d'une horloge "indépendante".
 */
#ifndef TIME_H
#define TIME_H
#include <Arduino.h>
#include "../base/Module.h"

#define CLOCK_OUT A3 /**<pin de sortie.*/
#define CLOCK_IN A7 /**<pin d'entrée.*/

class Time : public Module {
    public:
    /**
     * La sortie d'horloge est proportionnelle à l'horloge réelle
     * car on propose une multiplication ou une division du temps.
     */
    unsigned long delayBetweenClock;
    /**
     * Pour gérer la sortie.
     */
    unsigned long lastTimeClock;
    /**
     * Gestion de la sortie dans le cas d'une multiplication.
     */
    bool mult_gate_open;
    /**
     * @brief Constructeur par défaut.
     *
     * BPM utilisent les valeurs définies dans module.h
     */
    Time() {
        values[0] = 0;
        values[1] = BPM_DEFAULT;
        values[2] = 1;
        values[3] = 1;
        min[0] = 0;
        max[0] = 1;
        min[1] = BPM_MIN;
        min[2] = 1;
        min[3] = 1;
        max[1] = BPM_MAX;
        max[2] = 4;
        max[3] = 4;
        mult_gate_open = false;
        min[4] = 0;
        max[4] = 10;
        values[4] = max[4];
    }
    
    /**
    * @brief Si vrai on joue la clock.
    */
    bool no_jump() {
        return random(10) < values[4];
    }
    void getString(char str[8], byte item) override;
    void play() override;
    bool on_press_value(byte selectedIndex) override;
};
#endif
