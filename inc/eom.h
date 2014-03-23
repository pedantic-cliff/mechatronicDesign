#ifndef _EOM_H_
#define _EOM_H_

#define PI    = (3.14159265359f)
#define PI_2  = (1.57079632679f)
#define 2PI   = (6.28318530718f)

typedef struct { 
  float x; 
  float y; 
  float a; 
  float dx; 
  float dy; 
} state_t, *State; 

#endif //_EOM_H_
