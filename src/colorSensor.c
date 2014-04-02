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

typedef struct centroid{ 
  float r; 
  float g; 
  float b;
} centroid_t; 

static struct centroid edge    = { 101.0463f,  243.8729f,  280.8654f };
static struct centroid metal   = { 1172.3f,    1740.8f,   1944.7f    };
static struct centroid yellow  = { 2274.5f,    621.7f,    1933.0f    };
static struct centroid white   = { 2239.8f,    2219.5f,   2158.0f    };

static struct centroid *centroids[4]; 
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
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}
  
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 
                              | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}
  
/**************************************************************************************/
void ADC_TimerConfig(void){
  RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;
  
  TIM_TimeBaseStructure.TIM_Period = 0xFF;
  TIM_TimeBaseStructure.TIM_Prescaler = 0x4;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
 
  // Stop Timer after one iteration
  TIM_SelectOnePulseMode(TIM8, TIM_OPMode_Single); 
  // Set iterrupt bit but don't call interrupts
  
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM8, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
  TIM8->CCR1 = 0; 
  TIM_ITConfig(TIM8, TIM_IT_CC1, ENABLE);
}

void ADC_Configuration(void)
{
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_InitTypeDef ADC_InitStructure;
  
  /* ADC Common Init */
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8; // 2 4 6 or 8
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_10Cycles; //min is 10
  ADC_CommonInit(&ADC_CommonInitStructure);  
  
  ADC_InitStructure.ADC_Resolution            = ADC_Resolution_12b; //12b 10b 8b 6b
  ADC_InitStructure.ADC_ScanConvMode          = ENABLE; 
  ADC_InitStructure.ADC_ContinuousConvMode    = DISABLE; // Conversions Triggered
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

float calcCentDiff(int r, int g, int b, centroid_t *cent){
  float score = (cent->r - r)*(cent->r - r) 
              + (cent->g - g)*(cent->g - g) 
              + (cent->b - b)*(cent->b - b);
  return score;
}
void guessColor(int r, int g, int b){
  int i = 0; 
  int minIdx = 0; 
  float min = calcCentDiff(r,g,b,centroids[0]);
  float val = 0.f;
  for(i = 1; i < 4; i++){
    val = calcCentDiff(r,g,b,centroids[i]); 
    if(val < min){
      min = val;
      minIdx = i;
    }
  } 

  switch(minIdx){
    case 0:
      USART_puts("Edge\n");
      break;
    case 1:
      USART_puts("Metal\n");
      break;
    case 2:
      USART_puts("Yellow\n");
      break;
    case 3: 
      USART_puts("White\n");
      break;
  }
}

ColorSensors createColorSensors(void){
  ColorSensors cs = &colorSensors; 

  initLights(); 
  initSensors(); 

  cs->measureColor  = measureColor;
  cs->getResult     = getResult; 
  cs->guessColor    = guessColor; 

  centroids[0] = &edge;
  centroids[1] = &metal;
  centroids[2] = &yellow;
  centroids[3] = &white;

  return &colorSensors;
}

