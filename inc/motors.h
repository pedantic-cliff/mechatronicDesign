#ifndef _MOTORS_H_
#define _MOTORS_H_
#include "common.h"

typedef struct motors *Motors; 

#define PWM_MIN_L     4000
#define PWM_MIN_R     5000
#define PWM_PERIOD 	0x8000
#define PWM_MAX 		0x4000
#define PWM_SCALER 	1

typedef struct motors{
  float leftTargetSpeed;
  float rightTargetSpeed;
  long PWM_Min_L;
  long PWM_Min_R;
  long PWM_Base_L;
  long PWM_Base_R; 
  
  float p,s,d;
  
  int (*getLeftCount)(void); 
  int (*getRightCount)(void); 
  void (*resetCounts)(void); 

  void (*setSpeeds)(Motors self, float left, float right); 
  void (*setOffset)(Motors self, int base_L, int base_R); 
  void (*haltMotors)(Motors self);
  void (*updateOffset)(Motors self, float theta); 
  void (*setMotorPIDGains)(Motors self, PID_Gains gains);
  void (*doMotorPID)(Motors self);
  void (*setMotorTargSpeeds)(Motors self, float leftTargSpeed, float rightTargSpeed);
  void (*resetMotorPIDErrors)(void);
  void (*resetMotorPID)(Motors self);
} motors_t; 

Motors createMotors(void); 

#endif //_MOTORS_H_
