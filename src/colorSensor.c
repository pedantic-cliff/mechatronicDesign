#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "colorSensor.h"
#include "sensorConfig.h"
#include "usart.h"
#include "localize.h"
#include "map.h"
#include "main.h"

#define LIGHT_PORT  GPIOE
#define RED_PIN     GPIO_Pin_9
#define GREEN_PIN   GPIO_Pin_10
#define BLUE_PIN    GPIO_Pin_11
#define ALL_LIGHTS ( RED_PIN | GREEN_PIN | BLUE_PIN )

#define SENSOR_PORT GPIOD

#define TIMER_PRESCALER 20800
#define TIMER_DELAY     100


// Order goes edge+black, metal, yellow
static struct centroid *centroids[6]; 
__IO uint16_t ADC1ConvertedValue[NUM_SENSORS];
volatile static int color_running = 0; 
volatile Color currColor; 
volatile int currIdx;
volatile int done;

struct lightSensor_t sensors[NUM_SENSORS]; 
struct colorSensors_t _colorSensors;

Color colorState; 

void guessColor(pConfidences c, int r, int g, int b, struct centroid *cent);
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
  RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
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

void ADC_TimerStop(void){
  TIM_Cmd(TIM4, DISABLE);
}

void ADC_UpdateTimerPeriod(int period){
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_Cmd(TIM4, DISABLE);
  TIM_TimeBaseStructure.TIM_Period = period;
  TIM_TimeBaseStructure.TIM_Prescaler = TIMER_PRESCALER;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  TIM4->CCR4 = period;
  TIM_Cmd(TIM4, ENABLE);
}

/**************************************************************************************/
void ADC_TimerConfig(void){
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;

  TIM_TimeBaseStructure.TIM_Period = 0xF000;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  //  TIM_SelectOutputTrigger(TIM8,TIM_TRGOSource_OC1);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC4Init(TIM4, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Disable);

  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_TIM4);
  TIM4->CCR4 = 0x1000;
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

  ADC_InitStructure.ADC_Resolution            = ADC_Resolution_10b; //12b 10b 8b 6b
  ADC_InitStructure.ADC_ScanConvMode          = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode    = DISABLE; // Conversions Triggered
  ADC_InitStructure.ADC_ExternalTrigConvEdge  = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv      = ADC_ExternalTrigConv_T4_CC4;
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
  GPIO_SetBits(LIGHT_PORT, ALL_LIGHTS);
}

void initSensors(void){
  colorState = NONE;
  initDMA();
  ADC_TimerConfig();
  RCC_Configuration();
  GPIO_Configuration();
  ADC_Configuration();
  _colorSensors.isCalibrating = 0;
}

void startADC(void){
  int i = 0;  
  color_running = 1;
  _colorSensors.done = 0;
  for(; i < NUM_SENSORS; i++){
    ADC1ConvertedValue[i] = 0;
    sensors[i].measurements[currIdx] = 0;
  }
  ADC_UpdateTimerPeriod(TIMER_DELAY);
} 

void startColor(Color c){
  GPIO_SetBits(LIGHT_PORT, GREEN_PIN |  RED_PIN | BLUE_PIN);
  colorState = c;
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
  startADC();
}

void finish(){
  int s;
  float ambient, red, green, blue; 
  confidences_t conf;
  sensorPos poses = localizer->findSensorLocations(localizer);
  for(s = 0; s < NUM_SENSORS; s++){
    ambient = _colorSensors.sensors[s].measurements[NONE_IDX];
    red     = (_colorSensors.sensors[s].measurements[RED_IDX] - ambient) / COLOR_SENSOR_ITERS;
    green   = (_colorSensors.sensors[s].measurements[GREEN_IDX] - ambient) / COLOR_SENSOR_ITERS;
    blue    = (_colorSensors.sensors[s].measurements[BLUE_IDX] - ambient) / COLOR_SENSOR_ITERS;
    
    guessColor(&conf, red,green,blue, centroids[s]);
    applyConfidence(poses.s[s].row,poses.s[s].col, &conf);

  }
}

void nextColor(void){
  disableLEDs(GREEN);
  if(!color_running)
    return;
  switch(colorState){
    case RED:
      startColor(GREEN);
      break;
    case GREEN:
      startColor(BLUE);
      break;
    case BLUE: 
      finish();
      startColor(NONE);
      break;
    case NONE: 
      startColor(RED);
      break;
    default: 
      startColor(NONE);
      break;
  }
}

