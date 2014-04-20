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

state_t _targStates[] = {
                         {24.f,   0.f,    0.f,      0.f},
                         {0.0f,   0.f,    PI/2.0f,  0.f},
                         {24.f,   24.f,   PI/2.0f,  0.f},
                         {24.f,   24.f,   PI,       0.f},
                         {0.0f,   24.f,   PI,       0.f},
                         {0.0f,   24.f,  3*PI/2.0f, 0.f},
                         {0.0f,   12.0f, 3*PI/2.0f, 0.f},
                         {0.0f,   12.0f,  0.0f,     0.f},
                         {12.0f,  12.0f,  0.0f,    0.f}
												};
												
 MotorSpeeds speedSettings[] = 		{
													{8200,9000},		//RIGHT	+X
													{10500,10200},	//UP		+Y
													{9000,6400},		//LEFT	-X
													{0,0},					//DOWN	-Y
													{-10000,13000},	//LEFT 1
													{0,0},					//LEFT 2
													{0,0},					//LEFT 3
													{0,0},					//LEFT 4
													{0,0},					//RIGHT 1
													{0,0},					//RIGHT 2
													{0,0},					//RIGHT 3
													{0,0}						//RIGHT 4
												};

int numStates = sizeof(_targStates)/sizeof(state_t);
State targState;

typedef enum { POSX=0, POSY=1, NEGX=2, NEGY=3 } Orientation;
Orientation orientationFlag, nextOrientationFlag; 

int isTurning;
int motionComplete; 
static int started;

void findOutState(void) {
	float theta = atan2f(sinf(localizer->_state->theta),cosf(localizer->_state->theta));
	
	if((theta>=-PI/4 && theta<=0) || (theta<PI/4 && theta>=0))
  	orientationFlag = POSX;
	if(theta>=PI/4 && theta<3*PI/4)
	  orientationFlag = POSY;
	if((theta>=3*PI/4 && theta<=PI) || (theta<-3*PI/4 && theta>=-PI) )
  	orientationFlag = NEGX;
	if(theta<-PI/4 && theta>=-3*PI/4 )
	  orientationFlag = POSX;
  USART_puts("Found state: ");
  USART_putInt(orientationFlag);
  USART_puts("\n");
}

void startState(void) {
	localizer->restart(localizer);
	findOutState();
  targState->x = localizer->state->x;
  targState->y = localizer->state->y;
  targState->theta = localizer->state->theta;
}

void markStarted(void){
  started = 0; 
}
int isStarted(void){
  return started;
}
/*
  switch(orientationFlag){
    case POSX:
      break;
    case POSY:
      break;
    case NEGX:
      break;
    case NEGY:
      break;
  }
*/
void goForwardBy(float dist){
  USART_puts("Go forward\n");
  isTurning = 0;
  motionComplete = 0; 
  nextOrientationFlag = orientationFlag; 
  switch(orientationFlag){
    case POSX:
      targState->x += dist; 
      speeds = &speedSettings[0];
      break;
    case POSY:
      targState->y += dist; 
      speeds = &speedSettings[1];
      break;
    case NEGX:
      targState->x -= dist; 
      speeds = &speedSettings[2];
      break;
    case NEGY:
      targState->y -= dist; 
      speeds = &speedSettings[3];
      break;
  }
}

void turnLeft90(void){
  motionComplete = 0; 
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
}

int isMotionComplete(void){
  if(isTurning){
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
    switch(orientationFlag){
      case POSX:
        return (targState->theta >= localizer->state->theta);
          
      case POSY:
        return (0 <= localizer->state->theta || localizer->state->theta  < -PI/2);

      case NEGX:
        return (targState->theta <= localizer->state->theta);

      case NEGY:
        return (targState->y <= localizer->state->theta);
    }
  }
  return 0;
}

void doMotion(void){
  float theta; 
  if(motionComplete)
    return;
  else if(isMotionComplete()){
    motionComplete = 1;
    motors->setOffset(motors,9300);
    motors->setOffset(motors,7500);
    motors->setOffset(motors,5500);
    motors->setOffset(motors,PWM_MIN);
    return;
  }

  if(isTurning){
    motors->setOffset(motors,9000);
    theta = localizer->state->theta; 
    motors->setSpeeds(motors, cosf(theta)*speeds->l, cosf(theta)*speeds->r);
  } else {
    motors->setSpeeds(motors, speeds->l, speeds->r);
  }
}


