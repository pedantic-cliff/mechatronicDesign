#include "main.h"
#include "utils.h"
#include "usart.h"
#include "colorSensor.h"

ColorSensors colorSensors; 
int main(void) {
  int i = -1000; 
  init();
  pid_pos(500); 
  do {
    loop();
  } while (1);
}

void init() {
  init_USART(); 
  colorSensors = createColorSensors(); 
  colorSensors->init(colorSensors); 
}


void loop() {
  delay(2000);
  colorSensors->measureColor(colorSensors,RED); 
  USART_putInt(colorSensors->sensors[0]->measurements[RED_IDX]); 
  USART_puts("\n\r"); 
}

