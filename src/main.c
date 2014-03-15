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
int main(void) {
  init();
  do {
    loop();
  } while (1);
}

void init() {
  init_USART(); 
  initLEDs();
  //initEncoders();
  colorSensors = createColorSensors(); 
  colorSensors->init(colorSensors); 
}

void loop() {
  static int i = 0; 
  int ii;
  delay(500);
  colorSensors->measureColor(colorSensors,RED); 
  while(!colorSensors->done); 
  volatile uint16_t* res = colorSensors->getResult(); 
  USART_puts("Light Sensors: "); 
  for(ii = 0; ii < NUM_SENSORS; ii++){
    USART_putInt(res[ii]); 
    USART_puts("\t"); 
  }
  USART_puts("\n\r");
  if(i++ & 0x1)
    enableLEDs(RED);
  else 
    disableLEDs(RED);
}

