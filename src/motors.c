#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "misc.h"
#include "motors.h"
#include "usart.h"
#include "common.h"

// Left Motor Channels 
#define ENCRA_PIN GPIO_Pin_0
#define ENCRA_GPIO_PORT GPIOA
#define ENCRA_GPIO_CLK RCC_AHB1Periph_GPIOA
#define ENCRA_SOURCE GPIO_PinSource0
#define ENCRA_AF GPIO_AF_TIM5 

#define ENCRB_PIN GPIO_Pin_1
#define ENCRB_GPIO_PORT GPIOA
#define ENCRB_GPIO_CLK RCC_AHB1Periph_GPIOA
#define ENCRB_SOURCE GPIO_PinSource1
#define ENCRB_AF GPIO_AF_TIM5

#define ENCR_TIMER TIM5 
#define ENCR_TIMER_CLK RCC_APB1Periph_TIM5

// Right Motor Channels 
#define ENCLA_PIN GPIO_Pin_15
#define ENCLA_GPIO_PORT GPIOA 
#define ENCLA_GPIO_CLK RCC_AHB1Periph_GPIOA
#define ENCLA_SOURCE GPIO_PinSource15
#define ENCLA_AF GPIO_AF_TIM2

#define ENCLB_PIN GPIO_Pin_3
#define ENCLB_GPIO_PORT GPIOB
#define ENCLB_GPIO_CLK RCC_AHB1Periph_GPIOB
#define ENCLB_SOURCE GPIO_PinSource3
#define ENCLB_AF GPIO_AF_TIM2

#define ENCL_TIMER TIM2
#define ENCL_TIMER_CLK RCC_APB1Periph_TIM2

// Helpers
#define LEFT_COUNT() ENCL_TIMER->CNT 
#define RIGHT_COUNT() ENCR_TIMER->CNT

// MOTOR CONTROL 
#define PWM_PERIOD 	0x8000
#define PWM_MAX 		0x4000
#define PWM_SCALER 	700
#define PWM_TIMER TIM3
#define DIR_PORT GPIOE
#define DIR_PIN_FR GPIO_Pin_12
#define DIR_PIN_RR GPIO_Pin_13
#define DIR_PIN_RL GPIO_Pin_14
#define DIR_PIN_FL GPIO_Pin_15

// Private storage
static motors_t _storage;

// Prototypes
void encodersReset(void);

