#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "colorSensor.h"
#include "usart.h"

#define LIGHT_PORT  GPIOE
#define RED_PIN     GPIO_Pin_9
#define GREEN_PIN   GPIO_Pin_10
#define BLUE_PIN    GPIO_Pin_11
#define ALL_LIGHTS ( RED_PIN | GREEN_PIN | BLUE_PIN )

#define SENSOR_PORT GPIOD

typedef struct{ 
  float r; 
  float g; 
  float b;
} centroid; 

static centroid edge    = { 101.0463f,  280.8654f,  243.8729f };
static centroid metal   = { 1172.3f,    1944.7f,    1740.8f   };
static centroid yellow  = { 2274.5f,    1933.0f,    621.7f    };
static centroid white   = { 2239.8f,    2158.0f,    2219.5f   };

  ADC_InitStructure.ADC_ExternalTrigConvEdge  = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv      = ADC_ExternalTrigConv_T8_CC1;
  ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion       = NUM_SENSORS;
  ADC_Init(ADC1, &ADC_InitStructure);
  
  /* ADC1 regular channel 11 configuration */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_3,  1, ADC_SampleTime_144Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_144Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_144Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_144Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 5, ADC_SampleTime_144Cycles); // this is max
  ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 6, ADC_SampleTime_144Cycles); // this is max

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

void startADC(void){
  int i = 0;
  colorSensors.done = 0;
  for(; i < NUM_SENSORS; i++){
    ADC1ConvertedValue[i] = 0;
  }
  delay(150);
  enableLEDs(GREEN);
//  TIM_SetCounter(TIM8, 0xFFF);
//  TIM_Cmd(TIM8, ENABLE);
  ADC_SoftwareStartConv(ADC1);
} 

void ADC_IRQHandler(void){
  int i ;
  for(i = 0; i < NUM_SENSORS; i++){
    sensors[i].measurements[currIdx] += ADC1ConvertedValue[i]; 
  }
  colorSensors.done += 1;
  if(colorSensors.done < COLOR_SENSOR_ITERS)
    ADC_SoftwareStartConv(ADC1);
  else
    disableLEDs(GREEN);
}

void measureColor(ColorSensors cs, Color c){
  GPIO_SetBits(LIGHT_PORT, GREEN_PIN |  RED_PIN | BLUE_PIN);
  switch(c){
    case RED:
      GPIO_ResetBits(LIGHT_PORT, RED_PIN); 
      currIdx = RED_IDX;
      break;
    case GREEN:
      GPIO_ResetBits(LIGHT_PORT, GREEN_PIN); 
      currIdx = GREEN_IDX;
      break;
    case BLUE: 
      GPIO_ResetBits(LIGHT_PORT, BLUE_PIN); 
      currIdx = BLUE_IDX;
      break;
    case NONE: 
    default: 
      currIdx = NONE_IDX;
      break;
  }
  currColor = c;
  startADC(); 
}

volatile uint16_t* getResult(void){
  return ADC1ConvertedValue;
}

float calcCentDiff(int r, int g, int b, centroid cent){
  float score = 0.f;

  return score;
}
void guessColor(int r, int g, int b){
  float s_edge = calcCentDiff(r,g,b,edge),
        s_metal = calcCentDiff(r,g,b,edge)
  
}

ColorSensors createColorSensors(void){
  ColorSensors cs = &colorSensors; 

  initLights(); 
  initSensors(); 

  cs->measureColor  = measureColor;
  cs->getResult     = getResult; 
  cs->guessColor    = guessColor; 

  return &colorSensors;
}

