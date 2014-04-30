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

cell_t Grid[NROWS][NCOLS];

struct { 
  char numMeas;
  char numDefect;
  char cells[NROWS][NROWS];
} sendBuff; 

int guessCell(int x, int y){
  int minIdx = 0; 
  pConfidences conf = &Grid[y][x].conf;
  if(conf->yellow > conf->metal)
    return 1;
  return 0; 
}

void finishGrid(void){
  int x, y;
  for(y = 0; y < NROWS; y++){
    for(x = 0; x < NCOLS; x++){
      if(Grid[y][x].count == 0){
        Grid[y][x].count++;
        Grid[y][x].conf.yellow = 0.f;
        Grid[y][x].conf.metal  = 10.f;
      }
    }
  }
}

void sendGuesses(void){
  int x, y, i;
  volatile char buff[84];
  sendBuff.numMeas = 0; 
  sendBuff.numDefect = 0; 
  for(y = 0; y < NROWS; y++){
    for(x = 0; x < NCOLS; x++){
      if(Grid[y][x].count > 0){
        sendBuff.numMeas++; 
      }else{
        sendBuff.cells[y][x] = 2; 
        continue;
      }
      sendBuff.cells[y][x] = guessCell(x,y); //(Grid[y][x].conf.yellow);
      if(sendBuff.cells[y][x])
        sendBuff.numDefect++; 
    }
  }
  buff[0] = sendBuff.numMeas;
  buff[1] = sendBuff.numDefect;
  i = 2;
  for(y = 0; y < NROWS; y++){
    for(x = 0; x < NCOLS; x++){
      buff[i] = sendBuff.cells[y][x]; 
      i++;
    }
  }
  buff[83] = 0;
  USART_write(buff, 83);
}

void applyConfidence(int x, int y, pConfidences pConf){
  y = RMAX - y; 
  if ( x < 0 || x > RMAX || y < 0 || y > RMAX)
    return;
  Grid[y][x].count++;
  Grid[y][x].conf.metal     += pConf->metal;
  Grid[y][x].conf.yellow    += pConf->yellow;

  /*
  USART_puts("Grid: ");
  USART_putFloat(confs->metal) ;
  USART_puts("\t");
  USART_putFloat(pConf->yellow) ;
  USART_puts("\t");
  USART_putFloat(pConf->boundary) ;
  USART_puts("\n");
  */
  
}

int fakes[] = { 0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,1 };

void _fakepoint(int x,int y,int val){
  Grid[y][x].count++;
  if(val) 
    Grid[y][x].conf.metal = 1.0f;
  else
    Grid[y][x].conf.yellow = 1.0f;
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
      conf = &(Grid[i][j].conf); 
      conf->metal    = 0.f;
      conf->yellow   = 0.f;
      conf->boundary = 0.f;
      Grid[i][j].count = 0; 
    }
  }
  //fakeData();
}

