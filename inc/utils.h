#ifndef _UTILS_H_
#define _UTILS_H_
#include "stm32f4xx.h"

//---- COMMON DEFINES ----//

//---- COMMON TYPES ----//
// Define Colors for all modules
typedef enum { 
  NONE   = 0x01, 
  RED    = 0x02, 
  GREEN  = 0x04, 
  BLUE   = 0x08,
  ORANGE = 0x10
} Color; 

//---- BUTTONS ----//
void initButton(void);

//---- LEDS ----//
void initLEDs(void); 

// Can be bitmasked RED | BLUE | GREEN | ORANGE
void enableLEDs(Color c);
void disableLEDs(Color c); 

//---- TIMING ----//
void initSysTick(void);
long getCurrentTime(void); 
void delay(uint32_t ms); 

#endif //_UTILS_H_


