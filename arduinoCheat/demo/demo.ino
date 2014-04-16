#include "common.h"
#include "localize.h"
#include "map.h"
#include "colorSensor.h"

void start(){
  Serial.begin(9600);

  createGrid();
}
void loop(){
  pConfidences conf; 
  sensorPos poses; 

  conf = doColor(); 
  poses = findSensorLocations(x,y,th);
  

  sendGuesses(); 
  delay(2000); 
}
