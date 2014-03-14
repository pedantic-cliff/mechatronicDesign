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

u16 readADC1(u8 channel) { 
  ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_15Cycles); 
  // Start the conversion 
  ADC_SoftwareStartConv(ADC1); 
  // Wait until conversion completion
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET); 
  // Get the conversion value 
  return ADC_GetConversionValue(ADC1); 
}


void loop() {
  delay(2000);
  colorSensors->measureColor(colorSensors,RED); 
  USART_putInt(readADC1(ADC_Channel_14)); 
  USART_puts("\n\r"); 
}

