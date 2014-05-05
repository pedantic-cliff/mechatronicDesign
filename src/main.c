#include "main.h"
#include "misc.h"
#include <stdio.h>
#include "math.h"
#include "state.h"

typedef enum { NOP, FORWARD, LEFT } opType;

typedef struct {
  opType command; 
  float  argument; 
  int    delay;  
} operation; 

#define PAUSE 350
operation Commands[] = { 
  { NOP,      0.f,   PAUSE },
  { FORWARD,  6.0f,  PAUSE },  // Right 
  { FORWARD, 10.0f,  PAUSE },  // Right 
  { FORWARD, 22.0f,  PAUSE },  // Right 
  { FORWARD, 26.0f,  PAUSE },  // Right 
  { FORWARD, 29.f,  PAUSE },  // Right
  { LEFT,     5.f,   PAUSE },
  { FORWARD, 10.0f,  PAUSE },  // Up
  { FORWARD, 22.0f,  PAUSE },  // Up
  { FORWARD, 26.0f,  PAUSE },  // Up
  { FORWARD, 31.0f,  PAUSE },  // Up
  { LEFT,     5.f,   PAUSE },  
  { FORWARD, 26.f,   PAUSE },  // Left
  { FORWARD, 13.5f,   PAUSE },  // Left
  { FORWARD, 10.f,   PAUSE },  // Left
  { FORWARD,  6.5f,  PAUSE },  // Left
  { LEFT,     0.f, 2*PAUSE },
  { FORWARD, 26.f,   PAUSE },  // Down
  { FORWARD, 22.f,   PAUSE },  // Down
  { FORWARD, 18.f,   PAUSE },  // Down
  { LEFT,     0.f,   PAUSE },
  { FORWARD, 18.f,   PAUSE },  // Right
  { FORWARD, 22.f,   PAUSE },  // Right
  { FORWARD, 22.f,   PAUSE },  // Right
  { NOP,      0.f,   PAUSE}, 
};

const int numCommands = sizeof(Commands)/sizeof(operation); 
int currentCommandIndex = 0; 
int timeout = 0; 
volatile int running = 0;
volatile int ready   = 0;
int motionDone = 0; 

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
#ifdef DEBUG
  /*
  USART_puts("[");
  USART_putFloat(localizer->state->x);
  USART_puts(", ");
  USART_putFloat(localizer->state->y);
  USART_puts(", ");
  USART_putFloat(localizer->state->theta);
  USART_puts("]\n");
  */
#endif
  __enable_irq();
}

long time;
void start(void){
  running = 1;
  USART_puts("Start!\n");
  colorSensors->startColor(NONE);
}

void halt(void){
  running = 0;
  colorSensors->halt();
}

void doLog();

void doColorCalibrate(void){
  colorSensors->calibrateColors(colorSensors); 
}

void startCommand(int index){
  switch(Commands[index].command){
    case FORWARD:
      goForwardBy(Commands[index].argument);
      break;
    case LEFT:
      turnLeft90();
      break;
    case NOP:
    default: 
      doStall();
      break;
  }
}
void endCommand(int index){
  switch(Commands[index].command){
    case FORWARD:
      colorSensors->startColor(NONE);
      delay(Commands[index].delay);
      colorSensors->halt();
#ifndef DEBUG
      __disable_irq();
      sendGuesses();
      __enable_irq();
#endif 
      break;
    default:
      delay(Commands[index].delay);
  }

}

int main(void) {
  initSysTick(); 
  delay_blocking(500); // Give the hardware time to warm up on cold start
  init();
  delay(1000); 
  startCommand(currentCommandIndex);
#ifdef DEBUG
  start();
#endif 
  do {
    doUpdateState();
    if(calibrateColor){
      doColorCalibrate();
      calibrateColor = 0; 
    }
    if( (isMotionComplete() && !motionDone ) ){
      endCommand(currentCommandIndex++);
      motionDone = 1; 
      if (currentCommandIndex < numCommands){ 
        startCommand(currentCommandIndex);
        motionDone = 0;
      }else{
        finishGrid();
        __disable_irq();
        sendGuesses();
        __enable_irq();
      }
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
  initLEDs();
  init_USART(); 
  createGrid();
  colorSensors = createColorSensors(); 
  accel   = initAccel(); 
  motors  = createMotors();
  delay_blocking(500);
  localizer = createLocalizer(motors, accel);
  startState();
  ready = 1; 
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
    doMotion(); 
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
    motors->setOffset(motors, PWM_MIN_L, PWM_MIN_R);
    motors->setSpeeds(motors, 0,0);
  }
}
