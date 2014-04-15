#include "map.h"


#define NROWS 9 
#define NCOLS 9
#define RMAX  (NROWS-1)
#define NCLASSES 4

typedef struct {
  confidences_t conf; 
  unsigned int count;
} cell_t;

cell_t grid[NROW][NCOLS];

struct { 
  char numMeas;
  char numDefect;
  char cells[NROWS][NROWS];
} sendBuff; 

int guessCell(int x, int y){
  int minIdx = 0; 
  y = RMAX - y; 
  pConfidences conf = grid[y][x].conf;
  int minVal = conf.metal;
  if (minVal > conf.yellow){
    minIdx = 1; 
    minVal = conf.yellow;
  }
  if (minVal > conf.boundary){
    minIdx = 2; 
    minVal = conf.boundary;
  }
  return minIdx;
}

void sendGuesses(void){
  int x, y; 
  sendBuff.numMeas = 0; 
  sendBuff.numDefect = 0; 

  for(y = 0; y < NROWS; y++){
    for(x = 0; x < NCOLS; x++){
      if(grid[y][x].count > 0)
        sendBuff.numMeas++; 
      else{
        sendBuff.cells[y][x] = guessCell(x,y); 
        continue;
      }
      
      sendBuff.cells[y][x] = guessCell(x,y); 
      if(sendBuff.cells[y][x])
        sendBuff.numDefect++; 
    }
  }
  USART_sendByte(sendBuff.numMeas);
  USART_sendByte(sendBuff.numDefect);
  for(y = 0; y < NROWS; y++){
    for(x = 0; x < NCOLS; x++){
      USART_sendByte(sendBuff.cells[y][x]); 
    }
  }
  USART_sendByte(0xFF);
}

void applyConfidence(int x, int y, confidences_t conf){
  y = RMAX - y; 
  grid[y][x][0] += conf.metal; 
  grid[y][x][1] += conf.yellow; 
  grid[y][x][2] += conf.boundary; 
}

void createGrid(void){
  int i,j;
  pConfidence conf;
  fr(i = 0; i < NROWS; i++){
    for(j = 0; j < NCOLS; j++){
        conf = grid[i][j].conf; 
        conf.metal    = 0.f;
        conf.yellow   = 0.f;
        conf.boundary = 0.f;
        grid[i][j].count = 0; 
      }
    }
  }
}

