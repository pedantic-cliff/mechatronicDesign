#ifndef _MOTORS_H_
#define _MOTORS_H_

typedef struct motors *Motors; 

typedef struct motors{
  int PWM_Min;
  
  int (*getLeftCount)(void); 
  int (*getRightCount)(void); 
  void (*resetCounts)(void); 

  void (*setSpeeds)(Motors self, float left, float right); 
  void (*setOffset)(Motors self, int offset); 
} motors_t; 

Motors createMotors(void); 

#endif //_MOTORS_H_
