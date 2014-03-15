#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "colorSensor.h"
#include "usart.h"

__IO uint16_t ADC1ConvertedValue[NUM_SENSORS];

volatile Color currColor; 
volatile int currIdx;
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
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}
  
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* ADC Channel 14 -> PC4
  */
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 
                                | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}
  
/**************************************************************************************/
  
void ADC_Configuration(void)
{
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_InitTypeDef ADC_InitStructure;
  
  /* ADC Common Init */
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4; // 2 4 6 or 8
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_10Cycles; //min is 10
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
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_480Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 2, ADC_SampleTime_480Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 3, ADC_SampleTime_480Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 4, ADC_SampleTime_480Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 5, ADC_SampleTime_480Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 6, ADC_SampleTime_480Cycles); // this is max

  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
  NVIC_EnableIRQ(ADC_IRQn);

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
  int i = 0;
  enableLEDs(BLUE);
  colorSensors.done = 0;
  for(; i < NUM_SENSORS; i++){
    ADC1ConvertedValue[i] = 0;
  }
  ADC_SoftwareStartConv(ADC1);
} 

void ADC_IRQHandler(void){
  int i ;
  for(i = 0; i < NUM_SENSORS; i++){
    sensors[i].measurements[currIdx] = ADC1ConvertedValue[i]; 
  }
  colorSensors.done = 1;
  disableLEDs(BLUE);
}

void measureColor(ColorSensors cs, Color c){
  int i;
  GPIO_ResetBits(LIGHT_PORT, ALL_LIGHTS);
  switch(c){
    case RED:
      GPIO_SetBits(LIGHT_PORT, RED_PIN); 
      currIdx = RED_IDX;
      break;
    case GREEN:
      GPIO_SetBits(LIGHT_PORT, GREEN_PIN); 
      currIdx = GREEN_IDX;
      break;
    case BLUE: 
      GPIO_SetBits(LIGHT_PORT, BLUE_PIN); 
      currIdx = BLUE_IDX;
      break;
    case NONE: 
      currIdx = NONE_IDX;
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

