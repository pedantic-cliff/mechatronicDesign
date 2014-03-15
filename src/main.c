#include "main.h"
#include "utils.h"
#include "usart.h"
#include "accel.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include <stdio.h>
#include "math.h"
#include "colorSensor.h"

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
  colorSensors = createColorSensors(); 
  colorSensors->init(colorSensors); 
}

void loop() {
  int i = 0; 
  delay(1);
  enableLEDs(RED);
  delay(1);
  disableLEDs(RED|BLUE);
}

