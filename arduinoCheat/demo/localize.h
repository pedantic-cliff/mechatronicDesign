#ifndef _LOCALIZE_H_
#define _LOCALIZE_H_

#include "common.h"

sensorPos findSensorLocations(float x, float y, float th);

void incrementRobotDist(void);

void incrementRobotAngle(void);

void decrementRobotDist(void);

void decrementRobotAngle(void);

extern float	robotGlobalX,
robotGlobalY,
robotGlobalTh,
eachStepDeltaDist,		//Corresponds to 0.5 inches
eachStepDeltaAngle; 	//Corresponds to 10 deg

#endif //_LOCALIZE_H_

