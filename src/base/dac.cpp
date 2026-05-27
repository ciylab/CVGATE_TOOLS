/**
 * @file dac.cpp
 * @brief Définition de la fonction qui envoie l'information vers 
 * le DAC.
 */
#include "dac.h"

/**
 * @brief Cette fonction envoie un entier **cv** compris entre 0 et 4095
 * sur un des deux canaux du dac 12-bit.
 */
void OUT_CV(int ch, int cv) {
    digitalWrite(SS, LOW);
    if (ch == 0) {
        SPI.transfer((cv >> 8) | 0x30);  // H0x30=OUTA/1x
    } else if(ch == 1) {
        SPI.transfer((cv >> 8) | 0xB0);  // H0xB0=OUTB/1x
    }
    SPI.transfer(cv & 0xff);
    digitalWrite(SS, HIGH);
}


