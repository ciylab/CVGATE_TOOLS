/**
 * @file Page.h
 * @brief Classe utile pour l'affichage.
 */
#ifndef PAGE_H
#define PAGE_H
#include <Arduino.h>

/**
 * Il y a 5 pages : le menu et les 4 modules.
 */
#define NUM_PAGE 5

class Page
{
  public:
    byte num_item = 0; /**<Nombre d'items. */
    char text[64] = ""; /**<Chaîne affichée. */
    Page();
    Page(byte num_item);
    //Page(byte num_item, const char *text);
};

#endif
