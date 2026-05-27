/**
 * @file CVGATE_tools.ino
 * @brief Fichier principal du projet 
 * 
 * Le module envoie des volts avec possibilité de synchronisation.
 * @author Pierrick MEIGNEN 
 * @version 1.0
 * @date dim. 16 mars 2025 17:54:07 CET
 */
#include <U8x8lib.h>
#include <Versatile_RotaryEncoder.h>
#include <avr/pgmspace.h>

/*** base ***/
#include "src/base/Display.h"
#include "src/base/Page.h"
#include "src/base/encoder.h"
#include "src/base/dac.h"

/*** modules ***/
#include "src/euclidean/Euclidean.h"
#include "src/sequencer/Sequencer.h"
#include "src/random/Random.h"
#include "src/time/Time.h"

/**
 * Sur le pin du clock in, on reçoit toujours 5 volts par le transistor et
 * on lit 1024 sur ce pin. Sauf quand un signal est reçu, le voltage 
 * baisse et on lit moins. On considère qu'un signal est reçu lorsqu'on 
 * passe en dessous d'un seuil. La valeur est expérimentale avec une 
 * sensibilté de l'ordre de 2%.
 */ 
#define THRESHOLD 1000

/**Le tableau des deux encodeurs SW2 et SW1 dans l'ordre.*/
Versatile_RotaryEncoder *versatile_encoder[NUM_ENC];
#if defined(NANO)
int sws[2]  = {7, 2}; /**<pin des switchs */
int dts[2]  = {6, 3}; /**<pin des datas */
int clks[2] = {5, 4}; /**<pin des clocks */ 
#elif defined(EVERY)
int sws[2]  = {7, 2}; /**<pin des switchs */
int dts[2]  = {6, 3}; /**<pin des datas */
int clks[2] = {5, 4}; /**<pin des clocks */ 
#elif defined(THINARY)
int sws[2]  = {8, 0}; /**<pin des switchs */
int dts[2]  = {7, 2}; /**<pin des datas */
int clks[2] = {6, 3}; /**<pin des clocks */ 
#endif

Display oled; /**<l'écran */
/**
 * @brief liste des modules 
 */
Module *modules[NUM_PAGE - 1] = {
    new Time(), 
    new Euclidean(), 
    new Sequence(), 
    new Random()};

const char str_0[] PROGMEM = 
" TIME            EUCLIDEAN       5-SEQ           RANDOM        ";
const char str_1[] PROGMEM = 
" CLOCK   CHAOS   BPM             MULT            DIVIDE        ";
const char str_2[] PROGMEM = 
" LENGTH  LENGTH  BEATS   BEATS   SHIFT   SHIFT   GATE    GATE  ";
const char str_3[] PROGMEM = 
" LENGTH  NOTE 4  NOTE 1  NOTE 5  NOTE 2  SPEED   NOTE 3  GATE  ";
const char str_4[] PROGMEM = 
" LENGTH  FREEZE  SCALE   MIN     KEY     MAX     GATE    CHANGE";
const char *const str_table[] PROGMEM = {
    str_0, str_1, str_2, str_3, str_4};

/**
 * Suivant la page il y a entre 4 et 8 items.
 */
byte selectedIndex;

/**
 * Il y a 5 pages : le menu principal et 4 modules.
 */
byte current_page;

unsigned long tick_num; /**<Numéro de double-croche. */
unsigned long lastTimeTick; /**<L'instant de la dernière double-croche. */
unsigned long delayBetweenTick; /**<Durée d'une double-croche. */
unsigned long now; /**<L'instant où on reçoit l'impulsion. */
bool externalClock; /**<Vrai si l'horloge est externe. */
bool start; /**<Booléen indiquant si on a reçu une nouvelle impulsion. */
bool start_gates; /**<Booléen indiquant si on arrive sur un tick d'horloge. */

/* init pins out (led blue, blue, red, yellow, yellow)*/
byte pins[] = {SEQ_OUT, TM_OUT, CLOCK_OUT, EUC2_OUT, EUC1_OUT, SS};

/**
 * Nombre d'items par page
 */
Page pages[NUM_PAGE] = {{4},{5},{8},{8},{8}};

/**
 * Pour corriger un bug d'affectation de pin analog 
 * sinon la sortie est toujours 255.
 *
 * https://github.com/MCUdude/MegaCoreX/issues/128
 *
 * code source :
 * https://github.com/Thinary/ThinaryArduino/blob/master/Thinary-megaavr/cores/arduino/wiring_analog.c
 */
