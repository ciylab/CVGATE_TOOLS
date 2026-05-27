/**
 * @file Sequence.h
 * @brief Prototypes et initialisation.
 */
#ifndef SEQUENCE_H
#define SEQUENCE_H
#include "../base/Module.h"

#if defined(NANO)
#define SEQ_OUT 8 /**<pin de sortie pour le séquenceur. */
#elif defined(EVERY)
#define SEQ_OUT 9 /**<pin de sortie pour le séquenceur. */
#elif defined(THINARY)
#define SEQ_OUT 9 /**<pin de sortie pour le séquenceur. */
#endif

class Sequence : public Module 
{
  public:
      /**
       * @brief Constructeur par défaut.
       *
       * On passe d'un octave à l'autre.
       */
    Sequence() {
        num_param = 8;
        max[0] = 5;
        values[0] = 0;
        
        for (int i = 1; i < 7; i++) {
           min[i] = 0;
           max[i] = NOTE_MAX;
        } 
        values[1] = 0;
        values[2] = 12;
        values[3] = 24;
        values[4] = 36;
        values[5] = 48;
        // div
        min[6] = 1;
        values[6] = 1;
        max[6] = 16;
        
        min[7] = 1;
        max[7] = 9;
        values[7] = 5;
    }  
    void getString(char str[8], byte item) override;    
    void play() override;
    bool on_press_value(byte selectedIndex) override;
};

#endif
