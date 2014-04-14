#ifndef _PID_H_
#define _PID_H_

#ifdef pid_t
#undef pid_t
#endif //pid_t
#include "motors.h"
#include "common.h"

typedef struct { 
  float x; 
  float y; 
  float theta; 
} *Point; 

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
