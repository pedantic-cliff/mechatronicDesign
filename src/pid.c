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

float calcErrTheta(const float targ, const float curr){
  float errA = 0.f;
  if(targ < -PI && curr > PI){
    errA = (targ + 2*PI) - curr; 
  }else if(targ > PI && curr < -PI){
    errA = (targ) - (curr + 2*PI);
  }else{
    errA = (targ) - (curr);
  }

  return errA; 
}

float calcErrVel(const float targ, const float curr){ 

}

//
// Loop expects two fully updated State vectors to calculate error. 
// 
static void loop(Pid self, State targ, State curr){
  float errL = 0.f,
        errR = 0.f,
        errA = 0.f;

  errA = calcErrTheta(targ->theta, curr->theta);
  // If angle is off: correct that rather than make position more off. 
  if(fabsf(errA)){
    USART_puts("Angle Control: err = ");
    USART_putInt(self->angleGains.Kp*errA);
    USART_puts("\n\r");
    self->m->setSpeeds(  self->angleGains.Kp * errA, 
                        -self->angleGains.Kp * errA ); 
    return; 
  }

  // Now hold the velocity in the given direction

}

Pid createPID(PID_Gains aGains, PID_Gains pGains, PID_Gains vGains, Motors m){
  Pid pid = & _storage;
  
  pid->angleGains.Kp = aGains.Kp;
  pid->angleGains.Ki = aGains.Ki;
  pid->angleGains.Kd = aGains.Kd;
  pid->posGains.Kp = pGains.Kp;
  pid->posGains.Ki = pGains.Ki;
  pid->posGains.Kd = pGains.Kd;
  pid->velGains.Kp = vGains.Kp;
  pid->velGains.Ki = vGains.Ki;
  pid->velGains.Kd = vGains.Kd;

  pid->m = m;

  pid->loop = loop;

  return pid;
}

