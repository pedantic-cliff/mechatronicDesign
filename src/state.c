#include <stdio.h>
#include "motors.h"
#include "pid.h"
#include "map.h"
#include "localize.h"
#include "common.h"
#include "utils.h"
#include "state.h"
#include "math.h"
#include "main.h"
#include "usart.h"

MotorSpeeds *speeds;
Localizer localizer;
float AGain = 11000; 
float IntGain = 5.f; 
float errI = 0.f;

MotorSpeeds speedSettings[] = 		{
  {9500*0.91,   8200*1.10},  //RIGHT	+X
  {8000*0.82,   7400*0.99},  //UP		+Y
  {6750*1.65f,   6800*1.35f},  //LEFT	-X
  {8000*1.0,    8500*1.0},	//DOWN	-Y
  {-12000*0.95f,  14500*0.95},   //LEFT 1
  {-12300*1.23, 15500*1.5}, //LEFT 2
  {-14500*1.05, 12500*1.3},	//LEFT 3
  {-12000*0.85, 14500*0.9},	//LEFT 4
  {0,0},					//RIGHT 1
  {0,0},					//RIGHT 2
  {0,0},					//RIGHT 3
  {0,0}						//RIGHT 4
};

MotorSpeeds encBiases[] = {
  {0.99f,0.99f},		              //+X
  {0.95f, 0.95f},//{0.97f,0.87f},                //+Y
  {1.0, 1.0f},	            //-X
  {1.3f,1.3f}//{0.925f*1.45f,0.925f*1.45f}		//-Y
};
state_t _state_storage; 
State targState;

typedef enum { POSX=0, POSY=1, NEGX=2, NEGY=3 } Orientation;
Orientation orientationFlag, nextOrientationFlag; 

float calculateError(void);
int isTurning;
int motionComplete; 
int isStalling; 
void findOutState(void) {
  float theta = atan2f(sinf(localizer->_state->theta),cosf(localizer->_state->theta));

  if((theta>=-PI/4 && theta<=0) || (theta<PI/4 && theta>=0))
    orientationFlag = POSX;

  if(theta>=PI/4 && theta<3*PI/4)
    orientationFlag = POSY;

  if((theta>=3*PI/4 && theta<=PI) || (theta<-3*PI/4 && theta>=-PI) )
    orientationFlag = NEGX;

  if(theta<-PI/4 && theta>=-3*PI/4 )
    orientationFlag = NEGY;
}

void startState(void) {
  localizer->restart(localizer);
  findOutState();
  targState = &_state_storage;
  targState->x = localizer->state->x;
  targState->y = localizer->state->y;
  targState->theta = localizer->state->theta;
  errI = 0.f;
}

void goForwardBy(float dist){
  findOutState();

  errI = 0.f;
  __disable_irq();
  nextOrientationFlag = orientationFlag; 
  isTurning = 0;
  isStalling = 0; 
  switch(orientationFlag){
    case POSX:
      targState->x = dist;//localizer->state->x + dist;
      localizer->isHorizontal = 1;
      localizer->setEncBias(localizer,encBiases[0].l,encBiases[0].r);
      speeds = &speedSettings[0];
      break;
    case POSY:
      targState->y = dist;//localizer->state->y + dist;
      localizer->state->x += 1;   //This is a hack to make up a persistent error
      localizer->isHorizontal = 0;
      localizer->setEncBias(localizer,encBiases[1].l,encBiases[1].r); 
      speeds = &speedSettings[1];
      break;
    case NEGX:
      targState->x = dist;//localizer->state->x - dist;
      localizer->isHorizontal = 1;
      localizer->setEncBias(localizer,encBiases[2].l,encBiases[2].r);
      speeds = &speedSettings[2];
      break;
    case NEGY:
      targState->y = dist;//localizer->state->y - dist;
      localizer->isHorizontal = 0;
      localizer->setEncBias(localizer,encBiases[3].l,encBiases[3].r);
      speeds = &speedSettings[3];
      break;
  }

  motionComplete = 0; 
  __enable_irq();
}

void turnLeft90(void){
  findOutState();
  errI = 0.f;
  __disable_irq();
  isTurning = 1;
  isStalling = 0; 
  switch(orientationFlag){
    case POSX:
      nextOrientationFlag = POSY;
      localizer->isHorizontal = 2;
      speeds = &speedSettings[4];
      targState->theta = PI/2;
      break;
    case POSY:
      nextOrientationFlag = NEGX;
      localizer->isHorizontal = 2;
      speeds = &speedSettings[5];
      targState->theta = PI;
      break;
    case NEGX:
      nextOrientationFlag = NEGY;
      localizer->isHorizontal = 2;
      speeds = &speedSettings[6];
      targState->theta = -PI/2;
      break;
    case NEGY:
      nextOrientationFlag = POSX;
      localizer->isHorizontal = 2;
      speeds = &speedSettings[7];
      targState->theta = 0;
      break;
  }
  motionComplete = 0; 
  __enable_irq();
}

