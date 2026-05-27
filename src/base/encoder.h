/**
 * @file encoder.h
 * @brief Prototypes uniquement.
 */
#ifndef ENCODER_H
#define ENCODER_H
#include <Arduino.h>

#define NUM_ENC 2 /**<Nombre d'encodeurs.*/

void handleRotate0(int8_t rotation);
void handlePress0();
void handleLongPress0();
void handleRotate1(int8_t rotation);
void handlePress1();
#endif