void initEncoders (void) { 
  GPIO_InitTypeDef GPIO_InitStructure;

  // turn on the clocks for each of the ports needed 
  RCC_AHB1PeriphClockCmd (ENCLA_GPIO_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd (ENCLB_GPIO_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd (ENCRA_GPIO_CLK, ENABLE);
  RCC_AHB1PeriphClockCmd (ENCRB_GPIO_CLK, ENABLE);

  // now configure the pins themselves 
  // they are all going to be inputs with pullups 
  GPIO_StructInit (&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = ENCLA_PIN;
  GPIO_Init (ENCLA_GPIO_PORT, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = ENCLB_PIN;
  GPIO_Init (ENCLB_GPIO_PORT, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = ENCRA_PIN;
  GPIO_Init (ENCRA_GPIO_PORT, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = ENCRB_PIN;
  GPIO_Init (ENCRB_GPIO_PORT, &GPIO_InitStructure);

  // Connect the pins to their Alternate Functions 
  GPIO_PinAFConfig (ENCLA_GPIO_PORT, ENCLA_SOURCE, ENCLA_AF);
  GPIO_PinAFConfig (ENCLB_GPIO_PORT, ENCLB_SOURCE, ENCLB_AF);
  GPIO_PinAFConfig (ENCRA_GPIO_PORT, ENCRA_SOURCE, ENCRA_AF);
  GPIO_PinAFConfig (ENCRB_GPIO_PORT, ENCRB_SOURCE, ENCRB_AF);

  // Timer peripheral clock enable 
  RCC_APB1PeriphClockCmd (ENCL_TIMER_CLK, ENABLE);
  RCC_APB1PeriphClockCmd (ENCR_TIMER_CLK, ENABLE);

  // set them up as encoder inputs 
  // set both inputs to rising polarity to let it use both edges 
  TIM_EncoderInterfaceConfig (ENCL_TIMER, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_SetAutoreload (ENCL_TIMER, 0xffffffff);
  TIM_EncoderInterfaceConfig (ENCR_TIMER, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_SetAutoreload (ENCR_TIMER, 0xffffffff);

  // turn on the timer/counters 
  TIM_Cmd (ENCL_TIMER, ENABLE);
  TIM_Cmd (ENCR_TIMER, ENABLE);
  encodersReset();
}

void initPWM(void){
  uint16_t PrescalerValue = 0;

  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  /* GPIOC Configuration: TIM3 CH3 (PB0) and TIM3 CH4 (PB1) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 

  /* Connect TIM3 pins to AF2 */ 
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM3);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3); 

  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel3 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC3Init(TIM3, &TIM_OCInitStructure);
  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
  
  /* PWM1 Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OC4Init(TIM3, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}

void initMotorDir(void){
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  
  GPIO_InitTypeDef gpio;
  GPIO_StructInit(&gpio);
  gpio.GPIO_Mode = GPIO_Mode_OUT;
  gpio.GPIO_Pin  = DIR_PIN_FL | DIR_PIN_FR | DIR_PIN_RL | DIR_PIN_RR; 
  GPIO_Init(DIR_PORT, &gpio);
}

void encodersReset(void){
  __disable_irq();
  TIM_SetCounter (ENCL_TIMER, 0);
  TIM_SetCounter (ENCR_TIMER, 0);
  __enable_irq();
}

int getLeftCount(void){
  return -(LEFT_COUNT());
}
int getRightCount(void){
  return (RIGHT_COUNT());
}

void setSpeeds(Motors self, float l, float r){
  long L_PWM = 0;
  long R_PWM = 0;
  
//  TIM3->CCR3 = 0; 
//  TIM3->CCR4 = 0; 
  
  if ( l < 0 ){
    l = -l;
    GPIO_SetBits(DIR_PORT, DIR_PIN_RL);
    GPIO_ResetBits(DIR_PORT, DIR_PIN_FL);
  } else {
    GPIO_SetBits(DIR_PORT, DIR_PIN_FL);
    GPIO_ResetBits(DIR_PORT, DIR_PIN_RL);
  }
  if (r < 0 ){
    r = -r;
    GPIO_SetBits(DIR_PORT, DIR_PIN_RR);
    GPIO_ResetBits(DIR_PORT, DIR_PIN_FR);
  } else {
    GPIO_SetBits(DIR_PORT, DIR_PIN_FR);
    GPIO_ResetBits(DIR_PORT, DIR_PIN_RR);
  }

  L_PWM = self->PWM_Min_L + (long)(l * PWM_SCALER); 
  R_PWM = self->PWM_Min_R + (long)(r * PWM_SCALER); 
  TIM3->CCR3 = (int) (R_PWM < PWM_MAX ? R_PWM : PWM_MAX); 
  TIM3->CCR4 = (int) (L_PWM < PWM_MAX ? L_PWM : PWM_MAX); 
}; 

void setOffset(Motors self, int offset_L, int offset_R){
  self->PWM_Min_L = offset_L; 
  self->PWM_Min_R = offset_R; 
}

void setMotorTargSpeeds(Motors self, float leftTargSpeed, float rightTargSpeed){
	self->leftTargetSpeed = leftTargSpeed;
	self->rightTargetSpeed = rightTargSpeed;
}

void setMotorPIDGains(Motors self, float prop, float sum, float diff){
	self->p = prop;
	self->s = sum;
	self->d = diff;
}

static pidError_t eLm = {0.f, 0.f, 0.f, 1};
static pidError_t eRm = {0.f, 0.f, 0.f, 1};


static int firstMotorPID = 1;
static float initialMotorTime = 0;

static long currentLeftTicks = 0;
static long currentRightTicks = 0;
static long prevLeftTicks = 0;
static long prevRightTicks = 0;

static float currentTime = 0;
static float previousTime = 0;

void doMotorPID(Motors self){
	if(firstMotorPID){
		firstMotorPID = 0;
		initialMotorTime = 1;
		currentTime = 1 - initialMotorTime;
		currentLeftTicks = self->getLeftCount();
		currentRightTicks = self->getRightCount();
		self->setSpeeds(self,self->leftTargetSpeed,self->rightTargetSpeed);
		return;
	}
	
	previousTime = currentTime;
	currentTime = 1 - initialMotorTime;
	prevLeftTicks = currentLeftTicks;
	prevRightTicks = currentRightTicks;
	currentLeftTicks = self->getLeftCount();
	currentRightTicks = self->getRightCount();
	
	float leftVelErr,rightVelErr,deltaT,leftFbkVel,rightFbkVel;
	
	deltaT = currentTime - previousTime;
	
	leftVelErr = self->leftTargetSpeed - (ENC_TO_D(currentLeftTicks-prevLeftTicks))/(deltaT);
	rightVelErr = self->rightTargetSpeed - (ENC_TO_D(currentRightTicks-prevRightTicks))/(deltaT);
	
	calcErr(leftVelErr,&eLm,0.f);
	calcErr(rightVelErr,&eRm,0.f);
	
	leftFbkVel = eLm.p*self->p + eLm.s*self->s + eLm.d*self->d;
	rightFbkVel = eRm.p*self->p + eRm.s*self->s + eRm.d*self->d;
	
	self->setSpeeds(self,self->leftTargetSpeed+leftFbkVel,self->rightTargetSpeed+rightFbkVel);
}

Motors createMotors(void){
  Motors m = &_storage; 
	
  m->leftTargetSpeed = 0;
  m->rightTargetSpeed = 0;
  
  m->p = 0;
  m->s = 0;
  m->d = 0;
  	
  m->PWM_Min_L = 0xA80;
  m->PWM_Min_R = 0x800;

  m->getLeftCount   = getLeftCount;
  m->getRightCount  = getRightCount;
  m->resetCounts    = encodersReset;

  m->setSpeeds      = setSpeeds; 
  m->setOffset      = setOffset;
  m->doMotorPID     = doMotorPID;

  initEncoders();
  initPWM(); 


  initMotorDir(); 
  return m; 
}
