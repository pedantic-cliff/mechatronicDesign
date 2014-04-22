#include "map.h"
#include "usart.h"


#define NROWS 9 
#define NCOLS 9
#define RMAX  (NROWS-1)
#define NCLASSES 4

typedef struct {
  confidences_t conf; 
  unsigned int count;
} cell_t;

cell_t grid[NROWS][NCOLS];

struct { 
  char numMeas;
  char numDefect;
  char cells[NROWS][NROWS];
} sendBuff; 

int guessCell(int x, int y){
  int minIdx = 0; 
  y = RMAX - y; 
  pConfidences conf = &grid[y][x].conf;
  int minVal = conf->metal;
  if (minVal > conf->yellow){
    minIdx = 1; 
    minVal = conf->yellow;
  }
  if (minVal > conf->boundary){
    minIdx = 2; 
    minVal = conf->boundary;
  }
  return minIdx;
}

void sendGuesses(void){
  int x, y, i;
  volatile char buff[84];
  sendBuff.numMeas = 0; 
  sendBuff.numDefect = 0; 

  for(y = 0; y < NROWS; y++){
    for(x = 0; x < NCOLS; x++){
      if(grid[y][x].count > 0)
        sendBuff.numMeas++; 
      else{
        sendBuff.cells[y][x] = 2; 
        continue;
      }
      
      sendBuff.cells[y][x] = guessCell(x,y); 
      if(sendBuff.cells[y][x])
        sendBuff.numDefect++; 
    }
  }
  buff[0] = sendBuff.numMeas;
  buff[1] = sendBuff.numDefect;
  i = 2;
  for(y = 0; y < NROWS; y++){
    for(x = 0; x < NCOLS; x++){
      buff[i++] = sendBuff.cells[y][x]; 
    }
  }
  buff[83] = 0;
  USART_write(buff, 83);
}

void applyConfidence(int x, int y, pConfidences pConf){
  y = RMAX - y; 
    grid[y][x].count++;
  pConfidences confs = &grid[y][x].conf;
  confs->metal += pConf->metal; 
  confs->yellow += pConf->yellow; 
  confs->boundary += pConf->boundary; 
  
  USART_puts("Grid: ");
  USART_putInt(x);
  USART_puts(", ");
  USART_putInt(y);
  USART_puts(": ");
  USART_putFloat(conf->metal);
  USART_puts(", ");
  USART_putFloat(conf->yellow);
  USART_puts(", ");
  USART_putFloat(conf->boundary);
  USART_puts("\n");

}

int fakes[] = { 0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,1 };

void _fakepoint(int x,int y,int val){
  grid[y][x].count++;
  if(val) 
    grid[y][x].conf.metal = 1.0f;
  else
    grid[y][x].conf.yellow = 1.0f;
}
void fakeData(void){
  int i,j,k=0;; 
  for(i = 0; i < NROWS; i++){
    for(j = 0; j < NCOLS; j++){
      _fakepoint(i,j,fakes[k++]);
    }
  }
}

void createGrid(void){
  int i,j;
  pConfidences conf;
  for(i = 0; i < NROWS; i++){
    for(j = 0; j < NCOLS; j++){
      conf = &grid[i][j].conf; 
      conf->metal    = 0.f;
      conf->yellow   = 0.f;
      conf->boundary = 0.f;
      grid[i][j].count = 0; 
    }
  }
  //fakeData();
}

