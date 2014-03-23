#include "pid.h"
#include "utils.h"


static float Kp = 0.1f;

static long posL, 
            posR; 

static long posL_targ,
            posR_targ;

pid_t _storage; 

float calcErrA(float targ, float curr){
  float errA = 0.f;
  if(targ < -PI && curr > PI){
    errA = (targ + 2PI) - curr; 
  }else if(targ > PI && curr < -PI){
    errA = (targ) - (curr + 2PI)
  }else{
    errA = (targ) - (curr);
  }

  return errA; 
}

void holdPos(Pid self, Point targ, State curr){
  float errL = 0.f,
        errR = 0.f,
        errA = 0.f;

  errA = calcErrA(targ->a, curr->a);

  if(fabsf(errA) > 0.01f){
    m->setSpeeds( -self->angleGains.Kp * errA, 
                   self->angleGains.Kp * errA ); 
    return; 
  }
}

void holdVel(Pid self, State curr, Point target){

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

  pid->holdPos  = holdPos;
  pid->holdVel  = holdVel;
}

