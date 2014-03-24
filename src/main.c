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
  //colorSensors = createColorSensors(); 
  //colorSensors->init(colorSensors); 
  accel   = initAccel(); 
  motors  = createMotors(); 
  delay(500);
  motors->setSpeeds(0x8000,0x8000);
  localizer = createLocalizer(motors, accel);
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

void loop() {
  static int i = 0; 
  USART_puts("Loop\n\r");
  doLocalize();
  doLog();

  delay(500);
  if(i++ & 0x1)
    enableLEDs(RED);
  else 
    disableLEDs(RED);
}
