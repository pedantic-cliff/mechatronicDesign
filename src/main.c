#include "main.h"
#include "utils.h"
#include "usart.h"
#include "accel.h"
#include "colorSensor.h"
#include "motors.h"
#include "misc.h"
#include <stdio.h>
#include "math.h"

ColorSensors colorSensors; 
Accel accel; 
Motors motors; 

int main(void) {
  delay(500); // Give the hardware time to warm up on cold start
  init();
  do {
    loop();
  } while (1);
}

void init() {
  init_USART(); 
  initLEDs();
  //initEncoders();
  //colorSensors = createColorSensors(); 
  //colorSensors->init(colorSensors); 
  accel   = initAccel(); 
  motors  = createMotors(); 
  motors->setSpeeds(0x8000, 0x4000);
}

void doColors(void){
  int ii;
  colorSensors->measureColor(colorSensors,RED); 
  while(!colorSensors->done); 
  volatile uint16_t* res = colorSensors->getResult(); 
  USART_puts("Light Sensors: "); 
  for(ii = 0; ii < NUM_SENSORS; ii++){
    USART_putInt(res[ii]); 
    USART_puts("\t"); 
  }
  USART_puts("\n\r");
}

void doAccel(void){
  int8_t x = accel->getX(),
         y = accel->getY();
  int theta = accel->getAngle();
  USART_puts("Angle: ");
  USART_putInt(theta);
  USART_puts("\n\r");  
  disableLEDs(RED|BLUE|GREEN|ORANGE);
  if (x > 30)
    enableLEDs(RED);
  if (y > 30) 
    enableLEDs(ORANGE);
  if (y < -30) 
    enableLEDs(BLUE);
  if (x < -30) 
    enableLEDs(GREEN);
}

void loop() {
  static int i = 0; 
  delay(500);
  
  if(i++ & 0x1)
    enableLEDs(RED);
  else 
    disableLEDs(RED);
}
