/**
 * @file Display.cpp
 * @brief Gestion de l'affichage avec tampon.
 */

#include <U8x8lib.h>
#include "Display.h"
#include "../euclidean/Euclidean.h"
#include "../sequencer/Sequencer.h"
#include "Page.h"
#include "my_u8x8_font_7x14_1x2_r.h"

/**
 * Oled SSD1306 ou SH1106 (voir Display.h)
 */

#if defined(SSD1306)
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
#elif defined(SH1106)
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
#endif


extern Page pages[NUM_PAGE];
extern Module *modules[NUM_PAGE - 1];
extern byte selectedIndex;
extern byte current_page;

/**
 * @brief Position possible du curseur
 */
byte item_pos[8] = {0, 16, 32, 48, 8, 24, 40, 56}; // 8 items

/**
 * Pour l'affichage des notes C, D, E, F#,...
 */
const char *STR_NOTES[12] = {
    "C", "C#", "D", "D#", "E", "F", 
    "F#", "G", "G#", "A", "A#", "B"
};

/**
 * Initialisation du setup.
 */
void Display::init() {
    u8x8.begin();
    u8x8.setFont(my_u8x8_font_7x14_1x2_r);
}

/**
 * Initialisation de l'écran à vide = 63 espaces.
 */
Display::Display() {
    for (int i = 0; i < 63; i++) {
        screen[i] = ' ';
    }
    screen[63] = '\0';
}

/**
 * Fonction qui affiche le caractère **c** à la position **pos**.
 */
void Display::putChar(byte position, char c) {
    u8x8.drawGlyph(position % 16, 2 * (position / 16), c);
}

/**
 * @brief Affichage du message d'accueil.
 */
void Display::welcome() {
    sprintf(screen, "  RHYTHM BOX      AND             SEQUENCER       %s",
            VERSION);
    for (int i = 0; i < 63; i++) {
        putChar(i, screen[i]);
    }
}

/**
 * @brief Fonction qui met dans le tampon le texte de la nouvelle page. 
 */
void Display::display() {
    this->current_pos = 1;
    selectedIndex = 0;
    Page p = pages[current_page];
    strcpy(this->buffer, p.text);
    putChar(0, CURSOR);
}

/**
 * @brief Fonction qui remplace le curseur par un espace.
 */
void Display::hideSelected() {
    byte pos = item_pos[selectedIndex];
    putChar(pos, ' ');
}

/**
 * @brief Fonction qui remplace l'espace par le curseur.
 */
void Display::showSelected() {
    byte pos = item_pos[selectedIndex];
    putChar(pos, CURSOR);
}

/**
 * @brief Fonction qui met dans le tampon les 6 caractères de la valeur.
 */
void Display::showValue() {
    char str[8];
    modules[current_page - 1]->getString(str, selectedIndex);
    byte pos = item_pos[selectedIndex] + 1;
    for (int i = 0; i < 6; i++) {
        this->buffer[pos + i] = str[i];
    }
    this->current_pos = pos;
}

/**
 * @brief Fonction qui met dans le tampon les 6 caractères du nom du paramètre.
 */
void Display::hideValue() {
    Page p = pages[current_page];
    byte pos = item_pos[selectedIndex] + 1;
    for (int i = 0; i < 6; i++) {
        this->buffer[pos + i] = p.text[pos + i];
    }
    this->current_pos = pos;
}

/**
 * @brief Fonction qui change le numéro d'item. 
 */
void Display::change_selectedIndex(int8_t rotation) {
    Page p = pages[current_page];
    if (0 < rotation) {
        selectedIndex = (selectedIndex + 1) % p.num_item;
    } else if (rotation < 0) {
        selectedIndex += p.num_item;
        selectedIndex = (selectedIndex - 1) % p.num_item;
    }
}

/**
 * @brief Fonction qui change la valeur du paramètre entre le min et le max.
 *
 * Dans le cas particulier du séquenceur, désactive la note en ajoutant 128
 * à la hauteur, et dans le cas du générateur retourne aux valeurs limites.
 */
void Display::change_current_value(int8_t rotation) {
    Module *m = modules[current_page - 1];
    if (0 < rotation) {
        if (m->values[selectedIndex] < m->max[selectedIndex]) {
            m->values[selectedIndex]++;
        }
    } else if (rotation < 0) {
        if (m->min[selectedIndex] < m->values[selectedIndex] && 
                m->values[selectedIndex] <= m->max[selectedIndex]) { 
            // for sequence
            m->values[selectedIndex]--;
        }
    } else {
        if (m->on_press_value(selectedIndex)) {
            m->values[selectedIndex] = 1 - m->values[selectedIndex];
        }
    /*
        if (current_page == 4 && selectedIndex != 4) {
            if (selectedIndex == 5) {
                m->values[selectedIndex] = 0;
            } else if(selectedIndex == 6) {
                m->values[selectedIndex] = NOTE_MAX;
            }
            return;
        }
        */
        
    }
    m->set_value_constraint(selectedIndex);
}

/**
 * @brief Mise à jour de l'écran caractère par caractère à chaque loop.
 *
 * La mise à jour a lieu de la position courante du curseur jusqu'au 
 * 64-ième caractère.
 */
void Display::update() {
    if (this->current_pos == 64) {
        return;
    }
    while (this->current_pos < 64) {
        if (this->buffer[this->current_pos] != 
                this->screen[this->current_pos]) {
            this->screen[this->current_pos] = 
                this->buffer[this->current_pos];
            putChar(this->current_pos, this->screen[this->current_pos]);
            return;
        }
        this->current_pos++;
    }
}
