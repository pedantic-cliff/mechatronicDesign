#include "main.h"
#include "pid.h"

/* leds in the board will fade */
typedef enum { PID, ACCEL } STATE; 
volatile STATE state; 

int main(void) {
  init();
  pid_pos(500); 
  do {
    //    loop();
  } while (1);
}

void init() {
  state = PID; 
  initButton();
  initLEDs();
  initPID(1.f, 0.f, 0.f);
}

int lastButtonStatus = RESET;
void loop() {

  delay(4);
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
      case PID: 
        state = ACCEL;
        GPIO_SetBits(GPIOD,GPIO_Pin_12); 
        GPIO_ResetBits(GPIOD,GPIO_Pin_14); 
        break;
      case ACCEL: 
        state = PID; 
        GPIO_SetBits(GPIOD,GPIO_Pin_14); 
        GPIO_ResetBits(GPIOD,GPIO_Pin_12); 
        break;
    }
  }
  EXTI_ClearITPendingBit(EXTI_Line0); 
}

void initLEDs() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  GPIO_InitTypeDef gpio;
  GPIO_StructInit(&gpio);
  gpio.GPIO_Mode = GPIO_Mode_OUT;
  gpio.GPIO_Pin  = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOD, &gpio);
}

