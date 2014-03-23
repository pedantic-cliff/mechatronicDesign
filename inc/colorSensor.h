#ifndef _COLOR_SENSOR_H_
#define _COLOR_SENSOR_H_

#include "stm32f4xx_dma.h"
#include "stm32f4xx_adc.h"
#include "utils.h"

#define NUM_COLORS  4

#define NONE_IDX    0
#define RED_IDX     1
#define GREEN_IDX   2
#define BLUE_IDX    3

#define RED_VALID(sensor) (!!(sensor->validColors & RED))
#define GREEN_VALID(sensor) (!!(sensor->validColors & BLUE))
#define BLUE_VALID(sensor) (!!(sensor->validColors & GREEN))
#define NONE_VALID(sensor) (!!(sensor->validColors & NONE))

#define NUM_SENSORS 6

#define ADC1_DR_ADDRESS ((uint32_t)0x40012000 + 0x000 + 0x4C)

typedef struct lightSensor_t *LightSensor;
typedef struct lightSensor_t{
  uint16_t      measurements[NUM_COLORS]; 
} lightSensor; 



typedef struct colorSensors_t *ColorSensors; 
typedef struct colorSensors_t {
  LightSensor sensors[NUM_SENSORS]; 
  void (*init)  (ColorSensors sensors);

  void (*measureColor) (ColorSensors sensors, Color color); 
  volatile uint16_t* (*getResult) (void); 
  volatile int done; 
} colorSensor;

ColorSensors createColorSensors(void); 

#endif // _COLOR_SENSOR_H_

