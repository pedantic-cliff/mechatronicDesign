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
													{8200,  9000},  //RIGHT	+X
													{10500, 10200},	//UP		+Y
													{9000,  9100},	//LEFT	-X
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
												
MotorSpeeds encBiases[] = {
										{1.f,1.f},		//+X
										{1.f,1.f},		//+Y
										{1.f,1.f},		//-X
										{1.f,1.f}			//-Y
								  };

int numStates = sizeof(_targStates)/sizeof(state_t);
state_t _state_storage; 
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
}

void startState(void) {
	localizer->restart(localizer);
	findOutState();
  targState = &_state_storage;
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
  isTurning = 0;
  motionComplete = 0; 
  nextOrientationFlag = orientationFlag; 
  findOutState();
  USART_puts("Go forward: ");
  switch(orientationFlag){
    case POSX:
      targState->x = localizer->state->x + dist;
      localizer->setEncBias(localizer,encBiases[0].l,encBiases[0].r);
      speeds = &speedSettings[0];
      USART_putInt(localizer->state->x);
      USART_puts("->");
      USART_putInt(targState->x);
      USART_puts(" ");
      USART_putFloat(localizer->encBiasL);
      USART_puts(" ");
      USART_putFloat(localizer->encBiasR);
      break;
    case POSY:
      targState->y = localizer->state->y + dist;
      localizer->setEncBias(localizer,encBiases[1].l,encBiases[1].r); 
      speeds = &speedSettings[1];
      USART_putInt(localizer->state->y);
      USART_puts("->");
      USART_putInt(targState->y);
      USART_puts(" ");
      USART_putFloat(localizer->encBiasL);
      USART_puts(" ");
      USART_putFloat(localizer->encBiasR);
      break;
    case NEGX:
      targState->x = localizer->state->x - dist;
      localizer->setEncBias(localizer,encBiases[2].l,encBiases[2].r);
      speeds = &speedSettings[2];
      USART_putInt(localizer->state->x);
      USART_puts("->");
      USART_putInt(targState->x);
      USART_puts(" ");
      USART_putFloat(localizer->encBiasL);
      USART_puts(" ");
      USART_putFloat(localizer->encBiasR);
      break;
    case NEGY:
      targState->y = localizer->state->y - dist;
      localizer->setEncBias(localizer,encBiases[3].l,encBiases[3].r);
      speeds = &speedSettings[3];
      USART_putInt(localizer->state->y);
      USART_puts("->");
      USART_putInt(targState->y);
      USART_puts(" ");
      USART_putFloat(localizer->encBiasL);
      USART_puts(" ");
      USART_putFloat(localizer->encBiasR);
      break;
  }
  USART_puts("\n");
}

void turnLeft90(void){
  motionComplete = 0; 
  findOutState();
  USART_puts("Turn Left\n");
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
    switch(orientationFlag){
      case POSX:
        return (targState->theta - localizer->state->theta);
          
      case POSY:
        return (targState->theta - localizer->state->theta);

      case NEGX:
        return (targState->theta - localizer->state->theta);

      case NEGY:
        return (targState->theta - localizer->state->theta);
    }
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
  float theta; 
  if(motionComplete){
    motors->haltMotors(motors);
    return;
  }
  else if(isMotionComplete()){
    motionComplete = 1;
    orientationFlag = nextOrientationFlag;
    nextOrientationFlag = orientationFlag;
    motors->setSpeeds(motors,0,0);
    if(isTurning){
      motors->setOffset(motors,9300);
      motors->setOffset(motors,7500);
      motors->setOffset(motors,5500);
      motors->setOffset(motors,PWM_MIN);
    }
    return;
  }

  if(isTurning){
    motors->setOffset(motors,9000);
    theta = localizer->state->theta; 
    motors->setSpeeds(motors, cosf(theta)*speeds->l, cosf(theta)*speeds->r);
  } else {
    float err = calAngError();
    
    if(calculateError()>8)
    motors->setSpeeds(motors, speeds->l - 0.1*err, speeds->r + 0.1*err);
    else
    motors->setSpeeds(motors, speeds->l*(calculateError()/8 - 0.1*err), speeds->r*(calculateError()/8) + 0.1*err);
  }
}


