#include "pid.h"
#include "utils.h"
#include "usart.h"
#include "common.h"
#include "localize.h"
#include "math.h"


static Pid_t _storage; 

typedef struct { 
  float p;
  float s; 
  float d; 
} error_t, *Error; 

static error_t eT = { 0.f, 0.f, 0.f };
Error calcErrTheta(const float targ, const float curr){
  float err; 
  if(targ < -PI && curr > PI){
    err = (targ + 2*PI) - curr; 
  }else if(targ > PI && curr < -PI){
    err = (targ) - (curr + 2*PI);
  }else{
    err = (targ) - (curr);
  }
  eT.d = err - eT.p;  // e.p still the old error 
  eT.p = err;
  if(fabsf(err) < PI/3)
    eT.s = eT.s + err;

  return &eT; 

}

// Due to the nature of the motor being an integrator everything is 
// written as one derivative higher!
static error_t eV = { 0.f, 0.f, 0.f };
Error calcErrVel(const float targ, const float curr){ 
  float err = targ - curr; 
  eV.d = err - eV.p;  // e.p still the old error
  USART_puts("D:");
  USART_putFloat(err);
  USART_puts(" - ");
  USART_putFloat(eV.p);
  USART_puts(" = ");
  USART_putFloat(err - eV.p);
  USART_puts("\r\n");
  eV.p = err; 
  if(fabsf(err) < 10.0f)
    eV.s = eV.s + err; 
  return &eV; 
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

  pidA = self->angleGains.Kp * errA->p 
    + self->angleGains.Ks * errA->s
    + self->angleGains.Kd * errA->d; 
  pidV = self->velGains.Kp * errV->p 
    + self->velGains.Ks * errV->s
    + self->velGains.Kd * errV->d; 

  USART_puts("Ae = ");
  USART_putFloat(errA->p);
  USART_puts("\t");
  USART_putFloat(errA->s);
  USART_puts("\t");
  USART_putFloat(errA->d);
  USART_puts("\n\r");
  USART_puts("Ve = ");
  USART_putFloat(errV->p);
  USART_puts("\t");
  USART_putFloat(errV->s);
  USART_puts("\t");
  USART_putFloat(errV->d);
  USART_puts("\n\r");
  
  self->m->setSpeeds(self->m, pidA + pidV , -pidA + pidV );
}
void setGains(Pid pid, PID_Gains aGains, PID_Gains pGains, PID_Gains vGains){
  pid->angleGains.Kp = aGains.Kp;
  pid->angleGains.Ks = aGains.Ks;
  pid->angleGains.Kd = aGains.Kd;
  pid->posGains.Kp = pGains.Kp;
  pid->posGains.Ks = pGains.Ks;
  pid->posGains.Kd = pGains.Kd;
  pid->velGains.Kp = vGains.Kp;
  pid->velGains.Ks = vGains.Ks;
  pid->velGains.Kd = vGains.Kd;
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
  pid->setGains = setGains;

  return pid;
}

