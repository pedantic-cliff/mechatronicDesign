#include "main.h"
#include "utils.h"
#include "usart.h"
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
  delay(2000);
  enableLEDs(RED);
  colorSensors->measureColor(colorSensors,RED); 
//  while(!(colorSensors->done)); 
  volatile int *results = colorSensors->getResult();
  for(i = 0; i < NUM_SENSORS; i++){
    USART_putInt(results[i]); 
    USART_puts("\t");
  }
  USART_puts("\n\r");
  disableLEDs(RED|BLUE);
}

