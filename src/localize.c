#include "localize.h"
#include "math.h"
#include "usart.h"

static float compliFilter = 0.75;						//More implies more weight to accel
static struct localizer _storage; 

typedef struct enc{
  long L; 
  long R; 
} enc_t, *Enc;
static enc_t _enc;
static state_t _state;



// Derive update, currently uses encoders to get dS and Accel to get theta
void update(Localizer self){
  float dSL, dSR, dS, dTheta;									//Updates from encoders
  
  int newL = self->m->getLeftCount(),						//Get encoder ticks
      newR = self->m->getRightCount();
  
  // Encoder differences										//Inverse Kinematics
  dSL = newL - self->enc->L; 
  dSR = newR - self->enc->R; 

  // Translate to position updates
  dS      = ENC_TO_D((dSL + dSR) / 2.f);  
  dTheta  = (dSR - dSL) / WHEEL_BASE_WIDTH;
  
  // Apply Rw = Rw + dRw
  self->state->vel = dS;

  self->state->x += dS * cosf(self->state->theta + dTheta/2);  
  self->state->y += dS * sinf(self->state->theta + dTheta/2);  
																		//Get accel angle and do complimentary filter
  self->state->theta = compliFilter*self->acc->getAngle() + 
  								(1-compliFilter)*(self->state->theta + dTheta);
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
  l->state->vel = 0.f;

  l->enc = &_enc;
  l->enc->L = m->getLeftCount(); 
  l->enc->R = m->getRightCount();

  l->m = m;
  l->acc = acc;

  l->update = update; 
  
  return l;
}

