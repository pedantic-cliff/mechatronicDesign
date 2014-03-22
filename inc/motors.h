#ifndef _MOTORS_H_
#define _MOTORS_H_

typedef struct motors *Motors; 

typedef struct motors{
  
  int (*getLeftCount)(void); 
  int (*getRightCount)(void); 
  void (*resetCounts)(void); 

  void (*setSpeeds)(float left, float right); 
} motors_t; 

Motors createMotors(void); 

#endif //_MOTORS_H_
