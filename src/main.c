#include "main.h"
#include "misc.h"
#include <stdio.h>
#include "math.h"

volatile int running = 0;

ColorSensors colorSensors; 
Accel accel; 
Motors motors; 

Localizer localizer;

state_t _targStates[] = {
                         {0.f,   24.f,    PI/2,      0.f}
}; 
int numStates = sizeof(_targStates)/sizeof(state_t);
int currentState = 0; 
State targState;

Pid pid; 
PID_Gains angleGains  = { 35.f, 0.00f, 0.0f },
          distGains   = { 40.f, 0.00f, 0.0f },
          bearGains   = { -20.f, 0.00f, 0.0f },
          motorGains  = { 100.f, 0.00f, 50.0f };

static void init(void);
static void loop(void);

long time;
void start(void){
  localizer->restart(localizer);
  currentState = 0; 
  targState = &_targStates[currentState];
  running = 1;
  time = getCurrentTime();
}

void halt(void){
  running = 0;
  motors->setSpeeds(motors, 0,0);
}

int main(void) {
  initSysTick(); 
  delay(500); // Give the hardware time to warm up on cold start
  init();
  do {
    if(running)
      loop();
    else
      delay(500);
  } while (1);
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

int doColor(Color c){
  colorSensors->measureColor(colorSensors,c); 
  while(colorSensors->done < COLOR_SENSOR_ITERS); 
  volatile uint16_t* res = colorSensors->getResult(); 
  return res[0];
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

void doCalibrateColors(){
  USART_puts("NONE:\t\t");
  colorSensors->calibrateColor(colorSensors, NONE);
  USART_puts("RED:\t\t");
  colorSensors->calibrateColor(colorSensors, RED);
  USART_puts("GREEN:\t\t");
  colorSensors->calibrateColor(colorSensors, GREEN);
  USART_puts("BLUE:\t\t");
  colorSensors->calibrateColor(colorSensors, BLUE);
  USART_puts("\r\n\r\n");
}

void doUpdateState(void){
  __disable_irq();
  localizer->cacheState(localizer);
  __enable_irq();
}

int checkStateDone(void){
  return ( (fabsf(targState->x - localizer->state->x) < 0.7f)
//      && ( fabsf(targState->y - localizer->state->y) < 0.2f)
      && ( fabsf(targState->theta - localizer->state->theta) < 0.5f) );
}

void loop(void) {
  static int i = 0; 
  doLog();
  doUpdateState();
  if(localizer->state->x < targState->x){
    motors->setSpeeds(motors,10,10);
    delay(100);
  }else{
    motors->setSpeeds(motors, 0x0, 0x0);
    delay(1000);
  }
  delay(1000);
  if(i++ & 0x1)
    enableLEDs(BLUE);
  else 
    disableLEDs(BLUE);
}

void tick_loop(void){
  static int loopCount = 0;
  localizer->update(localizer);
  /*if(loopCount == 0){
    localizer->update(localizer);
    pid->loop(pid, targState, localizer->_state);
    loopCount = 10; 
  }
  motors->doMotorPID(motors); 
  loopCount--;
  */
}
