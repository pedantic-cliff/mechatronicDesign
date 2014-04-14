#include "pid.h"
#include "utils.h"
#include "usart.h"
#include "common.h"
#include "localize.h"
#include "math.h"
//#include "errno.h"

static Pid_t _storage; 

int __errno;

static pidError_t eT = {0.f, 0.f, 0.f, 1};
static pidError_t eB = {0.f, 0.f, 0.f, 1};
static pidError_t eDist = {0.f, 0.f, 0.f, 1};

void calcErr(const float err, Error E, float integThresh){
	if(E->first==1){								//Check if the error struct is virgin yet
		E->p = err;
		E->first = 0;
	}
	E->d = err - E->p;							// E.p still the old error. Set diff error 
	E->p = err;										// Set proprtional error
	if(fabsf(err) < integThresh)				// Check for size of error
		E->s += err;								// Set intergral error
}


void calcAllErrs(State targ,State curr){
	float xyDistErr,bearErr,angErr,xTrans,yTrans,theta;
	theta = curr->theta;
	
	xTrans =  (targ->x - curr->x)*cosf(theta) + (targ->y - curr->y)*sinf(theta);
	yTrans = -(targ->x - curr->x)*sinf(theta) + (targ->y - curr->y)*cosf(theta);
	
	xyDistErr = (xTrans * sqrt((xTrans*xTrans)+(yTrans*yTrans)))/fabsf(xTrans);
	bearErr = atan2f(yTrans,xTrans);
	angErr = targ->theta - curr->theta;
	angErr = fixAngle(angErr);
	
	calcErr(xyDistErr,&eDist,2.0f);
	calcErr(bearErr,&eB, PI/18.f);
	calcErr(angErr,&eT, PI/18.f);
}


static void loop (Pid self, State target, State current){
	float velocityFbk,omegaFbk,vleft,vright;
	calcAllErrs(target,current);
	
	velocityFbk =	eDist.p*self->xyDistGains.Kp 
              + eDist.s*self->xyDistGains.Ks
						  + eDist.d*self->xyDistGains.Kd;
	
	omegaFbk =  eB.p*self->bearGains.Kp 
						+ eB.s*self->bearGains.Ks 
						+ eB.d*self->bearGains.Kd 
						  
						+ eT.p*self->angGains.Kp 
						+ eT.s*self->angGains.Ks 
						+ eT.d*self->angGains.Kd;
	
	vleft = velocityFbk - omegaFbk*WHEEL_BASE_WIDTH/2;
	vright = velocityFbk + omegaFbk*WHEEL_BASE_WIDTH/2;
	
	//Feed forward part yet to be done
	
	self->m->setMotorTargSpeeds(self->m,vleft,vright);
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

void resetPIDError(void){
	eT.p = 0.f;eT.s = 0.f;eT.d = 0.f;eT.first = 1.f;
	eB.p = 0.f;eB.s = 0.f;eB.d = 0.f;eB.first = 1.f;
	eDist.p = 0.f;eDist.s = 0.f;eDist.d = 0.f;eDist.first = 1.f;

}

Pid createPID(PID_Gains distG, PID_Gains bearG, PID_Gains angG, Motors m){
	Pid pid = & _storage;

	setGains(pid,distG,bearG,angG);
	pid->m = m;

	pid->loop = loop;
	pid->setGains = setGains;

	return pid;
}

