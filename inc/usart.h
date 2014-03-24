// Uses Pins PB6 as Tx and PB7 as Rx

#include <stm32f4xx_usart.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src

void init_USART(void); 
void USART_puts(volatile char * str); 
void USART_putInt(int value); 
void USART_putFloat(float value); 
void USART_sendByte(uint8_t byte); 
