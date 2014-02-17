#include "pid.h"

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"

/* leds in the board will fade */
const uint16_t LEDS = GPIO_Pin_13 | GPIO_Pin_15;
static float  Kp = 0.f,
              Ki = 0.f, 
              Kd = 0.f;


void initPins(void); 
void initTimer(void); 
void initPWM(void); 

void initPID(float p, float i, float d){
  Kp = p, Ki = i, Kd = d;
  initPins(); 
  initTimer(); 
  initPWM();
}

void pid_pos(long pos){
  TIM_SetCompare1(TIM1, 300); // set brightness
}

void initPins(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);
}

void initTimer(void) {
  /* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM1, ENABLE);

  /* Compute the prescaler value */
  u32 PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 21000000) - 1;

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 1000;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
}

void initPWM(void) {
  TIM_OCInitTypeDef TIM_OCInitStructure;

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  /* PWM1 Mode configuration: Channel2 (GPIOD Pin 13)*/
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

  TIM_Cmd(TIM1, ENABLE);
}