int isMotionComplete(void){
  if(!isTurning){											//Added a not(!) here. Logic was reverse
    switch(orientationFlag){
      case POSX:
        return (targState->x - 0.6f <= localizer->_state->x);

      case POSY:
        return (targState->y - 0.35f <= localizer->_state->y);

      case NEGX:
        return (targState->x + 0.35f >= localizer->_state->x);

      case NEGY:
        return (targState->y + 0.6f >= localizer->_state->y);

    }
  } else {
    return fabsf(calculateError()) < 0.15f;
  }
  return 0;
}

float calculateError(void) {
  if(!isTurning){											//Added a not(!) here. Logic was reverse
    switch(orientationFlag){
      case POSX:
        return (targState->x - localizer->_state->x);

      case POSY:
        return (targState->y - localizer->_state->y);

      case NEGX:
        return -(targState->x - localizer->_state->x);

      case NEGY:
        return -(targState->y - localizer->_state->y);

    }
  } else {
    switch(orientationFlag){
      case POSX:
        return fixAngle(targState->theta - localizer->_state->theta);

      case POSY:
        return fixAngle(targState->theta - localizer->_state->theta);

      case NEGX:
        return fixAngle(targState->theta - localizer->_state->theta);

      case NEGY:
        return fixAngle(targState->theta - localizer->_state->theta);
    }
  }
  return 0.f;
}

float calAngError(void) {
  if(!isTurning){
    switch(orientationFlag){
      case POSX:
        return fixAngle(0 - localizer->_state->theta);

      case POSY:
        return fixAngle(PI/2 - localizer->_state->theta);

      case NEGX:
        return fixAngle(PI - localizer->_state->theta);

      case NEGY:
        return fixAngle(-PI/2 - localizer->_state->theta);
    }
  }else{
    switch(nextOrientationFlag){
      case POSX:
        return fixAngle(0 - localizer->_state->theta);

      case POSY:
        return fixAngle(PI/2 - localizer->_state->theta);

      case NEGX:
        return fixAngle(PI - localizer->_state->theta);

      case NEGY:
        return fixAngle(-PI/2 - localizer->_state->theta);
    }
  }
  return 0;
}

void doStall(void){
  __disable_irq();
  isStalling = 1;
  motors->haltMotors(motors);
  __enable_irq();
}

void doMotion(void){
  float errA, theta, err;
  //int i = 0; 

  if(isStalling){
    motors->haltMotors(motors);
    return;
  }

  findOutState();
  err = calculateError();
  errA = calAngError();

  if(motionComplete)
  {
    //motors->haltMotors(motors);
    motors->setSpeeds(motors, -6*AGain * errA, +6*AGain *errA);
    return;
  }

  else if(isMotionComplete())
  {
    motionComplete = 1;
    errI = 0.f; 
    orientationFlag = nextOrientationFlag;
    nextOrientationFlag = orientationFlag;
    motors->haltMotors(motors);
    motors->setOffset(motors,PWM_MIN_L,PWM_MIN_R);
    motors->setSpeeds(motors,0,0);

    USART_puts("Done Motion\n");
    return;
  }

  if(isTurning)
  {
    motors->setOffset(motors,9000,9000);
    theta = fixAngle(targState->theta - localizer->_state->theta); 
    if(err < PI/6.f){

      errI += err;
      motors->setSpeeds(motors, 
          sinf(theta)*speeds->l * err / (PI/6.f) - 0.05* AGain *errA - 1000 - errI*IntGain, 
          sinf(theta)*speeds->r * err / (PI/6.f) + 0.05* AGain *errA + 1000 + errI*IntGain );
    } else {
      errI = 0.f; 
      motors->setSpeeds(motors, sinf(theta)*speeds->l - errA*0.1*AGain, 
          sinf(theta)*speeds->r + errA*0.1*AGain);
    }
  } 
  else 
  {
    if(err>2.f){
      motors->setSpeeds(motors, speeds->l - AGain*errA, speeds->r + AGain*errA);
      if(orientationFlag!=POSY){
        errI = 0;
      }

    } else {
      errI += err;
      switch(orientationFlag)
      {
        case POSY:
          motors->setSpeeds(motors, 
              speeds->l*err/2.f - AGain*errA + 10000 + errI*10*IntGain, 
              speeds->r*err/2.f + AGain*errA + 10000 + errI*10*IntGain);
          break;
        default:
          motors->setSpeeds(motors, 
              speeds->l*err/2.f - AGain*errA + IntGain*errI, 
              speeds->r*err/2.f + AGain*errA + IntGain*errI);
      }
    }
  }
}
