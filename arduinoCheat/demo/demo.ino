#include "common.h"
#include "localize.h"
#include "map.h"
#include "colorSensor.h"

#define rectangleLength 36
#define rectangleBreath 36

static int loopCounter = 0;


int numHorizontalSteps,numVerticalSteps,numRightAngleSteps;

void setup(){
  Serial.begin(9600);

  createGrid();

  pinMode(13,OUTPUT);

  numHorizontalSteps 	= rectangleLength/eachStepDeltaDist;
  numVerticalSteps     = rectangleBreath/eachStepDeltaDist;
  numRightAngleSteps 	= PI/2/eachStepDeltaAngle;
}

void loop(){
  confidences_t conf; 
  sensorPos poses; 
  // Get and send colours
  conf = doColor(); 
  poses = findSensorLocations(robotGlobalX,robotGlobalY,robotGlobalTh);
  sendGuesses();
  
  Serial.print(robotGlobalX);
  Serial.print("\t");
  Serial.print(robotGlobalY);
  Serial.print("\t");
  Serial.println(robotGlobalTh);
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
  digitalWrite(13,HIGH);
  delay(10); 
  digitalWrite(13,LOW);
  delay(10);
}

