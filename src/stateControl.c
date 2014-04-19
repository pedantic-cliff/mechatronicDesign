#include <stdio.h>
#include "motors.h"
#include "pid.h"
#include "map.h"
#include "localize.h"
#include "common.h"
#include "utils.h"

MotorSpeeds speeds;
Localizer localizer;

static enum { POSX=0, POSY=1, NEGX=2, NEGY=3 } orientationFlag;

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
												
thresholds_t thresholdSettings[] = 	{
													{	 0,		0},		//RIGHT +X
													{3000,	3000},		//UP	  +Y
													{	 0,		0},		//LEFT  -X
													{-4000, -4000}			//DOWN  -Y
												};
												
thresholds_t speedSettings[] = 		{
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

void goRightBy24(void){
	motors->set
}

void goRightBy12(void){

}

void goUpBy24(void){

}

void goLeftBy24(void){

}

void goDownBy12(void){

}

void setTargetForTrajectory(state_t target,bool ifTurn)
{
	if(ifTurn)
	{
		//Use the sine cosine thing
	}
	else
	{
		//Do the normal thing
	}
}

void getTurnControls(float theta) {
	
}
