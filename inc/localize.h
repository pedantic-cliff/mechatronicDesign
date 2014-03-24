#ifndef _LOCALIZE_H_
#define _LOCALIZE_H_

#include "motors.h"
#include "accel.h"

typedef struct enc *Enc;

typedef struct localizer *Localizer;
typedef struct localizer{
  // Coordinates of robot in world frame
  struct {
    float x; 
    float y; 
    float theta; 
  } Rw;

  // Current counts of L and R encoders
  Enc enc; 

  // References localizing sensors
  Motors m; 
  Accel  acc;

  void (*update)(Localizer self);

} localizer_t;

Localizer createLocalizer(Motors m, Accel acc);

#endif //_LOCALIZE_H_
