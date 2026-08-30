/**
 * @file dac.h
 */

#ifndef DAC_H
#define DAC_H
#include <SPI.h>

void calibrate(int8_t rotation);
void send_cv(int ch, int cv);
void send_pitch(int ch, byte pitch);
#endif
