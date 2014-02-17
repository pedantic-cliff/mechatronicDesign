#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#ifndef _MAIN_H_
#define _MAIN_H_


void init();
void loop();

void delay();

void initButton();
void initLEDs();
#endif //_MAIN_H_
