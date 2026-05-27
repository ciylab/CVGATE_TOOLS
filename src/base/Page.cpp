/**
 * @file Page.cpp
 * @brief Constructeurs.
 */
#include <Arduino.h>
#include "Page.h"

/**
 * Constructeur vide.
 */
Page::Page(){};

/**
 * Constructeur avec les arguments.
 */
 
Page::Page(byte num_item) {
    this->num_item = num_item;
}
/*
Page::Page(byte num_item, const char *text) {
    this->num_item = num_item;
    strcpy(this->text, text);
}*/
