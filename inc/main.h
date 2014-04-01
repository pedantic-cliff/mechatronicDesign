#ifndef _MAIN_H_
#define _MAIN_H_

void init();
void loop();


extern ColorSensors colorSensors; 
extern Accel accel; 
extern Motors motors; 

extern Localizer localizer;

extern State targState;

extern Pid pid; 
extern PID_Gains angleGains = { 1.f, 0.5f, 0.0f },
                 posGains   = { 0.0f, 0.0f, 0.0f },
                 velGains   = { 1.0f, 0.0f, 0.0f };


#endif //_MAIN_H_
