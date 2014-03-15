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
  initEncoders();
  //colorSensors = createColorSensors(); 
  //colorSensors->init(colorSensors); 
}

void loop() {
  static int i = 0; 
  delay(500);
  USART_putInt(getLeftCount());
  USART_puts("\t"); 
  USART_putInt(getRightCount());
  USART_puts("\n\r"); 
  if(i++ & 0x1)
    enableLEDs(RED);
  else 
    disableLEDs(RED);
}

