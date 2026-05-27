/**
 * @file Module.h
 * @brief Classe mère des modules.
 */
#ifndef MODULE_H
#define MODULE_H
#include <Arduino.h>

#define NOTE_MAX 60 /**< C5 (MIDDLE C = C4 ?)*/
#define NOTE_MIN 0 /**< C0 */
#define BPM_MIN 30 /**<min bpm of the module */
#define BPM_MAX 240 /**<max bpm of the module */
#define BPM_DEFAULT 120 /**< bpm moyen en club*/
#define CURSOR '>' /**< curseur de l'écran*/
#define MAX_GATE 9

extern const char *STR_NOTES[12];

class Module 
{
  public:
    byte min[8]; /**<Tableau des valeurs minimales */
    byte max[8]; /**<Tableau des valeurs maximales */
    byte values[8]; /**<Tableau des valeurs */
    byte num_param; /**<Nombre de paramètres */
    bool gate_open; /**<Vrai si on envoie un voltage */
    /**
     * Constructeur par défaut qui initialise min, max et values.
     */
    Module(){
        for (int i = 0; i < 8; i++) {
            min[i] = 0;
            max[i] = 16;
            values[i] = 0;
        }
    }
    /**
     * Fonction qui met la valeur de l'item dans **str**.
     */
    virtual void getString(char str[8], byte item) {}
    /**
     * Fonction qui modifie les valeurs en fonction de 
     * certaines contraintes.
     */
    virtual void set_value_constraint(byte item) {}
    /**
     * Fonction qui envoie les impulsions analogiques.
     */
    virtual void play() {}
    /**
     * Fonction qui gère la pression sur le bouton VALUE.
     */
    virtual bool on_press_value(byte selectedIndex) {return false;}
    /**
     * Fonction qui affiche la note en fonction de numéro MIDI.
     */
    void to_string(char str[8], byte note) {
        sprintf(str, "%2s%d   ", STR_NOTES[note % 12], note / 12);
    }
    /**
    * @brief Copie de la chaîne indiquant la barre de progression.
    */
    void setProgressString(char str[8], byte value) {
        int i = 0;
        if(value != 0){
            while(i <= (value - 1) / 2) {
                str[i] = '\'';
                i++;
            }
            if (value % 2 == 1) {
                str[i - 1] = '$';
            }
        }
        while(i < 7) {
            str[i] = ' ';
            i++;
        }
        str[i] = '\0';
    }
};
#endif
