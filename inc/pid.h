#ifndef _PID_H_
#define _PID_H_

#include "motors.h"
#include "eom.h"

typedef struct { 
  float x; 
  float y; 
  float a; 
} *Point; 

typedef struct {
  float Kp; 
  float Ki; 
  float Kd;
} PID_Gains;

typedef struct pid *Pid;
typedef struct pid {
  PID_Gains angleGains; 
  PID_Gains posGains;
  PID_Gains velGains;
  
  Motors m; 

  void (*holdPos)(Pid self, Point target);
  void (*holdVel)(Pid self, Point target); 
} pid_t;

Pid createPID(Motors m);

#endif // _PID_H_
