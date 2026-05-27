/**
 * @file Random.h
 * @brief Fonctions du générateur aléatoire.
 */
#ifndef RANDOM_H
#define RANDOM_H
#include "../base/Module.h"

#if defined(NANO)
#define TM_OUT 9 /**<pin de sortie pour le gate.*/
#elif defined(EVERY)
#define TM_OUT 10 /**<pin de sortie pour le gate.*/
#elif defined(THINARY)
#define TM_OUT A0 /**<pin de sortie pour le gate.*/
#endif

#define TM_SIZE 64 /**<Nombre de double-croches mémorisables.*/
#define NUM_SCALE 4 /**<Nombre de gammes.*/

#define SELECT_LEN 0
#define SELECT_MIN 5
#define SELECT_MAX 6
#define SELECT_PROBA 7
#define SELECT_SCALE 1

class Random : public Module 
{
  private:
    byte notes[TM_SIZE];
    static byte scale_size[NUM_SCALE];
    static byte count[NUM_SCALE][12];  // number of pitch lower than the one
    static byte pitchs[NUM_SCALE][12]; // i-th pitch in C
    static byte scales[NUM_SCALE][12];
    byte length;
  public:
    /**
     * @brief Constructeur par défaut.
     *
     * Longueur = 0.
     */
    Random() {
        values[0] = 0;
        max[0] = 64;
        values[SELECT_MIN] = 0;
        max[SELECT_MIN] = NOTE_MAX;
        max[2] = 11;
        min[3] = 1;
        max[3] = 5;
        values[3] = 4;
        max[4] = 1;
        values[SELECT_MAX] = NOTE_MAX;
        max[SELECT_MAX] = NOTE_MAX;
        max[SELECT_SCALE] = 3;
        min[SELECT_PROBA] = 0;
        max[SELECT_PROBA] = 10;
        values[SELECT_PROBA] = 0;
        for (int i = 0; i < TM_SIZE; i++) {
            this->notes[i] = 0;
        }
        init_arrays();
        length = 0;
    }  
    void getString(char str[8], byte item) override;
    void set_value_constraint(byte item) override;
    void init_arrays();
    void play() override;
    bool on_press_value(byte selectedIndex) override;
    byte get_count(byte scale, byte note);
    byte get_pitch(byte scale, byte rank);
    byte get_rand_note(byte scale, byte tone, byte min, byte max);
    int randomize();
};

#endif
