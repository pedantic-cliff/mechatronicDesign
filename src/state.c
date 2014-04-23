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
float AGain = 5500; 


MotorSpeeds speedSettings[] = 		{
  {9500,  8200},  //RIGHT	+X
  {10500, 9875},  //UP		+Y
  {9000,  9100},  //LEFT	-X
  {4000,  4500},  //DOWN	-Y
  {-12000,14500}, //LEFT 1
  {-13300,15500}, //LEFT 2
  {-14500,12500},					//LEFT 3
  {-12000,14500},					//LEFT 4
  {0,0},					//RIGHT 1
  {0,0},					//RIGHT 2
  {0,0},					//RIGHT 3
  {0,0}						//RIGHT 4
};

MotorSpeeds encBiases[] = {
  {1.f,1.f},		    //+X
  {0.865f,0.865f},  //+Y
  {1.f,1.f},	      //-X
  {0.925f,0.925f}		//-Y
};
state_t _state_storage; 
State targState;

typedef enum { POSX=0, POSY=1, NEGX=2, NEGY=3 } Orientation;
Orientation orientationFlag, nextOrientationFlag; 

float calculateError(void);
int isTurning;
int motionComplete; 

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
}

void goForwardBy(float dist){
  isTurning = 0;
  nextOrientationFlag = orientationFlag; 
  findOutState();

  __disable_irq();
  switch(orientationFlag){
    case POSX:
      targState->x = localizer->state->x + dist;
      localizer->setEncBias(localizer,encBiases[0].l,encBiases[0].r);
      speeds = &speedSettings[0];
      break;
    case POSY:
      targState->y = localizer->state->y + dist;
      localizer->setEncBias(localizer,encBiases[1].l,encBiases[1].r); 
      speeds = &speedSettings[1];
      break;
    case NEGX:
      targState->x = localizer->state->x - dist;
      localizer->setEncBias(localizer,encBiases[2].l,encBiases[2].r);
      speeds = &speedSettings[2];
      break;
    case NEGY:
      targState->y = localizer->state->y - dist;
      localizer->setEncBias(localizer,encBiases[3].l,encBiases[3].r);
      speeds = &speedSettings[3];
      break;
  }

  motionComplete = 0; 
  __enable_irq();
}

void turnLeft90(void){
  findOutState();
  __disable_irq();
  isTurning = 1;
  switch(orientationFlag){
    case POSX:
      nextOrientationFlag = POSY;
      speeds = &speedSettings[4];
      targState->theta = PI/2;
      break;
    case POSY:
      nextOrientationFlag = NEGX;
      speeds = &speedSettings[5];
      targState->theta = PI;
      break;
    case NEGX:
      nextOrientationFlag = NEGY;
      speeds = &speedSettings[6];
      targState->theta = -PI/2;
      break;
    case NEGY:
      nextOrientationFlag = POSX;
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
        return (targState->x <= localizer->state->x);

      case POSY:
        return (targState->y <= localizer->state->y);

      case NEGX:
        return (targState->x >= localizer->state->x);

      case NEGY:
        return (targState->y >= localizer->state->y);

    }
  } else {
    return calculateError() < 0.01f;
  }
  return 0;
}

float calculateError(void) {
  if(!isTurning){											//Added a not(!) here. Logic was reverse
    switch(orientationFlag){
      case POSX:
        return (targState->x - localizer->state->x);

      case POSY:
        return (targState->y - localizer->state->y);

      case NEGX:
        return -(targState->x - localizer->state->x);

      case NEGY:
        return -(targState->y - localizer->state->y);

    }
  } else {
    switch(orientationFlag){
      case POSX:
        return fixAngle(targState->theta - localizer->state->theta);

      case POSY:
        return fixAngle(targState->theta - localizer->state->theta);

      case NEGX:
        return fixAngle(targState->theta - localizer->state->theta);

      case NEGY:
        return fixAngle(targState->theta - localizer->state->theta);
    }
  }
  return 0.f;
}

float calAngError(void) {
	switch(orientationFlag){
      case POSX:
        return fixAngle(0 - localizer->state->theta);
          
      case POSY:
        return fixAngle(PI/2 - localizer->state->theta);

      case NEGX:
        return fixAngle(PI - localizer->state->theta);

      case NEGY:
        return fixAngle(-PI/2 - localizer->state->theta);
    }
    return 0;
}

void doMotion(void){
  float errA, theta, err;
  //int i = 0; 

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
    orientationFlag = nextOrientationFlag;
    nextOrientationFlag = orientationFlag;
    motors->haltMotors(motors);
    //delay_blocking(100);
    motors->setOffset(motors,PWM_MIN_L,PWM_MIN_R);
    motors->setSpeeds(motors,0,0);

    USART_puts("Done Motion\n");
    return;
  }
  
  if(isTurning)
  {
    	motors->setOffset(motors,9000,9000);
    	theta = targState->theta - localizer->state->theta; 
    	if(err < PI/6.f)
      	motors->setSpeeds(motors, sinf(theta)*speeds->l * err / (PI/6.f), sinf(theta)*speeds->r * err / (PI/6.f));
    	else 
      	motors->setSpeeds(motors, sinf(theta)*speeds->l, sinf(theta)*speeds->r);
  } 
  else 
  {
		if(err>2.f)
      	motors->setSpeeds(motors, speeds->l - AGain*errA, speeds->r + AGain*errA);
    	else
      	switch(orientationFlag)
      	{
        		case POSY:
          		motors->setSpeeds(motors, speeds->l*err/2.f - AGain*errA + 9000, speeds->r*err/2.f + AGain*errA + 9000);
          		break;
        		case NEGY:
          		motors->setSpeeds(motors, speeds->l*err/2.f - AGain*errA, speeds->r*err/2.f + AGain*errA);
          		break;
        		default:
          		motors->setSpeeds(motors, speeds->l*err/2.f - AGain*errA, speeds->r*err/2.f + AGain*errA);
      	}
	}
}
