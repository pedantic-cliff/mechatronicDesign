#include "main.h"
#include "pid.h"

/* leds in the board will fade */

const uint16_t USER_BUTTON = GPIO_Pin_0;
int main(void) {
  init();
  pid_pos(500); 
  do {
//    loop();
  } while (1);
}

void init() {
  initButton();
  initPID(1.f, 0.f, 0.f);
}

int lastButtonStatus = RESET;
void loop() {

  uint8_t currentButtonStatus = GPIO_ReadInputDataBit(GPIOA, USER_BUTTON);
  if (lastButtonStatus != currentButtonStatus && currentButtonStatus != RESET) {
  }
  lastButtonStatus = currentButtonStatus;

  delay(4);
}

void delay(uint32_t ms) {
  ms *= 3360;
  while(ms--) {
    __NOP();
  }
}

void initButton(void) {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  GPIO_InitTypeDef gpio;
  GPIO_StructInit(&gpio);
  gpio.GPIO_Mode = GPIO_Mode_IN;
  gpio.GPIO_Pin = USER_BUTTON;
  GPIO_Init(GPIOA, &gpio);
}

