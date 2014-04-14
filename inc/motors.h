#ifndef _MOTORS_H_
#define _MOTORS_H_
#include "common.h"
typedef struct motors *Motors; 

typedef struct motors{
  int PWM_Min_L;
  int PWM_Min_R;
  
  float leftTargetSpeed;
  float rightTargetSpeed;
  
  float p,s,d;
  
  int (*getLeftCount)(void); 
  int (*getRightCount)(void); 
  void (*resetCounts)(void); 

  void (*setSpeeds)(Motors self, float left, float right); 
  void (*setOffset)(Motors self, int offset_L, int offset_R); 
  void (*setMotorPIDGains)(Motors self, PID_Gains gains);
  void (*doMotorPID)(Motors self);
  void (*setMotorTargSpeeds)(Motors self, float leftTargSpeed, float rightTargSpeed);
  
} motors_t; 

Motors createMotors(void); 

#endif //_MOTORS_H_
