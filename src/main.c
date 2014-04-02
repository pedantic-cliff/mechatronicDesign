#include "main.h"
#include "utils.h"
#include "usart.h"
#include "accel.h"
#include "colorSensor.h"
#include "localize.h"
#include "motors.h"
#include "misc.h"
#include <stdio.h>
#include "math.h"

ColorSensors colorSensors; 
Accel accel; 
Motors motors; 
Localizer localizer;

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
  colorSensors = createColorSensors(); 
  accel   = initAccel(); 
  motors  = createMotors(); 
  delay(500);
//  motors->setSpeeds(0x8000,0x8000);
  localizer = createLocalizer(motors, accel);
}

int doColor(Color c){
  int ii;
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
  USART_putFloat(localizer->Rw.x);
  USART_puts("\t");
  USART_putFloat(localizer->Rw.y);
  USART_puts("\n\r");

  USART_puts("Angle: ");
  USART_putFloat(accel->getAngle());
  USART_puts("\n\r");

}
void doColors(void){
  int n,r,g,b,min;
  n = doColor(NONE);
  r = doColor(RED)   - n;
  g = doColor(GREEN) - n;
  b = doColor(BLUE)  - n;

  colorSensors->guessColor(r,g,b);

  
  USART_putInt(n);
  USART_puts("\t");
  USART_putInt(r);
  USART_puts("\t");
  USART_putInt(g);
  USART_puts("\t");
  USART_putInt(b);
  USART_puts("\n\r");
  
}

void loop() {
  static int i = 0; 
  doColors();
  //doLocalize();
  //doLog();
  if(i++ & 0x1)
    enableLEDs(BLUE);
  else 
    disableLEDs(BLUE);
}
