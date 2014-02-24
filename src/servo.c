
#include <stm32f4xx_usart.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include <stm32f4xx_gpio.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include <stm32f4xx_rcc.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src

#define BAUD_RATE 15000

#define BROADCAST_ID 0xFE

#define MAX_PARAMS 16

#define PING        0x01
#define READ_DATA   0x02
#define WRITE_DATA  0x03

#define ID_ADDR                0x03
#define MOVING_SPEED_LOW_ADDR  0x21

typedef struct __attribute__((__packed__)){
  char      StartBytes[2];
  char      Id;
  char      Length; 
  char      Instruction;
} inner_Instruction_t;

typedef struct {
  inner_Instruction_t  Header;
  char                Parameters[MAX_PARAMS];
  char                Checksum;
} Instruction_t;

typedef struct __attribute__((__packed__)){
  char      StartBytes[2]; 
  char      Id; 
  char      Length; 
  char      Error; 
} inner_Response_t;

typedef struct {
  inner_Response_t  Header; 
  char              Parameters[MAX_PARAMS];
  char              Checksum;
} Response_t;

Instruction_t instruction;
Response_t    response; 

void initServos(void){
  //_buff_ptr = &_buffer[0];
  /* --------------------------- System Clocks Configuration -----------------*/
  /* USART3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  
  /* GPIOD clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
 
  /*-------------------------- GPIO Configuration ----------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
 
  /* Connect USART pins to AF */
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);

  USART_InitTypeDef USART_InitStructure;
 
  /* USARTx configuration ------------------------------------------------------*/
  /* USARTx configured as follow:
        - BaudRate = 9600 baud
        - Word Length = 8 Bits
        - Two Stop Bit
        - Odd parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 3124950;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
 
  USART_Init(USART3, &USART_InitStructure);
  //USART3->CR3 |= USART_CR3_HDSEL;
  USART_Cmd(USART3, ENABLE);
}

void _fillChecksum(void){
  int i; 
  unsigned char checksum =  instruction.Header.Id 
                            + instruction.Header.Length 
                            + instruction.Header.Instruction; 
  for (i = 0; i < instruction.Header.Length; i++){
    checksum += instruction.Parameters[i]; 
  }
  checksum = ~checksum;
  instruction.Checksum = checksum; 
}

int _write(char * buffer, int length){
  int ii;
  for(ii = length; ii; buffer++, ii--){
    while( !(USART3->SR & 0x00000040) );
    USART_SendData(USART3, *buffer);
  }
  return length - ii; 
}

volatile char _buffer[256];
volatile _buff_index = 0;
/*
void USART3_IRQHandler(void){
  if (USART_GetITStatus(USART1, USART_IT_RXNE) ){
    _buffer[_buff_index] = USART3->DR; 
    _buff_index = (_buff_index + 1) & 0xFF;
  }
}

int _read(char * buffer, int length){
  int ii; 
  for (ii = length; ii; buffer++, ii--){
  }

  return 0;
}
*/
void _writeInstruction(){
  _write((char*)&instruction.Header, sizeof(instruction.Header)); 
  _write((char*)&instruction.Parameters, instruction.Header.Length - 2); 
  _write(&instruction.Checksum, 1); 
}

void _readResponse(){
}

int setID(void){
  instruction.Header.StartBytes[0]  = 0xFF; 
  instruction.Header.StartBytes[1]  = 0xFF; 
  instruction.Header.Id             = BROADCAST_ID; 
  instruction.Header.Length         = 4;
  instruction.Header.Instruction    = WRITE_DATA;
  instruction.Parameters[0]         = 0x03;
  instruction.Parameters[1]         = 0x01;
  _fillChecksum(); 

  _writeInstruction(); 
  return 0; 
}

int enableTorque(void){
  instruction.Header.StartBytes[0]  = 0xFF; 
  instruction.Header.StartBytes[1]  = 0xFF; 
  instruction.Header.Id             = 0x01; 
  instruction.Header.Length         = 4;
  instruction.Header.Instruction    = WRITE_DATA;
  instruction.Parameters[0]         = 0x18;
  instruction.Parameters[1]         = 0x01;
  _fillChecksum(); 

  _writeInstruction(); 
  return 0; 
}
int setSpeed(void){
  instruction.Header.StartBytes[0]  = 0xFF; 
  instruction.Header.StartBytes[1]  = 0xFF; 
  instruction.Header.Id             = 0x01; 
  instruction.Header.Length         = 4;
  instruction.Header.Instruction    = WRITE_DATA;
  instruction.Parameters[0]         = MOVING_SPEED_LOW_ADDR;
  instruction.Parameters[1]         = 0x10;
  _fillChecksum(); 

  _writeInstruction(); 
  _readResponse(); 
  return 0; 

}

int pingServo(int ID){
  instruction.Header.StartBytes[0] = 0xFF; 
  instruction.Header.StartBytes[1] = 0xFF; 
  instruction.Header.Id = ID; 
  instruction.Header.Length = 2; 
  instruction.Header.Instruction = PING; 
  _fillChecksum();

  _writeInstruction();
  return 0; 
}
