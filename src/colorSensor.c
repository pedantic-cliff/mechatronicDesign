#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "colorSensor.h"

__IO uint16_t ADC3ConvertedValue[4];
__IO uint32_t ADC3ConvertedVoltage[4];

volatile Color currColor; 

struct lightSensor_t sensors[NUM_SENSORS]; 
struct colorSensors_t colorSensors;

void RCC_Configuration(void)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}
  
/**************************************************************************************/
  
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* ADC Channel 11 -> PC1
  */
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
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
  ADC_InitStructure.ADC_ScanConvMode          = DISABLE; // 1 Channel
  ADC_InitStructure.ADC_ContinuousConvMode    = DISABLE; // Conversions Triggered
  ADC_InitStructure.ADC_ExternalTrigConvEdge  = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv      = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion       = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
  
  /* ADC1 regular channel 11 configuration */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_15Cycles); // PC1
  
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
  //initDMA();
  RCC_Configuration();
  GPIO_Configuration();
  ADC_Configuration();
}

void initialize(ColorSensors this){
  initLights(); 
  initSensors(); 
}

void startADC(void){
  delay(100); 
  //NVIC_EnableIRQ(ADC_IRQn);
  ADC_SoftwareStartConv(ADC3);
} 

void ADC_IRQHandler(void){
  colorSensors.sensors[0]->measurements[0] = ADC_GetConversionValue(ADC3);
  colorSensors.sensors[0]->validColors |= RED;
}

void measureColor(ColorSensors cs, Color c){
  int i;
  for(i = 0; i < NUM_SENSORS; i++){
    cs->sensors[i]->validColors &= ~c; 
  }
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

ColorSensors createColorSensors(void){
  ColorSensors cs = &colorSensors; 

  cs->init          = initialize; 

  cs->measureColor  = measureColor;

  return &colorSensors;
}

