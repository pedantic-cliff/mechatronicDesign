#ifndef _PID_H_
#define _PID_H_

#include "motors.h"
#include "common.h"

typedef struct { 
  float x; 
  float y; 
  float theta; 
} *Point; 

typedef struct {
  float Kp; 
  float Ks; 
  float Kd;
} PID_Gains;

#ifdef pid_t
#undef pid_t
#endif 
typedef struct pid *Pid;
typedef struct pid {
  PID_Gains xyDistGains; 
  PID_Gains bearGains;
  PID_Gains angGains;
  
  Motors m; 

  void (*setGains)(Pid self, PID_Gains a, PID_Gains p, PID_Gains v);
  void (*loop)(Pid self, State target, State current);
} Pid_t;

Pid createPID(PID_Gains a, PID_Gains p, PID_Gains v, Motors m);

#endif // _PID_H_
