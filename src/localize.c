#include "localize.h"
#include "math.h"
#include "usart.h"

#define CLEAN_ANGLE(X) ( atan2f(cos(Y

static float compliFilter = 0.f;						//More implies more weight to accel
static struct localizer _storage; 

typedef struct enc{
  long L; 
  long R; 
} enc_t, *Enc;
static enc_t _enc;
static state_t _statePriv;
static state_t _state;


// Derive update, currently uses encoders to get dS and Accel to get theta
static void update(Localizer self){
  static int iters = 0; 
  iters++;
  float dSL, dSR, dS, dTheta;									//Updates from encoders
  
  int newL = self->m->getLeftCount(),						//Get encoder ticks
      newR = self->m->getRightCount();
  
  // Encoder differences										//Inverse Kinematics
  dSL = newL - self->enc->L; 
  dSR = newR - self->enc->R; 

  // Translate to position updates
  dS      =  ENC_TO_D((dSL + dSR) / 2.f);  
  dTheta  = fixAngle((ENC_TO_D(dSR - dSL)) / WHEEL_BASE_WIDTH);

  // Apply Rw = Rw + dRw
  self->_state->vel = dS;

  self->_state->x += dS * cosf(self->_state->theta + dTheta/2);  
  self->_state->y += dS * sinf(self->_state->theta + dTheta/2);  

  if (iters & 0x10){
    iters = 0; 
    // Get accel angle and do complimentary filter
    // This is a delay as accelerometer is much slower
    self->_state->theta = compliFilter*self->acc->getAngle() + 
                    (1-compliFilter)*(self->_state->theta + dTheta);
  } else { 
    self->_state->theta = (self->_state->theta + dTheta);
  }
  self->_state->theta = fixAngle(self->_state->theta);
  self->enc->L = newL;
  self->enc->R = newR;
  // TODO Update the Transforms here??
}

static void cacheState(Localizer self){
  self->state->x      = self->_state->x;
  self->state->y      = self->_state->y;
  self->state->theta  = self->_state->theta;
  self->state->vel    = self->_state->vel;
}

static void restart(Localizer self){
  self->_state->x       = 0.f;
  self->_state->y       = 0.f;
  self->_state->theta   = 0.f;//self->acc->getAngle();
  self->_state->vel     = 0.0f;

  self->cacheState(self);
}

Localizer createLocalizer(Motors m, Accel acc){
  Localizer l = &_storage;

  // Public state outside ISR
  l->state = &_state;
  // Private state inside ISR
  l->_state = &_statePriv;

  l->enc    = &_enc;
  l->enc->L = m->getLeftCount(); 
  l->enc->R = m->getRightCount();

  l->m    = m;
  l->acc  = acc;

  l->update     = update; 
  l->restart    = restart; 
  l->cacheState = cacheState;
  
  // Initialize state
  l->restart(l);

  return l;
}

