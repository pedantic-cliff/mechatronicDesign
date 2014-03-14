#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "colorSensor.h"
#include "usart.h"

__IO uint16_t ADC1ConvertedValue[NUM_SENSORS];

volatile Color currColor; 
volatile int done;

struct lightSensor_t sensors[NUM_SENSORS]; 
struct colorSensors_t colorSensors;

void initDMA(void){
  DMA_InitTypeDef       DMA_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

 // * DMA2 Stream0 channel0 configuration ************************************* //
  DMA_DeInit(DMA2_Stream0);
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC1ConvertedValue[0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = NUM_SENSORS;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // orig dis
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //orig dis
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);
}

void RCC_Configuration(void){
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}
  
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* ADC Channel 14 -> PC4
  */
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | 
                                GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}
  
/**************************************************************************************/
  
void ADC_Configuration(void)
{
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_InitTypeDef ADC_InitStructure;
  
  /* ADC Common Init */
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);  
  
  ADC_InitStructure.ADC_Resolution            = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode          = ENABLE; 
  ADC_InitStructure.ADC_ContinuousConvMode    = DISABLE; // Conversions Triggered
  ADC_InitStructure.ADC_ExternalTrigConvEdge  = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv      = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion       = NUM_SENSORS;
  ADC_Init(ADC1, &ADC_InitStructure);
  
  /* ADC1 regular channel 11 configuration */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_15Cycles); // PC0
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_15Cycles); // PC1
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_15Cycles); // PC2
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_15Cycles); // PC3
  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 5, ADC_SampleTime_15Cycles); // PC4
  ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 6, ADC_SampleTime_15Cycles); // PC5

  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
  //NVIC_EnableIRQ(ADC_IRQn);

  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
}
 
  
void initLights(void){

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitTypeDef gpio;
  GPIO_StructInit(&gpio);
  gpio.GPIO_Mode = GPIO_Mode_OUT;
  gpio.GPIO_Pin  = ALL_LIGHTS;
  GPIO_Init(LIGHT_PORT, &gpio);
}


void initSensors(void){
  initDMA();
  RCC_Configuration();
  GPIO_Configuration();
  ADC_Configuration();
}

void initialize(ColorSensors this){
  initLights(); 
  initSensors(); 
}

void startADC(void){
  colorSensors.done = 0;
  delay(100); 
  ADC_SoftwareStartConv(ADC1);
} 

void ADC_IRQHandler(void){
  colorSensors.done = 1;
  enableLEDs(BLUE);
}

void measureColor(ColorSensors cs, Color c){
  int i;
  GPIO_ResetBits(LIGHT_PORT, ALL_LIGHTS);
  switch(c){
    case RED:
      GPIO_SetBits(LIGHT_PORT, RED_PIN); 
      break;
    case GREEN:
      GPIO_SetBits(LIGHT_PORT, GREEN_PIN); 
      break;
    case BLUE: 
      GPIO_SetBits(LIGHT_PORT, BLUE_PIN); 
      break;
    case NONE: 
      break;
  }
  currColor = c;
  startADC(); 
}

volatile uint16_t* getResult(void){
  return ADC1ConvertedValue;
}

ColorSensors createColorSensors(void){
  ColorSensors cs = &colorSensors; 

  cs->init          = initialize; 

  cs->measureColor  = measureColor;
  cs->getResult     = getResult; 

  return &colorSensors;
}

