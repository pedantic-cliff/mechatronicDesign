#ifndef _MAP_H_
#define _MAP_H_
#include "common.h"

void applyConfidence(int x, int y, confidences_t conf);
void sendGuesses(void);

void createGrid(void);

#endif //_MAP_H_