void ADC_IRQHandler(void){
  int i ;
  ADC_TimerStop();
  enableLEDs(GREEN);
  for(i = 0; i < NUM_SENSORS; i++){
    _colorSensors.sensors[i].measurements[currIdx] += ADC1ConvertedValue[i]; 
  }
  _colorSensors.done++;
  if((!_colorSensors.isCalibrating)
      && _colorSensors.done == COLOR_SENSOR_ITERS){
    nextColor();
  } else {
    ADC_SoftwareStartConv(ADC1);
  }
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
  delay(1350);
  startADC(); 
}

void calibrateColors(ColorSensors cs){ 
  int i = 0, j = 0, k = 0; 
  Color c = 0; 
  volatile uint16_t *results; 
  float senMeans[NUM_SENSORS];
  float ambMeans[NUM_SENSORS] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
  _colorSensors.isCalibrating = 1; 
  USART_puts("Start Calibrating: \n");
  for(k = 0; k < NUM_COLORS; k++){
    for(i = 0; i < NUM_SENSORS; i++){
      senMeans[i] = 0.f;
    }
    GPIO_SetBits(LIGHT_PORT, ALL_LIGHTS);
    switch(k){
      case 1:
        c = RED;
        GPIO_ResetBits(LIGHT_PORT, RED_PIN); 
        currIdx = RED_IDX;
        USART_puts("Red:\t"); 
        break;
      case 2:
        c = GREEN;
        GPIO_ResetBits(LIGHT_PORT, GREEN_PIN); 
        currIdx = GREEN_IDX;
        USART_puts("Green:\t"); 
        break;
      case 3: 
        c = BLUE;
        GPIO_ResetBits(LIGHT_PORT, BLUE_PIN); 
        currIdx = BLUE_IDX;
        USART_puts("Blue:\t"); 
        break;
      default: 
        c = NONE;
        currIdx = NONE_IDX;
        break;
    }
    currColor = c;
    delay(100);
    for(i = 0; i < COLOR_SENSOR_CALIB_ITERS; i++){
      startADC(); 
      ADC_SoftwareStartConv(ADC1); 
      while(cs->done < COLOR_SENSOR_ITERS); 
      results = cs->getResult(); 
      for(j = 0; j < NUM_SENSORS; j++){
        senMeans[j] += results[j]; 
      }
    }
    if(k == 0){
      for(j = 0; j < NUM_SENSORS; j++){
        ambMeans[j] = senMeans[j];
      }
    }else{
      for(j = 0; j < NUM_SENSORS; j++){
        senMeans[j] -= ambMeans[j];
        senMeans[j] /= COLOR_SENSOR_CALIB_ITERS; 
        USART_putFloat(senMeans[j]);
        USART_puts("\t"); 
      }
      USART_puts("\n");
    }
  }
  GPIO_SetBits(LIGHT_PORT, ALL_LIGHTS);
  _colorSensors.isCalibrating = 0; 
  USART_puts("Done Calibrating\n");
}

volatile uint16_t* getResult(void){
  return ADC1ConvertedValue;
}

float calcCentDiff(int r, int g, int b, struct centroid *cent){
  float score = (cent->r - r)*(cent->r - r) 
    + (cent->g - g)*(cent->g - g) 
    + (cent->b - b)*(cent->b - b);
  return score;
}

void guessColor(pConfidences c, int r, int g, int b, struct centroid *cent){
  float e = calcCentDiff(r,g,b,&cent[0]),
        m = calcCentDiff(r,g,b,&cent[1]),
        y = calcCentDiff(r,g,b,&cent[2]);
  float sum = e + m + y; 
  c->boundary = e;
  c->metal    = m;
  c->yellow   = y;
}

void startColorSensor(void){
}
void haltColorSensor(void){
  color_running = 0; 
  ADC_TimerStop();
  delay_blocking(1000);
  GPIO_SetBits(LIGHT_PORT, ALL_LIGHTS);
}

ColorSensors createColorSensors(void){
  ColorSensors cs = &_colorSensors;

  cs->sensors = sensors;
  initLights(); 
  initSensors(); 

  cs->measureColor  = measureColor;
  cs->getResult     = getResult; 
  cs->startColor    = startColor; 
  cs->start         = startColorSensor;
  cs->halt          = haltColorSensor; 
  cs->calibrateColors = calibrateColors; 

  centroids[0] = cent_sen1;
  centroids[1] = cent_sen2;
  centroids[2] = cent_sen3;
  centroids[3] = cent_sen4;
  centroids[4] = cent_sen5;
  centroids[5] = cent_sen6;

  return cs;
}

