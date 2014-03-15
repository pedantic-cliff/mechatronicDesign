#include "pid.h"

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "misc.h"

/* leds in the board will fade */
const uint16_t LEDS = GPIO_Pin_13 | GPIO_Pin_15;
static float  Kp = 0.f,
              Ki = 0.f, 
              Kd = 0.f;


void PWM_Config(int period); 
void TIM_Config(void); 
void ENC_Config(void); 
void setSpeeds(long left, long right); 

void initPID(float p, float i, float d){
  Kp = p, Ki = i, Kd = d;
  TIM_Config(); 
  PWM_Config(1000); 
  ENC_Config();
}

void pid_pos(long pos){
  setSpeeds(50,100); 
}

void setSpeeds(long left, long right){
  TIM3->CCR1 = left;
  TIM3->CCR2 = right; 
}

void TIM_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* GPIOC and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  /* GPIOC Configuration: TIM3 CH1 (PC6) and TIM3 CH2 (PC7) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOC, &GPIO_InitStructure); 

  /* Connect TIM3 pins to AF2 */ 
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM3); 
}
void PWM_Config(int period)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;
  uint16_t PrescalerValue = 0;
  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = period;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM3, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
  /* PWM1 Mode configuration: Channel2 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OC2Init(TIM3, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}
void ENC_Config(void){
  //******* Currently only does one port **********//
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_ICInitTypeDef TIM_ICInitStructure;

  /*TIM2 clock source enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* Enable GPIOA, clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);   

  /* Encoder unit connected to TIM2, quadrature mode */   
  //GPIOA Config  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Enable the TIM2 Update Interrupt for left encoder*/                                                                           
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;            
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Timer configuration in Encoder mode for left encoder*/
  TIM_TimeBaseStructure.TIM_Prescaler = 0x00;  // No prescaling
  TIM_TimeBaseStructure.TIM_Period = 65535; //max resolution
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//devide by clock by one
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;// count up  
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12,  TIM_ICPolarity_Falling, TIM_ICPolarity_Falling);
  TIM_ICStructInit(&TIM_ICInitStructure);
  TIM_ICInitStructure.TIM_ICFilter = 6;//ICx_FILTER;
  TIM_ICInit(TIM2, &TIM_ICInitStructure);

  // Clear all pending interrupts
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  //Reset counter
  TIM2->CNT = 10000;//prevent exceeding 0 when turning wheel backwards
  TIM_Cmd(TIM2, ENABLE);//enable left encoder
}

