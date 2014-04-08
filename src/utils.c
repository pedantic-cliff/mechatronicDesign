#include "utils.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include "main.h"

/******* Init USER_BUTTON on the project board *********/
void initButton(void) {
  // Helper Typedefs

  // Connect EXTI to Button Pin 
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, GPIO_PinSource0); 

  //Configure EXTI (Interrupt MUX) 
  EXTI_InitTypeDef  EXTI_InitStructure; 

  EXTI_InitStructure.EXTI_Line      = EXTI_Line0;          // Line select
  EXTI_InitStructure.EXTI_Mode      = EXTI_Mode_Interrupt; // 
  EXTI_InitStructure.EXTI_Trigger   = EXTI_Trigger_Rising; // Trigger Mode
  EXTI_InitStructure.EXTI_LineCmd   = ENABLE;
  EXTI_Init(&EXTI_InitStructure); 

  //Configure NVIC 
  NVIC_InitTypeDef  NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;        //Channel Select
  // Set interrupt priority 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F; // Lowest
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;        // Lowest
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  GPIO_InitTypeDef gpio;
  GPIO_StructInit(&gpio);
  gpio.GPIO_Mode = GPIO_Mode_IN;
  gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOA, &gpio);
}

// Interrupt for USER_BUTTON
void EXTI0_IRQHandler(void){
  EXTI_ClearITPendingBit(EXTI_Line0); 
}

/************ Surface LEDs *****************/
#define GREEN_PIN   GPIO_Pin_12
#define RED_PIN     GPIO_Pin_14
#define BLUE_PIN    GPIO_Pin_15
#define ORANGE_PIN  GPIO_Pin_13
#define LED_PORT    GPIOD

void initLEDs() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  GPIO_InitTypeDef gpio;
  GPIO_StructInit(&gpio);
  gpio.GPIO_Mode = GPIO_Mode_OUT;
  gpio.GPIO_Pin  = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOD, &gpio);
}

void enableLEDs(Color c){ 
  int mask = 0; 
  if(c & RED)
    mask |= RED_PIN;
  if(c & GREEN)
    mask |= GREEN_PIN;
  if(c & ORANGE)
    mask |= ORANGE_PIN;
  if(c & BLUE)
    mask |= BLUE_PIN;
  GPIO_SetBits(LED_PORT, mask); 
}
void disableLEDs(Color c){ 
  int mask = 0; 
  if(c & RED)
    mask |= RED_PIN;
  if(c & GREEN)
    mask |= GREEN_PIN;
  if(c & ORANGE)
    mask |= ORANGE_PIN;
  if(c & BLUE)
    mask |= BLUE_PIN;
  GPIO_ResetBits(LED_PORT, mask); 
}

/************ Timing ********************/
static long currentTime; 
void initSysTick(void){
  currentTime = 0; 
  SysTick_Config(SystemCoreClock / 1000);
  NVIC_SetPriority(SysTick_IRQn, 1); 
}

long getCurrentTime(void){ 
  return currentTime; 
}

void SysTick_Handler(void){
  currentTime++; 
  enableLEDs(RED);
  if(running)
    tick_loop();
  disableLEDs(RED);
}

void delay(uint32_t ms) {
  ms *= 9270;
  while(ms--) {
    __NOP();
  }
}


