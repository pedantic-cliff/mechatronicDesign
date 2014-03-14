#ifndef _COLOR_SENSOR_H_
#define _COLOR_SENSOR_H_

#include "stm32f4xx_dma.h"
#include "stm32f4xx_adc.h"

#define LIGHT_PORT  GPIOE
#define RED_PIN     GPIO_Pin_10
#define GREEN_PIN   GPIO_Pin_11
#define BLUE_PIN    GPIO_Pin_12
#define ALL_LIGHTS ( RED_PIN | GREEN_PIN | BLUE_PIN )
#define NUM_COLORS  4

#define NONE_IDX    0
#define RED_IDX     1
#define GREEN_IDX   2
#define BLUE_IDX    3

#define RED_MASK (0x01)
#define BLUE_MASK (0x02)
#define GREEN_MASK (0x04)
#define NONE_MASK (0x08)
#define RED_VALID(sensor) (!!(sensor->validColors & RED_MASK))
#define GREEN_VALID(sensor) (!!(sensor->validColors & BLUE_MASK))
#define BLUE_VALID(sensor) (!!(sensor->validColors & GREEN_MASK))
#define NONE_VALID(sensor) (!!(sensor->validColors & NONE_MASK))

typedef enum { NONE, RED, GREEN, BLUE } COLORS; 

#define NUM_SENSORS 1
#define SENSOR_PORT GPIOD

#define ADC3_DR_ADDRESS ((uint32_t)0x4001224C)

typedef struct lightSensor_t *LightSensor;
typedef struct lightSensor_t{
 uint16_t   In_Pin;
 char       validColors; 
 uint16_t   measurements[NUM_COLORS]; 
} lightSensor; 



typedef struct colorSensors_t *ColorSensors; 
typedef struct colorSensors_t {
  LightSensor sensors; 

  void (*init)  (ColorSensors sensors);

  void (*measureRed)  (ColorSensors sensors); 
  void (*measureGreen)(ColorSensors sensors); 
  void (*measureBlue) (ColorSensors sensors); 
  void (*measureNone) (ColorSensors sensors); 
} colorSensor;

ColorSensors createColorSensors(void); 

#endif // _COLOR_SENSOR_H_
