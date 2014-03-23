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
  //initEncoders();
  //colorSensors = createColorSensors(); 
  //colorSensors->init(colorSensors); 
  accel   = initAccel(); 
  motors  = createMotors(); 

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
  USART_putFloat(localizer->Rw.x);
  USART_puts(",\t");
  USART_putFloat(localizer->Rw.y);
  USART_puts(",\t");
  USART_putFloat(localizer->Rw.theta);
  USART_puts("\n\r");

}

void loop() {
  static int i = 0; 
  doLocalize();

  if(i++ & 0x1)
    enableLEDs(RED);
  else 
    disableLEDs(RED);
  doLog();
}
