#include "pid.h"
#include "utils.h"
#include "usart.h"
#include "common.h"
#include "localize.h"
#include "math.h"

static long posL, 
            posR; 

static long posL_targ,
            posR_targ;

static Pid_t _storage; 

typedef struct { 
  float p;
  float s; 
  float d; 
} error_t, *Error; 

Error calcErrTheta(const float targ, const float curr){
  static error_t e; 
  if(targ < -PI && curr > PI){
    e.p = (targ + 2*PI) - curr; 
  }else if(targ > PI && curr < -PI){
    e.p = (targ) - (curr + 2*PI);
  }else{
    e.p = (targ) - (curr);
  }
  if(fabsf(e.p) < PI/3)
    e.s = e.s + e.p;

  return &e; 
}

// Due to the nature of the motor being an integrator everything is 
// written as one derivative higher!
Error calcErrVel(const float targ, const float curr){ 
  static error_t ePrev,
                 err; 
  err.p = targ - curr + 0.8* ePrev.p; 
  ePrev.p = err.p; 
  return &err; 
}

//
// Loop expects two fully updated State vectors to calculate error. 
// 
static void loop(Pid self, State targ, State curr){
  Error errV, errA;
  float pidA = 0.f,
        pidV = 0.f;

  errA = calcErrTheta(targ->theta, curr->theta);
  errV = calcErrVel(targ->vel, curr->vel);
  pidA = self->angleGains.Kp * errA->p + self->angleGains.Ks * errA->s;
  pidV = self->velGains.Kp * errV->p;

  
 /* USART_puts("Angle Control: e = ");
  USART_putFloat(errA->p);
  USART_puts("\tu = ");
  USART_putFloat(pidA);
  USART_puts("\n\r");
  */
  /*
  errV = calcErrVel(targ->vel, curr->vel);
  USART_puts("Velocity Control: err = ");
  USART_putFloat(pidV);
  USART_puts("\n\r");
  */
  self->m->setSpeeds( pidA + pidV , -pidA + pidV );
}

Pid createPID(PID_Gains aGains, PID_Gains pGains, PID_Gains vGains, Motors m){
  Pid pid = & _storage;
  
  pid->angleGains.Kp = aGains.Kp;
  pid->angleGains.Ks = aGains.Ks;
  pid->angleGains.Kd = aGains.Kd;
  pid->posGains.Kp = pGains.Kp;
  pid->posGains.Ks = pGains.Ks;
  pid->posGains.Kd = pGains.Kd;
  pid->velGains.Kp = vGains.Kp;
  pid->velGains.Ks = vGains.Ks;
  pid->velGains.Kd = vGains.Kd;

  pid->m = m;

  pid->loop = loop;

  return pid;
}

