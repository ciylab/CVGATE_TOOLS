/**
 * @file dac.cpp
 * @brief Définition de la fonction qui envoie l'information vers 
 * le DAC.
 */
#include "dac.h"

extern int referenceC4cv;

void calibrate(int8_t rotation) {
    if(rotation < 0) {
        referenceC4cv--;
    } else if(0 < rotation) {
        referenceC4cv++;
    }
    send_cv(0, referenceC4cv);
}

/**
 * @brief Cette fonction envoie un entier **cv** compris entre 0 et 4095
 * sur un des deux canaux du dac 12-bit.
 */
void send_cv(int ch, int cv) {
    digitalWrite(SS, LOW);
    if(cv < 0) {
        cv = 0;
    } else if(4095 < cv) {
        cv = 4095;
    }
    if (ch == 0) {
        SPI.transfer((cv >> 8) | 0x30);  // H0x30=OUTA/1x
    } else if(ch == 1) {
        SPI.transfer((cv >> 8) | 0xB0);  // H0xB0=OUTB/1x
    }
    SPI.transfer(cv & 0xff);
    digitalWrite(SS, HIGH);
}

/**
 * Joue la note
 */
void send_pitch(int ch, byte pitch) {
    int cv = (int) (round(1. * referenceC4cv * pitch / 48));
    send_cv(ch, cv);
}


