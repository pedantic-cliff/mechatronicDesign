#include "main.h"
#include "misc.h"
#include <stdio.h>
#include "math.h"
#include "state.h"

volatile int running = 0;
int calibrateColor = 0; 
int sendMap = 0;
ColorSensors colorSensors; 
Accel accel; 
Motors motors; 

Pid pid; 
PID_Gains angleGains  = { 35.f, 0.00f, 0.0f },
          distGains   = { 40.f, 0.00f, 0.0f },
          bearGains   = { -20.f, 0.00f, 0.0f },
          motorGains  = { 100.f, 0.00f, 50.0f };

static void init(void);
static void loop(void);

void doUpdateState(void){
  __disable_irq();
  localizer->cacheState(localizer);
  __enable_irq();
}

long time;
void start(void){
  startState();
  running = 1;
  USART_puts("Start!\n");
  time = getCurrentTime();
  colorSensors->startColor(NONE);
}

void halt(void){
  running = 0;
  colorSensors->halt();
}

void doLog();
void doUpdate();

void doColorCalibrate(void){
  colorSensors->calibrateColors(colorSensors); 
}
int main(void) {
  initSysTick(); 
  delay(500); // Give the hardware time to warm up on cold start
  init();
  delay(1000); 
  //start();
  markStarted();
  do {
    doUpdateState();
    if(calibrateColor){
      doColorCalibrate();
      calibrateColor = 0; 
    }

    if(running){
      loop();
    }
    delay(500);
  } while (1);
}

void setCalibrateColor(void){
  calibrateColor = 1; 
}

static void init() {
  init_USART(); 
  initLEDs();
  createGrid();
  colorSensors = createColorSensors(); 
  accel   = initAccel(); 
  motors  = createMotors();
  motors->setMotorPIDGains(motors,motorGains); 
  delay(500);
  localizer = createLocalizer(motors, accel);
  pid = createPID(distGains, bearGains,angleGains, motors); 
  initSysTick(); 
  USART_puts("Init finished\n");
}

void doLog(void){
  USART_putInt(motors->getLeftCount());
  USART_puts("\t");
  USART_putInt(motors->getRightCount());
  USART_puts("\n");

  USART_puts("x, y: ");
  USART_putFloat(localizer->state->x);
  USART_puts("\t");
  USART_putFloat(localizer->state->y);
  USART_puts("\n");
   
  USART_puts("Angle: ");
  USART_putFloat(localizer->state->theta);
  USART_puts("\n");
  USART_puts("Vel: ");
  USART_putFloat(localizer->state->vel);
  USART_puts("\n");
}

void loop(void) {
  static int i = 1; 
  //doLog();

  if(i % 30)
    sendGuesses();

  if(i++ & 0x1)
    enableLEDs(BLUE);
  else 
    disableLEDs(BLUE);
}

void tick_loop(void){
  static int loopCount = 0;
  localizer->update(localizer);
  motors->updateOffset(motors, localizer->_state->theta);
  if(running){
  //  doMotion(); 
  /*
  if(loopCount == 0){
    localizer->update(localizer);
    pid->loop(pid, targState, localizer->_state);
    loopCount = 10; 
  }
  motors->doMotorPID(motors); 
  loopCount--;
  */
  }
  else{
    motors->setOffset(motors, PWM_MIN);
    motors->setSpeeds(motors, 0,0);
  }
}
