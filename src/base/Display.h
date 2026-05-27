/**
 * @file Display.h
 * @brief Champs de la classe d'affichage.
 */

#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>

/**
 * Caractère permettant de repérer l'item.
 */
#define CURSOR '>'

class Display 
{
  public:
    Display();
    char buffer[64]; /**<Chaîne de caractères en tampon qui doit être affichée. */
    char screen[64]; /**<Chaîne de caractères en tampon qui est affichée. */
    byte current_pos; /**<Position courante du caractère dans le tampon */
    void putChar(byte position, char c);
    void init();
    void display();
    void welcome();
    void update();
    void change_selectedIndex(int8_t rotation);
    void change_current_value(int8_t rotation);
    void hideSelected();
    void showSelected();
    void showValue();
    void hideValue();
};
#endif
