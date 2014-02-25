#include "main.h"
#include "pid.h"
#include "usart.h"
#include "accel.h"
#include "servo.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include <stdio.h>
/* leds in the board will fade */
typedef enum { CYCLE, ACCEL, SERVO } STATE; 
volatile STATE state; 

Servo_t servos[0]; 

Servo leftServo; 
Servo rightServo; 

int main(void) {
  init();
  pid_pos(500); 
  do {
//    pingServo(0x01);
    loop();
  } while (1);
}

void init() {
  state = CYCLE; 
  initButton();
  initLEDs();
  initPID(1.f, 0.f, 0.f);
  init_USART(); 
  initAccel();
  initSysTick(); 
  initServos(); 
  leftServo  = createServo(&servos[0], SERVO_ID_LEFT,  DIRECTION_FORWARD); 
  rightServo = createServo(&servos[1], SERVO_ID_RIGHT, DIRECTION_REVERSE);
}


void loop() {
  switch(state){
    case ACCEL: 
      doAccel();
      break;
    case SERVO: 
      break;
    case CYCLE:
      break;
  }
  delay(500);
}

void doAccel(void){
  int8_t x, y; 
  GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |  GPIO_Pin_15); 
  x = (int8_t)accel_getX(); 
  y = (int8_t)accel_getY(); 
  if (x > 30)
    GPIO_SetBits(GPIOD, GPIO_Pin_14); 
  if (y > 30) 
    GPIO_SetBits(GPIOD, GPIO_Pin_13); 
  if (y < -30) 
    GPIO_SetBits(GPIOD, GPIO_Pin_15); 
  if (x < -30) 
    GPIO_SetBits(GPIOD, GPIO_Pin_12); 
}

void delay(uint32_t ms) {
  ms *= 3360;
  while(ms--) {
    __NOP();
  }
}

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
  GPIO_Init(GPIOD, &gpio);

}

// Interrupt Channel 0 handler
void EXTI0_IRQHandler(void){
  if(EXTI_GetITStatus(EXTI_Line0) != RESET){
    switch(state){
      case ACCEL: 
        state = CYCLE; 
        break;
      case CYCLE:
        state = SERVO; 
        break; 
      case SERVO:
        state = ACCEL; 
        break; 
    }
  }
  EXTI_ClearITPendingBit(EXTI_Line0); 
}


typedef enum { ORANGE, GREEN, BLUE, RED } LED_STATE;
LED_STATE activeLED; 
long currentTime; 
void initSysTick(void){
  activeLED = ORANGE; 
  currentTime = 0; 
  SysTick_Config(SystemCoreClock / 1000);
  NVIC_SetPriority(SysTick_IRQn, 1); 
}

void SysTick_Handler(void){
  if(state != CYCLE) return; 

  currentTime++;
  if(currentTime % 200 != 0) return;
  
  GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15); 
  switch(activeLED){
    case ORANGE: 
      activeLED = GREEN; 
      GPIO_SetBits(GPIOD, GPIO_Pin_12);
      break;
    case GREEN: 
      activeLED = BLUE; 
      GPIO_SetBits(GPIOD, GPIO_Pin_15);
      break; 
    case BLUE: 
      activeLED = RED; 
      GPIO_SetBits(GPIOD, GPIO_Pin_14);
      break;
    case RED: 
      activeLED = ORANGE; 
      GPIO_SetBits(GPIOD, GPIO_Pin_13);
      break;
  }
}

void initLEDs() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  GPIO_InitTypeDef gpio;
  GPIO_StructInit(&gpio);
  gpio.GPIO_Mode = GPIO_Mode_OUT;
  gpio.GPIO_Pin  = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOD, &gpio);
}

