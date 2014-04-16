#ifndef _COMMON_H_
#define _COMMON_H_
#define PI (3.14159)

typedef struct {
  float metal; 
  float yellow; 
  float boundary;
  float white;
} confidences_t, *pConfidences; 

typedef struct { 
	int row;
	int col;	
} sensorCoords, *sensorCoordPtr;

typedef struct {
	sensorCoords s[6];
} sensorPos, *sensorPosPtr;

#endif //_COMMON_H_
