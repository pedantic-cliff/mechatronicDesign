#ifndef _ACCEL_H_
#define _ACCEL_H_
#include <stm32f4xx.h>
typedef struct accel *Accel;
typedef struct accel {
  void (*init)(void); 
  uint8_t (*getX)(void); 
  uint8_t (*getY)(void); 
  uint8_t (*getZ)(void); 
} accel_t; 

Accel initAccel(void); 

#endif //_ACCEL_H_
