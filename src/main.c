#include "main.h"
#include "misc.h"
#include <stdio.h>
#include "math.h"

static volatile int running = 0;


ColorSensors colorSensors; 
Accel accel; 
Motors motors; 

Localizer localizer;

state_t _targState; 
State targState;

Pid pid; 
PID_Gains angleGains  = { 0.0f, 0.0f, 0.0f },
          distGains   = { 0.0f, 0.0f, 0.0f },
          bearGains   = { 0.0f, 0.0f, 0.0f };
static void init(void);
static void loop(void);

void start(void){
  localizer->restart(localizer);
  running = 1;
}
void halt(void){
  running = 0;
}

int main(void) {
  delay(500); // Give the hardware time to warm up on cold start
  init();
  start();
  do {
    if(running)
      loop();
    delay(300);
  } while (1);
}

static void init() {
  static PID_Gains angleGains = { 0.0f, 0.0f, 0.0f },
                   posGains   = { 0.0f, 0.0f, 0.0f },
                   velGains   = { 0.0f, 0.0f, 0.0f };
  targState = &_targState;
  init_USART(); 
  initLEDs();
  colorSensors = createColorSensors(); 
  accel   = initAccel(); 
  motors  = createMotors(); 
  delay(500);
  localizer = createLocalizer(motors, accel);
  pid = createPID(angleGains, posGains, velGains, motors); 
  //USART_puts("Init finished \r\n");
}

int doColor(Color c){
  colorSensors->measureColor(colorSensors,c); 
  while(colorSensors->done < COLOR_SENSOR_ITERS); 
  volatile uint16_t* res = colorSensors->getResult(); 
  return res[0];
}

void doLocalize(void){
  localizer->update(localizer);
}

void doLog(void){
  USART_puts("State Vector: [");
  USART_puts("\n\r");
  
  USART_puts("Encoders: ");
  USART_putInt(motors->getLeftCount());
  USART_puts("\t");
  USART_putInt(motors->getRightCount());
  USART_puts("\n\r");


  USART_puts("x, y: ");
  USART_putFloat(localizer->state->x);
  USART_puts("\t");
  USART_putFloat(localizer->state->y);
  USART_puts("\n\r");
   
  USART_puts("Angle: ");
  USART_putFloat(localizer->state->theta);
  USART_puts("\n\r");
  USART_puts("Vel: ");
  USART_putFloat(localizer->state->vel);
  USART_puts("\n\r");
}

void doColors(void){
  int n,r,g,b,min;
  n = doColor(NONE);
  r = doColor(RED);
  g = doColor(GREEN);
  b = doColor(BLUE);

  min = n;
  min = min < r ? min: r;
  min = min < b ? min: b;
  min = min < g ? min: g;

  USART_putInt(n - min);
  USART_puts("\t");
  USART_putInt(r - min);
  USART_puts("\t");
  USART_putInt(g - min);
  USART_puts("\t");
  USART_putInt(b - min);
  USART_puts("\n\r");
}

void doPID(void){
  pid->loop(pid, targState, localizer->state);
}

static void loop() {
  static int i = 0; 
  //doColors();
  doLocalize();
  doPID();
  //doLog();
  if(i++ & 0x1)
    enableLEDs(BLUE);
  else 
    disableLEDs(BLUE);
}


