#include "localize.h"
#include "math.h"
#include "usart.h"

static struct localizer _storage; 

typedef struct enc{
  long L; 
  long R; 
} enc_t, *Enc;
static enc_t _enc;
static state_t _state;



// Derive update, currently uses encoders to get dS and Accel to get theta
void update(Localizer self){
  float dSL, dSR, dS, dTheta; // Updates from encoders
  int newL = self->m->getLeftCount(),
      newR = self->m->getRightCount();
  self->state->theta = self->acc->getAngle(); 
  
  // Encoder differences
  dSL = newL - self->enc->L; 
  dSR = newR - self->enc->R; 

  // Translate to position updates
  dS      = ENC_TO_D((dSL + dSR) / 2.f);  
  dTheta  = (dSR - dSL) / WHEEL_BASE_WIDTH;
  // Apply Rw = Rw + dRw
  self->state->vx = dS * cosf(self->state->theta + dTheta); 
  self->state->x += self->state->vx;

  self->state->vy = dS * sinf(self->state->theta + dTheta); 
  self->state->y += self->state->vy; 

  self->enc->L = newL;
  self->enc->R = newR;
  // TODO Update the Transforms here??
}

Localizer createLocalizer(Motors m, Accel acc){
  Localizer l = &_storage;

  l->state = &_state;
  l->state->x = 0.f;
  l->state->y = 0.f;
  l->state->theta = acc->getAngle();
  l->state->vx = 0.f;
  l->state->vy = 0.f;

  l->enc = &_enc;
  l->enc->L = m->getLeftCount(); 
  l->enc->R = m->getRightCount();

  l->m = m;
  l->acc = acc;

  l->update = update; 
  
  return l;
}

