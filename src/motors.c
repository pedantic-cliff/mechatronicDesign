#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "misc.h"
#include "motors.h"

// Left Motor Channels 
#define ENCLA_PIN GPIO_Pin_6
#define ENCLA_GPIO_PORT GPIOC
#define ENCLA_GPIO_CLK RCC_AHB1Periph_GPIOC
#define ENCLA_SOURCE GPIO_PinSource6
#define ENCLA_AF GPIO_AF_TIM3 

#define ENCLB_PIN GPIO_Pin_7
#define ENCLB_GPIO_PORT GPIOC
#define ENCLB_GPIO_CLK RCC_AHB1Periph_GPIOC
#define ENCLB_SOURCE GPIO_PinSource7
#define ENCLB_AF GPIO_AF_TIM3

#define ENCL_TIMER TIM3 
#define ENCL_TIMER_CLK RCC_APB1Periph_TIM3

// Right Motor Channels 
#define ENCRA_PIN GPIO_Pin_15
#define ENCRA_GPIO_PORT GPIOA 
#define ENCRA_GPIO_CLK RCC_AHB1Periph_GPIOA
#define ENCRA_SOURCE GPIO_PinSource15
#define ENCRA_AF GPIO_AF_TIM2

#define ENCRB_PIN GPIO_Pin_3
#define ENCRB_GPIO_PORT GPIOB
#define ENCRB_GPIO_CLK RCC_AHB1Periph_GPIOB
#define ENCRB_SOURCE GPIO_PinSource3
#define ENCRB_AF GPIO_AF_TIM2

#define ENCR_TIMER TIM2
#define ENCR_TIMER_CLK RCC_APB1Periph_TIM2

// Helpers
#define LEFT_COUNT() ENCL_TIMER->CNT 
#define RIGHT_COUNT() ENCR_TIMER->CNT


void initEncoders (void) { GPIO_InitTypeDef GPIO_InitStructure;

  // turn on the clocks for each of the ports needed 
  RCC_AHB1PeriphClockCmd (ENCLA_GPIO_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd (ENCLB_GPIO_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd (ENCRA_GPIO_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd (ENCRB_GPIO_CLK, ENABLE);

  // now configure the pins themselves 
  // they are all going to be inputs with pullups 
  GPIO_StructInit (&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = ENCLA_PIN;
  GPIO_Init (ENCLA_GPIO_PORT, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = ENCLB_PIN;
  GPIO_Init (ENCLB_GPIO_PORT, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = ENCRA_PIN;
  GPIO_Init (ENCRA_GPIO_PORT, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = ENCRB_PIN;
  GPIO_Init (ENCRB_GPIO_PORT, &GPIO_InitStructure);

  // Connect the pins to their Alternate Functions 
  GPIO_PinAFConfig (ENCLA_GPIO_PORT, ENCLA_SOURCE, ENCLA_AF);
  GPIO_PinAFConfig (ENCLB_GPIO_PORT, ENCLB_SOURCE, ENCLB_AF);
  GPIO_PinAFConfig (ENCRA_GPIO_PORT, ENCRA_SOURCE, ENCRA_AF);
  GPIO_PinAFConfig (ENCRB_GPIO_PORT, ENCRB_SOURCE, ENCRB_AF);

  // Timer peripheral clock enable 
  RCC_APB1PeriphClockCmd (ENCL_TIMER_CLK, ENABLE);
  RCC_APB1PeriphClockCmd (ENCR_TIMER_CLK, ENABLE);

  // set them up as encoder inputs 
  // set both inputs to rising polarity to let it use both edges 
  TIM_EncoderInterfaceConfig (ENCL_TIMER, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_SetAutoreload (ENCL_TIMER, 0xffff);
  TIM_EncoderInterfaceConfig (ENCR_TIMER, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_SetAutoreload (ENCR_TIMER, 0xffff);

  // turn on the timer/counters 
  TIM_Cmd (ENCL_TIMER, ENABLE);
  TIM_Cmd (ENCR_TIMER, ENABLE);
  encodersReset();
}

void encodersReset(void){
  __disable_irq();
/*  oldLeftEncoder = 0;
  oldRightEncoder = 0;
  leftTotal = 0;
  rightTotal = 0;
  fwdTotal = 0;
  rotTotal = 0; */
  TIM_SetCounter (ENCL_TIMER, 0);
  TIM_SetCounter (ENCR_TIMER, 0);
//  encodersRead();
  __enable_irq();
}

int getLeftCount(void){
  return LEFT_COUNT();
}
int getRightCount(void){
  return RIGHT_COUNT();
}

