#ifndef _MAIN_H_
#define _MAIN_H_
#include "utils.h"
#include "usart.h"
#include "accel.h"
#include "colorSensor.h"
#include "localize.h"
#include "motors.h"
#include "pid.h"
#include "map.h"

void halt(void);
void start(void);
void tick_loop(void);
void setCalibrateColor(void);

extern volatile int running;
extern ColorSensors colorSensors; 
extern Accel accel; 
extern Motors motors; 

extern Localizer localizer;

extern State targState;
extern int calibrateColor; 

extern Pid pid; 
extern PID_Gains angleGains,
                 distGains ,
                 motorGains, 
                 bearGains ;


#endif //_MAIN_H_
