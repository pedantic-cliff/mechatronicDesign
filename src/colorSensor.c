#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "colorSensor.h"
#include "main.h"

__IO uint16_t ADC3ConvertedValue[4];
__IO uint32_t ADC3ConvertedVoltage[4];

volatile COLORS currColor; 

struct lightSensor_t sensors[NUM_SENSORS]; 
struct colorSensors_t colorSensors;

void initDMA(void){
  DMA_InitTypeDef       DMA_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

 /* DMA2 Stream0 channel0 configuration **************************************/
  DMA_DeInit(DMA2_Stream0);
  DMA_InitStructure.DMA_Channel = DMA_Channel_2;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC3_DR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC3ConvertedValue[0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 4;
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

void initADC(void){
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;

  /* Enable ADC3 and GPIO clocks ****************************************/  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

  /* Configure ADC3 Channel1,2,3,4 pin as analog input ******************************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; // Orig dis
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC3 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE; //orig disable
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC3, &ADC_InitStructure);

  /* ADC3 regular channel1,2,3,4 configuration *************************************/
  ADC_RegularChannelConfig(ADC3, ADC_Channel_1, 1, ADC_SampleTime_15Cycles);
//  ADC_RegularChannelConfig(ADC3, ADC_Channel_2, 2, ADC_SampleTime_15Cycles);
//  ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 3, ADC_SampleTime_15Cycles);
//  ADC_RegularChannelConfig(ADC3, ADC_Channel_4, 4, ADC_SampleTime_15Cycles);

 /* Enable DMA request after last transfer (Single-ADC mode) */
  //ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

  /* Enable ADC3 DMA */
  //ADC_DMACmd(ADC3, ENABLE);

  /* Enable ADC3 */
  ADC_Cmd(ADC3, ENABLE);
}
void initLEDs(void){

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitTypeDef gpio;
  GPIO_StructInit(&gpio);
  gpio.GPIO_Mode = GPIO_Mode_OUT;
  gpio.GPIO_Pin  = ALL_LIGHTS;
  GPIO_Init(LIGHT_PORT, &gpio);
}

void initSensors(void){
  //initDMA();
  initADC();
}

void initialize(ColorSensors this){
  initLEDs(); 
  initSensors(); 
}

void startADC(int colorIdx){
  delay(100); 
  NVIC_EnableIRQ(ADC_IRQn);
  ADC_SoftwareStartConv(ADC3);
} 

void ADC_IRQHandler(void){
  colorSensors.sensors[0].measurements[0] = ADC_GetConversionValue(ADC3);
  colorSensors.sensors[0]->validColors |= RED_MASK;
}

void measureRed(ColorSensors cs){
  GPIO_ResetBits(LIGHT_PORT, ALL_LIGHTS);
  GPIO_SetBits(LIGHT_PORT, RED_PIN); 
  currColor = RED;
  for(int i = 0; i < NUM_SENSORS; i++){
    cs->sensors[i]->validColors &= ~RED_MASK; 
  }
  startADC(RED_IDX); 
}

void measureGreen(ColorSensors cs){
  GPIO_ResetBits(LIGHT_PORT, ALL_LIGHTS);
  GPIO_SetBits(LIGHT_PORT, GREEN_PIN); 
  currColor = GREEN;
  for(int i = 0; i < NUM_SENSORS; i++){
    cs->sensors[i]->validColors &= ~GREEN_MASK; 
  }
  startADC(GREEN_IDX); 
}

void measureBlue(ColorSensors cs){
  GPIO_ResetBits(LIGHT_PORT, ALL_LIGHTS);
  GPIO_SetBits(LIGHT_PORT, BLUE_PIN); 
  currColor = BLUE;
  for(int i = 0; i < NUM_SENSORS; i++){
    cs->sensors[i]->validColors &= ~BLUE_MASK; 
  }
  startADC(BLUE_IDX); 
}

void measureNone(ColorSensors cs){
  GPIO_ResetBits(LIGHT_PORT, ALL_LIGHTS);
  currColor = NONE;
  for(int i = 0; i < NUM_SENSORS; i++){
    cs->sensors[i]->validColors &= ~NONE_MASK; 
  }
  startADC(NONE_IDX); 
}

ColorSensors createColorSensors(void){
  ColorSensors cs = &colorSensors; 

  cs->init          = initialize; 

  cs->measureRed    = measureRed; 
  cs->measureGreen  = measureGreen;
  cs->measureBlue   = measureBlue;
  cs->measureNone   = measureNone;

  return &colorSensors;
}

