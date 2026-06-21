/**
 * Fichier : test_random_notes.c
 * Date : 21 juin 2026
 * Compilation :  gcc -Wall test_random_notes.c -lgsl
 *
 * Ici c'est le code pour générer une hauteur aléatoire dans 
 * une plage de valeur, suivant une tonalité et une gamme.
 *
 * Toutes les fonction font l'objet d'un test souvent systématique
 * (pour toutes les valeurs). 
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_cdf.h>

#define NB_SCALE 4

#define NOTE_MAX 144 // 12 volts -> 12 octaves

typedef unsigned char byte;

// pour un éventuel affichage
const char *STR_NOTES[12] = {
    " C", "C#", " D", "D#", " E", " F", "F#", " G", "G#", " A", "A#", " B"
};
const char *STR_SCALES[NB_SCALE] = {
    "CHROMA", "MAJOR ", "PENTA ", "HARMON"
};

// booléen indiquant si la note est dans la gamme :
const byte scales[NB_SCALE][12] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // chromatic
    {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, // major
    {1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0}, // pentatonic
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1}, // harmonic
};

// number of pitch lower or equals than the one 
// to compute with uniform distribution
// count[i][j] = sum_k=0^k=j scales[i][k]
byte count[NB_SCALE][12] = {
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}, // chromatic
    {1, 1, 2, 2, 3, 4, 4, 5, 5,  6,  6,  7}, // major
    {1, 1, 2, 2, 3, 3, 3, 4, 4,  5,  5,  5}, // pentatonic
    {1, 1, 2, 3, 3, 4, 4, 5, 6,  6,  6,  7}, // harmonic
};

// i-ième pitch relativement à C
// pitchs[i] = {j | scales[i][j] = 1}
byte pitchs[NB_SCALE][12] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, // chromatic
    {0, 2, 4, 5, 7, 9, 11},                 // major
    {0, 2, 4, 7, 9},                        // pentatonic
    {0, 2, 3, 5, 7, 8, 11},                 // harmonic
};

// teste la validité du tableau des hauteurs
void test_pitchs_array() {
    int c;
    for(int i = 0; i < NB_SCALE; i++) {
        c = 0;
        for(int j = 0; j < 12; j++) {
            if(scales[i][j]) {
                assert(j == pitchs[i][c]);
                c += scales[i][j];
            }
        }
    }
}

// retourne le nombre de notes strictement inférieures à N et vaut :
// (nombre de notes dans un gamme) * (nombre de gammes) +
//     nombre de notes inférieures ou égales dans la gamme de référence -
//     1 si la note N est dans la gamme
byte get_count(byte scale, byte note) {
    int size = count[scale][11];
    return size * (note / 12) + 
        count[scale][note % 12] - scales[scale][note % 12];
}

// pour les tests retourne le nombre par comptage
byte get_count_real(byte scale, byte note) {
    int count = 0;
    for (int i = 0; i < note; i++) {
        count += scales[scale][i % 12];
    }
    return count; 
}

// on teste tout
void test_get_count() {
    for (int scale = 0; scale < NB_SCALE; scale++) {
        for (int note = 0; note < NOTE_MAX; note++) {
            assert(get_count(scale, note) == get_count_real(scale, note));
        }
    }
}

// min inclus et max exclus
byte get_count_between(byte scale, byte min, byte max) {
    if (max <= min) {
        return 0;
    }
    return get_count(scale, max) - get_count(scale, min); 
}

// pour les tests retourne le nombre par comptage
byte get_count_between_real(byte scale, byte min, byte max) {
    int count = 0;
    for (int i = min; i < max; i++) {
        count += scales[scale][i % 12];
    }
    return count; 
}

// on teste tout
void test_get_count_between() {
    for (int scale = 0; scale < NB_SCALE; scale++) {
        for (int min = 0; min < NOTE_MAX; min++) {
            for (int max = 0; max < NOTE_MAX; max++) {
                assert(get_count_between(scale, min, max) == 
                        get_count_between_real(scale, min, max));
            }
        }
    }
}

// retourne la hauteur de la note en fonction de la gamme et de
// son rang. par exemple dans la gamme pentatonique, la hauteur de :
// - la note 0 est 0
// - la note 2 est 4
// - la note 4 est 9
// - la note 5 est 12
// - la note 22 (= 4 * 5 + 2) est 52 (4 * 12 + 4) 
byte get_pitch(byte scale, byte rank) {
    int size = count[scale][11];
    return 12 * (rank / size) + pitchs[scale][rank % size];    
}

// par comptage
byte get_pitch_real(byte scale, byte rank) {
    byte count = 0;
    byte i = 0;
    while(1) {
        count += scales[scale][i % 12];
        if (count == rank + 1) {
            break;
        }
        i++;
    }
    return i;
}

// on teste tout
void test_get_pitch() {
    for (int scale = 0; scale < NB_SCALE; scale++) {
        for (int rank = 0; rank < NOTE_MAX; rank++) {
            if (NOTE_MAX <= get_pitch_real(scale, rank)) {
                break;
            }
            assert(get_pitch(scale, rank) == get_pitch_real(scale, rank));
        }
    }    
}

// retourne un entier en a inclus et b exclus
byte _random(byte a, byte b) {
    if (b <= a) {
        return 0;
    }
    return a + rand() % (b - a);
}

// l'algorithme retourne une note au hasard entre min et max :
// - on translate en C relatif à l'octave supérieur
// - on compte le nombre a de note strictement inférieures au min
//   qui est le rang de la première note possible
// - on compte de même pour le max qui est le rang de la première 
//   note impossible
// - on prend un rang au hasard entre ces deux rangs
// - on retourne la hauteur de cette note ramenée à la tonalité 
byte get_rand_note(byte scale, byte tone, byte min, byte max) {
    byte a = get_count(scale, min - tone + 12);
    byte b = get_count(scale, max - tone + 12);
    byte rank = _random(a, b);
    return get_pitch(scale, rank) + tone - 12;
}

// pour 1000 notes on teste que la note aléatoire est :
// - entre le min et le max
// - dans la tonalité
// et que la distribution est uniforme (test du chi2)
void test_get_rand_note() {
    byte min;
    byte max;
    byte note;
    int t[NOTE_MAX];
    int count; // compteur pour le degré de liberté
    double chisq;
    double pvalue;
    double size = 100; // nombre de hauteurs 
    for (int scale = 0; scale < NB_SCALE; scale++) {
        for (int tone = 0; tone < 12; tone++) {
            min = 5;
            max = 20;
            for (int i = 0; i < NOTE_MAX; i++) {
                t[i] = 0;
            }
            // on génère la note avec les deux premiers tests
            for (int i = 0; i < size; i++) {
                note = get_rand_note(scale, tone, min, max);
                assert(min <= note && note < max);
                int j = note + 12 - tone;
                assert(scales[scale][j % 12]);
                t[j]++;
            }
            count = 0;
            // on calcule de nombre de notes distinctes
            for (int i = min + 12 - tone; i < max + 12 - tone; i++) {
                if (scales[scale][i % 12]) {
                    count++;
                }
            }
            chisq = 0;
            // on calcule le chi2
            for (int i = min + 12 - tone; i < max + 12 - tone; i++) {
                if (scales[scale][i % 12]) {
                    chisq += (t[i] - size / count) * (t[i] - size / count);
                }
                chisq = count * chisq / size;
            }
            // on calcule la p-value
            pvalue = 1 - gsl_cdf_chisq_P(chisq, count - 1);
            if (pvalue <= 0.95) {
                printf("df chisq p-value\n");
                printf("%2d %.3lf  %.4lf\n", count - 1, chisq, pvalue);
            }
            assert(0.95 < pvalue);
        }
    }
}

void to_string(char str[5], byte note) {
    sprintf(str, "%s%d", STR_NOTES[note % 12], note / 12);    
}

void test_to_string() {
    char str[5];
    for(int i = 0; i < 120; i++) {
        printf("%3d : ", i);
        to_string(str, i);
        printf("%s\n", str);
    }    
}

int main(void) {
    // pour la reproductibilité :
    srand(0);
    test_pitchs_array();
    test_get_count();
    test_get_count_between();
    test_get_pitch();
    test_get_rand_note();
    return 0;
}
