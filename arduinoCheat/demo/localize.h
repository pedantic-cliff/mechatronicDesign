#ifndef _LOCALIZE_H_
#define _LOCALIZE_H_

#include "common.h"

sensorPos findSensorLocations(float x, float y, float th);

void incrementRobotDist(void);

void incrementRobotAngle(void);

void decrementRobotDist(void);

void decrementRobotAngle(void);

#endif //_LOCALIZE_H_
