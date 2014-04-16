#include "common.h"
#include "localize.h"
#include "map.h"
#include "colorSensor.h"

#define rectangleLength 36
#define rectangleBreath 36

void start(){
  Serial.begin(9600);

  createGrid();
  static int loopCounter = 0;
  int numHorizontalSteps,numVerticalSteps,numRightAngleSteps;
  
  numHorizontalSteps 	= rectangleLength/eachStepDeltaDist;
  numVerticalSteps 		= rectangleBreath/eachStepDeltaDist;
  numRightAngleSteps 	= PI/2/eachStepDeltaAngle;
}

void loop(){
	pConfidences conf; 
	sensorPos poses; 
						// Get and send colours
	conf = doColor(); 
	poses = findSensorLocations(robotGlobalX,robotGlobalY,robotGlobalTh);
	sendGuesses();
						// Move the ass conditionally
	if(loopCounter<numHorizontalSteps)
	incrementRobotDist();
	else if(loopCounter<numHorizontalSteps+numRightAngleSteps)
	incrementRobotAngle();
	else if(loopCounter<numHorizontalSteps+numRightAngleSteps+numVerticalSteps)
	incrementRobotDist();
	else if(loopCounter<numHorizontalSteps+2*numRightAngleSteps+numVerticalSteps)
	incrementRobotAngle();
	else if(loopCounter<2*numHorizontalSteps+2*numRightAngleSteps+numVerticalSteps)
	incrementRobotDist();
	else if(loopCounter<2*numHorizontalSteps+3*numRightAngleSteps+numVerticalSteps)
	incrementRobotAngle();
	else if(loopCounter<2*numHorizontalSteps+3*numRightAngleSteps+2*numVerticalSteps)
	incrementRobotDist();
	else if(loopCounter<2*numHorizontalSteps+4*numRightAngleSteps+2*numVerticalSteps)
	incrementRobotAngle();
	
						// Take breath and keep track	
	loopCounter ++;
	delay(2000); 
}