#if defined(THINARY)
int myAnalogRead(uint8_t pin) {
    pin = digitalPinToAnalogInput(pin);
    if(pin > 15) return NOT_A_PIN;

    /* Check if TWI is operating on double bonded pin (Master Enable is high 
       in both Master and Slave mode for bus error detection, so this can 
       indicate an active state for Wire) */
    if(isDoubleBondedActive(pin)) return 0;

    uint8_t low, high;

#if defined(analogPinToChannel)
    /* If analog pin number != adc0 channel */
#endif

#if defined(ADC0)
    /* Reference should be already set up */
    /* Select channel */
    ADC0.MUXPOS = (pin << ADC_MUXPOS_gp);

    /* Start conversion */
    ADC0.COMMAND = ADC_STCONV_bm;

    /* Wait for result ready */
    while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));

    /* Save state */
    uint8_t status = SREG;
    cli();

    /* Read result */
    low = ADC0.RESL;
    high = ADC0.RESH;

    /* Restore state */
    SREG = status;

#else	/* No ADC, return 0 */
    low = 0;
    high = 0;
#endif

    /* Combine two bytes */
    return (high << 8) | low;
}
#else
#define myAnalogRead analogRead
#endif

/**
 * @brief Gestion principale de l'horloge en double-croches.
 * 
 * Si l'horloge est externe alors on lit le signal reçu. On incrémente
 * le compteur, on démarre le gate de chaque module et on met à jour 
 * le délai entre chaque tick.
 *
 * Si l'horloge est interne alors en fonction du temps écoulé
 * on incrémente le compteur et on démarre le gate de chaque module.
 */
void handle_time() {
    now = micros();
    start_gates = false;
    if (externalClock) {
        int val = myAnalogRead(CLOCK_IN);
        if (val <= THRESHOLD && !start) {            
            start = true;
            tick_num++;
            start_gates = true;
            delayBetweenTick = 
                (now - lastTimeTick + 2 * delayBetweenTick) / 3;
            lastTimeTick = now;
            if(current_page == 1 && selectedIndex == 1) {
                oled.showValue();
            } 
        } else if (THRESHOLD < val && start) {
            start = false;
        }
    } else {  
        if (now - lastTimeTick >= delayBetweenTick) {
            lastTimeTick += delayBetweenTick;
            tick_num++;
            start_gates = true;
        }
    }
}

/**
 * @brief Initialisation.
 *
 * On allume les leds à tour de rôle, on charge le texte de chaque page,
 * on affiche le menu principal puis on déclare les encodeurs et les 
 * fonctions associées.
 */
void setup() {
    SPI.begin();
    /* init values */
    current_page = 0;
    selectedIndex = 0;
    externalClock = false;
    start = false;
    tick_num = 0;
    delayBetweenTick = 15000000 / BPM_DEFAULT;
    lastTimeTick = micros();

    /* display */
    oled.init();
    for (int i = 0; i < 5; i++) {
        strcpy_P(pages[i].text, (char *)pgm_read_word(&(str_table[i])));  
    } 
    oled.welcome();
    for (byte i = 0; i < 6; i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], LOW);
    }
    for (byte i = 0; i < 5; i++) {
        digitalWrite(pins[i], HIGH);
        delay(500);
    }
    for (byte i = 0; i < 5; i++) {
        digitalWrite(pins[i], LOW);
    }
      
    oled.display();
    oled.showSelected();
    /* rotary encoders */
    for (int i = 0; i < NUM_ENC; i++) {
        versatile_encoder[i] =
            new Versatile_RotaryEncoder(clks[i], dts[i], sws[i]);
    }
    versatile_encoder[0]->setHandleRotate(handleRotate0);
    versatile_encoder[0]->setHandlePress(handlePress0);
    versatile_encoder[0]->setHandleLongPress(handleLongPress0);
    versatile_encoder[1]->setHandleRotate(handleRotate1);
    versatile_encoder[1]->setHandlePress(handlePress1);
}

/**
 * @brief La boucle du micro-contrôleur.
 *
 * On lit les encodeurs, on met à jour l'écran, on incrémente le
 * compteur de ticks et on gère chaque module.
 */
void loop() {
    for (int i = 0; i < NUM_ENC; i++) {
        versatile_encoder[i]->ReadEncoder();
    }
    oled.update();
    handle_time();
    for (int i = 0; i < 4; i++) {
        modules[i]->play();
    }
}

