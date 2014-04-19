#include <stdio.h>
#include "motors.h"
#include "pid.h"
#include "map.h"
#include "localize.h"
#include "common.h"
#include "utils.h"
#include "state.h"

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
													{8200,9000},			//RIGHT	+X
													{10500,10200},			//UP		+Y
													{9000,6400},			//LEFT	-X
													{0,0},					//DOWN	-Y
													{0,0},					//LEFT 1
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

void findOutState(void) {
	float theta = atan2f(sinf(localizer->_state->theta),cosf(localizer->_state->theta));
	
	if((theta>=-PI/4 && theta<=0) || (theta<Pi/4 && theta>=0)
  	orientationFlag = POSX;
	if(theta>=PI/4 && theta<3*Pi/4)
	  orientationFlag = POSY;
	if((theta>=3*PI/4 && theta<=PI) || (theta<-3*PI/4 && theta>=-PI) )
  	orientationFlag = NEGX;
	if(theta<-PI/4 && theta>=-3*PI/4 )
	  orientationFlag = POSX;
}

void updateStateParameters(void)

void startMotorsAndLocalizer(void) {
	localizer->restart(localizer);
	findOutState();
	motors->setSpeeds(motors,speeds.l,speeds.r);
}

void loopLocalizerAndFeedMotors(void) {
	localizer->update;
	calRequiredSpeeds();
	motors->setSpeeds(motors,speeds.l,speeds.r);
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
  switch(orientationFlag){
    case POSX:
      targState->x += dist; 
      break;
    case POSY:
      targState->y += dist; 
      break;
    case NEGX:
      targState->x -= dist; 
      break;
    case NEGY:
      targState->y -= dist; 
      break;
  }
}

void turnLeft90(void){
  motionComplete = 0; 
  isTurning = 1;
  switch(orientationFlag){
    case POSX:
      nextOrientationFlag = POSY;
      break;
    case POSY:
      nextOrientationFlag = NEGX;
      break;
    case NEGX:
      nextOrientationFlag = NEGY;
      break;
    case NEGY:
      nextOrientationFlag = POSX;
      break;
  }
}

void doMotion(void){
  float theta; 
  if(motionComplete)
    return;
  else if(isMotionComplete()){
    finishMotion(); 
    motionComplete = 1;
    return;
  }

  if(isTurning){
    motors->setSpeeds(motors, speeds->l, speeds->r);
  } else {
    theta = localizer->state->theta; 
    motors->setSpeeds(motors, cosf(theta)*speeds->l, cosf(theta)*speeds->r);
  }
}


