#include "pid.h"
#include "motors.h"


static float Kp = 0.1f;

static long posL, 
            posR; 

static long posL_targ,
            posR_targ;

void startHoldPos(void){
  posL_targ = m->getLeftCount(); 
  posR_targ = m->getRightCount();

}
void holdPos(void){
  float errL = 0.f,
        errR = 0.f;
  posL = m->getLeftCount(); 
  posR = m->getRightCount(); 
  
  errL = posL_targ - posL;
  errR = posR_targ - posR; 
  
  m->setSpeeds( Kp * errL, 
                Kp * errR ); 
  
}


