#include "pid.h"
#include "utils.h"
#include "usart.h"
#include "common.h"
#include "localize.h"
#include "math.h"
//#include "errno.h"

static Pid_t _storage; 


typedef struct { 
	float p;
	float s; 
	float d;
	char first;
} pidError_t, *Error; 

int __errno;

static pidError_t eT = {0.f, 0.f, 0.f, 1};
static pidError_t eB = {0.f, 0.f, 0.f, 1};
static pidError_t eDist = {0.f, 0.f, 0.f, 1};
static pidError_t eLm = {0.f, 0.f, 0.f, 1};
static pidError_t eRm = {0.f, 0.f, 0.f, 1};

float fixAngles(float inAngle){
	float outAngle;								//Calc and convert angular error to (-pi,pi]
	outAngle = atan2f(sin(inAngle),cos(inAngle));
	return outAngle;
}


void calcErr(const float err, Error E){
	if(E->first==1){								//Check if the error struct is virgin yet
		E->p = err;
		E->first = 0;
	}
	E->d = err - E->p;							// e.p still the old error. Set diff error 
	E->p = err;										// Set proprtional error
	if(fabsf(err) < PI/6)						// Check for size of error
		E->s = E->s + err;						// Set intergral error
}


void calcAllErrs(State targ,State curr){
	float xyDistErr,bearErr,angErr;
	xyDistErr = sqrt(	(targ->x - curr->x)*(targ->x - curr->x) + 
								(targ->y - curr->y)*(targ->y - curr->y));
	bearErr = atan2f(targ->y - curr->y,targ->x - curr->x);
	angErr = targ->theta - curr->theta;
	angErr = fixAngles(angErr);
	
	calcErr(xyDistErr,&eDist);
	calcErr(bearErr,&eB);
	calcErr(angErr,&eT);
}


void loop (Pid self, State target, State current){
	float velocityFbk,omegaFbk,vleft,vright;
	calcAllErrs(target,current);
	
	velocityFbk = 	eDist.p*self->xyDistGains.Kp +
						eDist.s*self->xyDistGains.Ks +
						eDist.d*self->xyDistGains.Kd;
	
	omegaFbk = 		eB.p*self->bearGains.Kp +
						eB.s*self->bearGains.Ks +
						eB.d*self->bearGains.Kd +
						
						eT.p*self->angGains.Kp +
						eT.s*self->angGains.Ks +
						eT.d*self->angGains.Kd;
	
	vleft = velocityFbk - omegaFbk*WHEEL_BASE_WIDTH/2;
	vright = velocityFbk + omegaFbk*WHEEL_BASE_WIDTH/2;
	
	//Feed forward part yet to be done
	
	self->m->setSpeeds(self->m,vleft,vright);
}

void setGains(Pid pid, PID_Gains distG, PID_Gains bearG, PID_Gains angG){
	pid->xyDistGains.Kp = distG.Kp;
	pid->xyDistGains.Ks = distG.Ks;
	pid->xyDistGains.Kd = distG.Kd;
	pid->bearGains.Kp = bearG.Kp;
	pid->bearGains.Ks = bearG.Ks;
	pid->bearGains.Kd = bearG.Kd;
	pid->angGains.Kp = angG.Kp;
	pid->angGains.Ks = angG.Ks;
	pid->angGains.Kd = angG.Kd;
}

Pid createPID(Pid pid ,PID_Gains distG, PID_Gains bearG, PID_Gains angG, Motors m){
	Pid pid = & _storage;

	setGains(distG,bearG,angG);
	pid->m = m;

	pid->loop = loop;
	pid->setGains = setGains;

	return pid;
}

