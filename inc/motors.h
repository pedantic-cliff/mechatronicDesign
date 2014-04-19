#ifndef _MOTORS_H_
#define _MOTORS_H_
#include "common.h"

typedef struct motors *Motors; 

typedef struct motors{
  float leftTargetSpeed;
  float rightTargetSpeed;
  long PWM_Min;
  
  float p,s,d;
  
  int (*getLeftCount)(void); 
  int (*getRightCount)(void); 
  void (*resetCounts)(void); 

  void (*setSpeeds)(Motors self, float left, float right); 
  void (*setOffset)(Motors self, float angle); 
  void (*setMotorPIDGains)(Motors self, PID_Gains gains);
  void (*doMotorPID)(Motors self);
  void (*setMotorTargSpeeds)(Motors self, float leftTargSpeed, float rightTargSpeed);
  void (*resetMotorPIDErrors)(void);
  void (*resetMotorPID)(Motors self);
} motors_t; 

Motors createMotors(void); 

#endif //_MOTORS_H_
