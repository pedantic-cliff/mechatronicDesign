#ifndef _COMMON_H_
#define _COMMON_H_

#define PI (3.1415926535897932384626f)

#define ENC_TO_D(X) ( X  * ( (PI * 4.14f) / 8400.f ))
#define WHEEL_BASE_WIDTH (8)

typedef struct state *State;
typedef struct state{
    float x; 
    float y; 
    float theta; 
    float vel;
} state_t;

#endif //_COMMON_H_
