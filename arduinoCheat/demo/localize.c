#include "localize.h"
#include "math.h"
#include "usart.h"

#define s1x (-4)
#define s1y (-4)

#define s2x (-4)
#define s2y ( 0)

#define s3x (-4)
#define s3y ( 4)

#define s4x ( 4)
#define s4y (-4)

#define s5x ( 4)
#define s5y ( 0)

#define s6x ( 4)
#define s6y ( 4)

#define CLEAN_ANGLE(X) ( atan2f(cos(Y

sensorPos findSensorLocations(float x, float y, float th){
	float xRobot,yRobot,tRobot;
	
	sensorPos senPositions;
	
	xRobot = x;
	yRobot = y;
	tRobot = th;
	
	senPositions.s[0].row = (xRobot + s1x*cosf(tRobot) - s1y*sinf(tRobot))/4;
	senPositions.s[0].col = (yRobot + s1x*sinf(tRobot) + s1y*cosf(tRobot))/4;

	senPositions.s[1].row = (xRobot + s2x*cosf(tRobot) - s2y*sinf(tRobot))/4;
	senPositions.s[1].col = (yRobot + s2x*sinf(tRobot) + s2y*cosf(tRobot))/4;

	senPositions.s[2].row = (xRobot + s3x*cosf(tRobot) - s3y*sinf(tRobot))/4;
	senPositions.s[2].col = (yRobot + s3y*sinf(tRobot) + s3y*cosf(tRobot))/4;

	senPositions.s[3].row = (xRobot + s4x*cosf(tRobot) - s4y*sinf(tRobot))/4;
	senPositions.s[3].col = (yRobot + s4x*sinf(tRobot) + s4y*cosf(tRobot))/4;

	senPositions.s[4].row = (xRobot + s5x*cosf(tRobot) - s5y*sinf(tRobot))/4;
	senPositions.s[4].col = (yRobot + s5x*sinf(tRobot) + s5y*cosf(tRobot))/4;
	
	senPositions.s[5].row = (xRobot + s6x*cosf(tRobot) - s6y*sinf(tRobot))/4;
	senPositions.s[5].col = (yRobot + s6x*sinf(tRobot) + s6y*cosf(tRobot))/4;
}

