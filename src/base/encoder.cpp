/**
 * @file encoder.cpp
 * @brief Gestion des deux encodeurs.
 */
#include "encoder.h"
#include "Display.h"
#include "Page.h"

extern byte selectedIndex;
extern byte current_page;
extern Display oled;
extern bool externalClock;

/**
 * Vrai si une valeur est affichée sur l'écran. Utile pour
 * le premier usage de l'encodeur VALUE.
 */
bool firstShowed = false;

/**
 * @brief Gestion de la rotation de l'encodeur PARAMETER.
 * 
 * On cache la valeur et on gère le curseur.
 */
void handleRotate0(int8_t rotation) {
    if (current_page != 0) {
        oled.hideValue();
    }
    firstShowed = false;
    oled.hideSelected();
    oled.change_selectedIndex(rotation);
    oled.showSelected();  
}

/**
 * @brief Gestion de la pression de l'encodeur PARAMETER.
 *
 * Pour alterner menu principal/menu de module.
 */
void handlePress0() {
    oled.hideSelected();
    if (current_page == 0) {
        current_page = selectedIndex + 1;
    } else {
        current_page = 0;
    }
    firstShowed = false;
    oled.display();
    oled.showSelected();  
}

/**
 * @brief Reboot.
 */
void handleLongPress0() {
    asm volatile("jmp 0x00");
}

/**
 * @brief Gestion de la rotation de l'encodeur VALUE.
 * 
 * Désactivé dans quelques situations. Le premier usage ne fait 
 * que montrer la valeur.
 */
void handleRotate1(int8_t rotation) {
    if (current_page == 0) {
        return;
    }
    if (current_page == 1 && selectedIndex == 5) {
        return;
    }
    if (current_page == 1 && selectedIndex == 1 && externalClock == 1) {
        return;
    }
    if (current_page == 4 && selectedIndex == 4) {
        return;
    }
    if (firstShowed) {
        oled.change_current_value(rotation);
    }
    firstShowed = true;
    oled.showValue();
}

/**
 * @brief Gestion de la pression sur l'encodeur VALUE.
 * 
 * Activé dans quelques situations et appel de l'affichage avec
 * une rotation nulle.
 */
void handlePress1() {
    if (current_page == 0) {
        return;
    }
    if (firstShowed) {
        oled.change_current_value(0);
    }
    firstShowed = true;
    oled.showValue();
}

