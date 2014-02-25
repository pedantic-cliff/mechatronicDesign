#include <servo.h>
#include <stm32f4xx_usart.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include <stm32f4xx_gpio.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include <stm32f4xx_rcc.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src

#define BAUD_RATE 3124950   // Somehow this is 1Mbps
#define PING        0x01
#define READ_DATA   0x02
#define WRITE_DATA  0x03

#define ID_ADDR                0x03
#define TORQUE_ADDR            0x18
#define MOVING_SPEED_LOW_ADDR  0x21

void writeInstruction(Servo s, char *params, int len); 

int setTorque(Servo s, char en){
  char params[2] = { 
    TORQUE_ADDR,
    STATE_ENABLE 
  }; 
  writeInstruction(s, params, 2); 
  return 0;
}

Servo createServo(Servo s, char ID, char direction){
  s->id = ID; 
  s->direction = direction; 

  s->setTorque = setTorque; 
  return s;
}


void initServos(void){
  /* USART3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  
  /* GPIOD clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  // Set up GPIO B10 //
  GPIO_InitTypeDef GPIO_InitStructure;
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
 
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);

  // Set up USART3 //
  USART_InitTypeDef USART_InitStructure;

  USART_InitStructure.USART_BaudRate = BAUD_RATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
 
  USART_Init(USART3, &USART_InitStructure);
  //USART3->CR3 |= USART_CR3_HDSEL;
  USART_Cmd(USART3, ENABLE);
}

void _write(char * buffer, int length){
  for(; length; buffer++, length--){
    while( !(USART3->SR & 0x00000040) );
    USART_SendData(USART3, *buffer);
  }
}

void writeInstruction(Servo s, char *params, int length){ 
  int i = 0; 
  char checksum = 0; 
  char buffer[5 + length]; 
  buffer[0] = buffer[1] = 0xFF;
  buffer[2] = s->id; 
  buffer[3] = length; 
  buffer[4] = WRITE_DATA; 
  for(i = 0; i < length; i++){
    buffer[4 + i] = params[i]; 
  }
  for(i = 2; i < 4 + length; i++){
    checksum += buffer[i]; 
  }
  buffer[4+length] = ~checksum; 

  _write(buffer, sizeof(buffer)); 
}
