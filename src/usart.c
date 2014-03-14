#include <stm32f4xx.h>
#include <misc.h>			 // I recommend you have a look at these in the ST firmware folder
#include <stm32f4xx_usart.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include <stm32f4xx_gpio.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include <stm32f4xx_rcc.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include <stm32f4xx_conf.h> 

#define MAX_STRLEN 64 // this is the maximum string length of our string in characters
#define BAUD_RATE 15000     // Somehow this corresponds to 4800!!

volatile char received_string[MAX_STRLEN+1]; // this will hold the recieved string

/* This funcion initializes the USART1 peripheral
 *
 * Arguments: baudrate --> the baudrate at which the USART is
 * 						   supposed to operate
 */
void init_USART(void){
  uint32_t baudrate = BAUD_RATE;
  /* This is a concept that has to do with the libraries provided by ST
   * to make development easier the have made up something similar to
   * classes, called TypeDefs, which actually just define the common
   * parameters that every peripheral needs to work correctly
   *
   * They make our life easier because we don't have to mess around with
   * the low level stuff of setting bits in the correct registers
   */
  GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as TX and RX
  USART_InitTypeDef USART_InitStruct; // this is for the USART1 initilization
  NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)

  /* enable APB2 peripheral clock for USART1
   * note that only USART1 and USART6 are connected to APB2
   * the other USARTs are connected to APB1
   */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  /* enable the peripheral clock for the pins used by
   * USART1, PB6 for TX and PB7 for RX
   */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  /* This sequence sets up the TX and RX pins
   * so they work correctly with the USART1 peripheral
   */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // Pins 6 (TX) and 7 (RX) are used
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; 			// the pins are configured as alternate function so the USART peripheral has access to them
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// this defines the IO speed and has nothing to do with the baudrate!
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			// this defines the output type as push pull mode (as opposed to open drain)
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// this activates the pullup resistors on the IO pins
  GPIO_Init(GPIOB, &GPIO_InitStruct);					// now all the values are passed to the GPIO_Init() function which sets the GPIO registers

  /* The RX and TX pins are now connected to their AF
   * so that the USART1 can take over control of the
   * pins
   */
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

  /* Now the USART_InitStruct is used to define the
   * properties of USART1
   */
  USART_InitStruct.USART_BaudRate = baudrate;				// the baudrate is set to the value we passed into this init function
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
  USART_InitStruct.USART_StopBits = USART_StopBits_1;		// we want 1 stop bit (standard)
  USART_InitStruct.USART_Parity = USART_Parity_No;		// we don't want a parity bit (standard)
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
  USART_Init(USART1, &USART_InitStruct);					// again all the properties are passed to the USART_Init function which takes care of all the bit setting

  USART_SendData(USART2, 'U');

  /* Here the USART1 receive interrupt is enabled
   * and the interrupt controller is configured
   * to jump to the USART1_IRQHandler() function
   * if the USART1 receive interrupt occurs
   */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		 // we want to configure the USART1 interrupts
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART1 interrupts
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART1 interrupts are globally enabled
  NVIC_Init(&NVIC_InitStructure);							 // the properties are passed to the NVIC_Init function which takes care of the low level stuff

  // finally this enables the complete USART1 peripheral
  USART_Cmd(USART1, ENABLE);
}

/* This function is used to transmit a string of characters via
 * the USART specified in USARTx.
 *
 * It takes two arguments: USARTx --> can be any of the USARTs e.g. USART1, USART2 etc.
 * 						   (volatile) char *s is the string you want to send
 *
 * Note: The string has to be passed to the function as a pointer because
 * 		 the compiler doesn't know the 'string' data type. In standard
 * 		 C a string is just an array of characters
 *
 * Note 2: At the moment it takes a volatile char because the received_string variable
 * 		   declared as volatile char --> otherwise the compiler will spit out warnings
 * */
void USART_puts(volatile char *s){

  while(*s){
    // wait until data register is empty
    while( !(USART1->SR & 0x00000040) );
    USART_SendData(USART1, *s);
    s++;
  }
}

void USART_putInt(int16_t input){
  int len = 0; 
  int c = 0; 
  int flag = 0;
  char buffer[8]; 
  if (input < 0){
    buffer[len++] = '-'; 
    input = -input; 
  }
/*  if (( c = input / 10000) ){
    input -= c * 10000; 
    buffer[len++] = '0' + c; 
    flag = 1; 
  }
  if (flag || (c = (input / 1000))){
    input -= c * 1000; 
    buffer[len++] = '0' + c; 
    flag = 1; 
  }
*/
  if (flag || input > 9999){
    c = 0; 
    while(input > 9999){
      c++, input -= 10000; 
    }
    buffer[len++] = '0' + c; 
    flag = 1; 
  }

  if (flag || input >= 999){
    c = 0; 
    while(input > 999){
      c++, input -= 1000; 
    }
    buffer[len++] = '0' + c; 
    flag = 1; 
  }
  if (flag || input >= 99){
    c = 0; 
    while(input > 99){
      c++, input -= 100; 
    }
    buffer[len++] = '0' + c; 
    flag = 1; 
  }
  if (flag || input > 9){
    c = 0;
    while(input > 9){
      c++, input -= 10; 
    }
    buffer[len++] = '0' + c; 
  }
  buffer[len++] = '0' + input; 
  buffer[len++] = '\0';
  USART_puts(buffer); 
}

void USART_sendByte(uint8_t byte){
  USART_SendData(USART1, byte);
}

// this is the interrupt request handler (IRQ) for ALL USART1 interrupts
void USART1_IRQHandler(void){

  // check if the USART1 receive interrupt flag was set
  if (USART_GetITStatus(USART1, USART_IT_RXNE) ){

    //static uint8_t cnt = 0; // this counter is used to determine the string length
    char t = USART1->DR; // the character from the USART1 data register is saved in t
    USART_SendData(USART1,t);
    /* check if the received character is not the LF character (used to determine end of string)
     * or the if the maximum string length has been been reached
     */
    /*if( (t != '\n') && (cnt < MAX_STRLEN) ){
      received_string[cnt] = t;
      cnt++;
      }
      else{ // otherwise reset the character counter and print the received string
      cnt = 0;
      USART_puts(USART1, received_string);
      }
      */
  }
}

