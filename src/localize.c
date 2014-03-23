#include "localize.h"
#include "constants.h"
#include "math.h"


localizer_t _storage; 

// Derive update, currently uses encoders to get dS and Accel to get theta
void update(Localizer self){
  float dSL, dSR, dS, dTheta; // Updates from encoders

  int newL = self->m->getLeftCount(),
      newR = self->m->getRightCount();

  // Encoder differences
  dSL = newL - self->encoders.L; 
  dSR = newR - self->encoders.R; 

  // Translate to position updates
  dS      = ENC_TO_D((dSL + dSR) / 2.f);        // Stable as long as dSL,dSR are not too large

  // Apply Rw = Rw + dRw
  self->Rw.x += dS * cosf(self->Rw.theta + dTheta); 
  self->Rw.y += dS * sinf(self->Rw.theta + dTheta); 

  self->Rw.theta = self->acc->getAngle(); 

  // TODO Update the Transforms here??
}

Localizer createLocalizer(Motors m, Accel acc){
  Localizer l = &_storage;
  
  l->m = m;
  l->acc = acc
  l->Rw.x = 0.f;
  l->Rw.y = 0.f;
  l->Rw.theta = acc->getAngle();

  l->encoders.L = m->getLeftCount();
  l->encoders.R = m->getRightCount();

  l->update = update; 
  
  return l;
}

