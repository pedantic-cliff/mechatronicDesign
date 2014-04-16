#include "colorSensor.h"
#include "localize.h"
#include "map.h"

#define RED_PIN     GPIO_Pin_9
#define GREEN_PIN   GPIO_Pin_10
#define BLUE_PIN    GPIO_Pin_11

typedef struct centroid{ 
  float r; 
  float g; 
  float b;
} centroid_t; 

static struct centroid edge    = { 101.0463f,  243.8729f,  280.8654f };
static struct centroid metal   = { 1172.3f,    1740.8f,   1944.7f    };
static struct centroid yellow  = { 2274.5f,    621.7f,    1933.0f    };
static struct centroid white   = { 2239.8f,    2219.5f,   2158.0f    };

static struct centroid *centroids[4]; 

void doColor(void){
  int red = 0, green = 0, blue = 0, ambient = 0;
  int i; 
  delay(100);

  for(i = 0; i < 4; i++){
    ambient += analogRead(SENSOR_PIN);
    delay(10);
  }

  digitalWrite(RED_PIN,HIGH);
  delay(100);
  for(i = 0; i < 4; i++){
    red += analogRead(SENSOR_PIN);
    delay(10);
  }

  digitalWrite(GREEN_PIN,HIGH);
  delay(100);
  for(i = 0; i < 4; i++){
    green += analogRead(SENSOR_PIN);
    delay(10);
  }

  digitalWrite(BLUE_PIN,HIGH);
  delay(100);
  for(i = 0; i < 4; i++){
    blue += analogRead(SENSOR_PIN);
    delay(10);
  }

}

float calcCentDiff(int r, int g, int b, centroid_t *cent){
  float score = (cent->r - r)*(cent->r - r) 
    + (cent->g - g)*(cent->g - g) 
    + (cent->b - b)*(cent->b - b);
  return score;
}

void guessColor(pConfidences c, int r, int g, int b){
  int i = 0; 
  int minIdx = 0; 
  
  c->boundary = calcCentDiff(r,g,b,centroids[0]); 
  c->metal    = calcCentDiff(r,g,b,centroids[1]); 
  c->yellow   = calcCentDiff(r,g,b,centroids[2]); 
}


