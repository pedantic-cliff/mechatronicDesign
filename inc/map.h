#ifndef _MAP_H_
#define _MAP_H_
#include "common.h"

void applyConfidence(int r, int c, pConfidences conf);
void sendGuesses(void);

void createGrid(void);
void finishGrid(void);

#endif //_MAP_H_
