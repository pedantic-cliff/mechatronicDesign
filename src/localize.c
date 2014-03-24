#include "localize.h"
#include "constants.h"
#include "math.h"
#include "usart.h"

static struct localizer _storage; 

typedef struct enc{
  long L; 
  long R; 
} enc_t, *Enc;
static enc_t _enc;

// Derive update, currently uses encoders to get dS and Accel to get theta
void update(Localizer self){
  float dSL, dSR, dS, dTheta; // Updates from encoders
  int newL = self->m->getLeftCount(),
      newR = self->m->getRightCount();
  self->Rw.theta = self->acc->getAngle(); 
  
  // Encoder differences
  dSL = newL - self->enc->L; 
  dSR = newR - self->enc->R; 

  // Translate to position updates
  dS      = ENC_TO_D((dSL + dSR) / 2.f);        // Stable as long as dSL,dSR are not too large
  dTheta  = (dSR - dSL) / WHEEL_BASE_WIDTH;
  // Apply Rw = Rw + dRw
  self->Rw.x += dS * cosf(self->Rw.theta + dTheta); 
  self->Rw.y += dS * sinf(self->Rw.theta + dTheta); 

  self->enc->L = newL;
  self->enc->R = newR;
  // TODO Update the Transforms here??
}

Localizer createLocalizer(Motors m, Accel acc){
  Localizer l = &_storage;
  
  l->Rw.x = 0.f;
  l->Rw.y = 0.f;
  l->Rw.theta = acc->getAngle();

  l->enc = &_enc;
  l->enc->L = m->getLeftCount(); 
  l->enc->R = m->getRightCount();

  l->m = m;
  l->acc = acc;

  l->update = update; 
  
  return l;
}

