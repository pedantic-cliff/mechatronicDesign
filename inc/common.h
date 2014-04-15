#ifndef _COMMON_H_
#define _COMMON_H_

#define PI (3.1415926535897932384626f)

#define ENC_TO_D(X) ( (X)  * ( (PI * 4.14f) / 8400.f ))
#define WHEEL_BASE_WIDTH (7.5f)

#define fixAngle(theta) (atan2f(sinf(theta),cosf(theta)))

typedef struct state *State;
typedef struct state{ 
    float x; 
    float y; 
    float theta; 
    float vel;
} state_t;

typedef struct { 
	float p;
	float s; 
	float d;
	char first;
} pidError_t, *Error; 

typedef struct {
  float Kp; 
  float Ks; 
  float Kd;
} PID_Gains;

typedef struct {
  float metal; 
  float yellow; 
  float boundary; 
} confidences_t, pConfidences; 

void calcErr(const float err, Error E, float integThresh);

#endif //_COMMON_H_
