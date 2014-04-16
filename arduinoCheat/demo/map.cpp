#include "map.h"
#include "Arduino.h"
#define NROWS 9 
#define NCOLS 9
#define RMAX  (NROWS-1)
#define NCLASSES 4

typedef struct {
  confidences_t conf; 
  unsigned int count;
} 
cell_t;

cell_t grid[NROWS*NCOLS];

char cells[NROWS*NROWS];
char numMeas;
char numDefect;


int guessCell(int x, int y){
  int minIdx = 0; 
  y = RMAX - y; 
  confidences_t conf = grid[y*9 + x].conf;
  int minVal = grid[y * 9 + x].conf.metal;
  if (minVal > grid[y * 9 + x].conf.yellow){
    minIdx = 1; 
    minVal = grid[y*9 + x].conf.yellow;
  }
  if (minVal > grid[y*9 + x].conf.boundary){
    minIdx = 2; 
    minVal = grid[y*9 + x].conf.boundary;
  }
  return minIdx;
}

void sendGuesses(void){
  int x, y; 
  numMeas = 0; 
  numDefect = 0; 

  for(y = 0; y < NROWS; y++){
    for(x = 0; x < NCOLS; x++){
      /*if(grid[y*9+x].count > 0)
        numMeas++; 
      else{
        cells[y*9+x] = 2; 
        continue;
      }*/

      //cells[y*9+x] = guessCell(x,y); 
      if(cells[y*9+x])
        numDefect++; 
    }
  }
  
/*
   Serial.print(sendBuff.numMeas);
   delay(10);
   Serial.print('\t');
   delay(10);
   Serial.println(sendBuff.numDefect);
   for(y = 0; y < NROWS; y++){
   for(x = 0; x < NCOLS; x++){
   Serial.print(sendBuff.cells[y*9+x]);
   delay(10);
   Serial.print('\t');
   delay(10);
   }
   Serial.print('\n');
   delay(10);
   }
   Serial.print(0xFF);
  */ 
}

void applyConfidence(int x, int y, confidences_t conf){
  y = RMAX - y; 

  grid[y*9+x].conf.metal += conf.metal; 
  grid[y*9+x].conf.yellow += conf.yellow; 
  grid[y*9+x].conf.boundary += conf.boundary; 
}

int fakes[] = { 
  0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,1 };

void _fakepoint(int x,int y,int val){
  grid[y*9+x].count++;
  if(val) 
    grid[y*9+x].conf.metal = 1.0f;
  else
    grid[y*9+x].conf.yellow = 1.0f;
}
void fakeData(void){
  int i,j,k=0;
  ; 
  for(i = 0; i < NROWS; i++){
    for(j = 0; j < NCOLS; j++){
      _fakepoint(i,j,fakes[k++]);
    }
  }
}

void createGrid(void){
  int i,j;
  for(i = 0; i < NROWS; i++){
    for(j = 0; j < NCOLS; j++){
      grid[i*9+j].conf.metal    = 0.f;
      grid[i*9+j].conf.yellow   = 0.f;
      grid[i*9+j].conf.boundary = 0.f;
      grid[i*9+j].count = 0; 
    }
  }
  fakeData();
}


