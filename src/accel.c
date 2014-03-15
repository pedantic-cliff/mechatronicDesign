#include <accel.h>
#include <stm32f4xx_spi.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <math.h>

#define CTRL_REG1 0x20
#define OUT_X     0x29
#define OUT_Y     0x2B
#define OUT_Z     0x2D

void initSPI(void);
void sendData(uint8_t addr, uint8_t data);
uint8_t getData (uint8_t addr); 

// Internal Storage for the one Accel object
struct accel _storage; 

uint8_t accel_getX(void){
  return getData(OUT_X);
}
uint8_t accel_getY(void){
  return getData(OUT_Y);
}
uint8_t accel_getZ(void){
  return getData(OUT_Z);
}

float accel_getAngle(){ 
  int8_t x, y; 
  x = accel_getX(); 
  y = accel_getY();
  return 180.*atan2f(y,x)/3.14159; 
}

void initSPI(void){
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  SPI_InitTypeDef SPI_InitTypeDefStruct;


  SPI_InitTypeDefStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitTypeDefStruct.SPI_Mode = SPI_Mode_Master;
  SPI_InitTypeDefStruct.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitTypeDefStruct.SPI_CPOL = SPI_CPOL_High;
  SPI_InitTypeDefStruct.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitTypeDefStruct.SPI_NSS = SPI_NSS_Soft;
  SPI_InitTypeDefStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitTypeDefStruct.SPI_FirstBit = SPI_FirstBit_MSB;

  SPI_Init(SPI1, &SPI_InitTypeDefStruct);


  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE , ENABLE);

  GPIO_InitTypeDef GPIO_InitTypeDefStruct;

  GPIO_InitTypeDefStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitTypeDefStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitTypeDefStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitTypeDefStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitTypeDefStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitTypeDefStruct);

  GPIO_InitTypeDefStruct.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitTypeDefStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitTypeDefStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitTypeDefStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitTypeDefStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOE, &GPIO_InitTypeDefStruct);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

  GPIO_SetBits(GPIOE, GPIO_Pin_3);

  SPI_Cmd(SPI1, ENABLE);
}

void sendData(uint8_t address, uint8_t data){
  GPIO_ResetBits(GPIOE, GPIO_Pin_3);

  while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
  SPI_I2S_SendData(SPI1, address);
  while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)); // Reset RXNE flag
  SPI_I2S_ReceiveData(SPI1);

  while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
  SPI_I2S_SendData(SPI1, data);
  while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)); // Reset RXNE flag
  SPI_I2S_ReceiveData(SPI1);

  GPIO_SetBits(GPIOE, GPIO_Pin_3);
}

uint8_t getData(uint8_t address){
  GPIO_ResetBits(GPIOE, GPIO_Pin_3); 

  address = 0x80 | address; // Mark as Read Mode

  while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
  SPI_I2S_SendData(SPI1, address);
  while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
  SPI_I2S_ReceiveData(SPI1); //Clear RXNE bit

  while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
  SPI_I2S_SendData(SPI1, 0x00); //Dummy byte to generate clock
  while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));

  GPIO_SetBits(GPIOE, GPIO_Pin_3);

  return  SPI_I2S_ReceiveData(SPI1);
}

Accel initAccel(void){
  Accel a = (Accel)&_storage;
  initSPI();
  sendData(CTRL_REG1, 0x67); // POWER_ON | Zen | Yen | Xen 

  a->getX = accel_getX;
  a->getY = accel_getY;
  a->getZ = accel_getZ; 
  a->getAngle = accel_getAngle; 
  return a;
}

