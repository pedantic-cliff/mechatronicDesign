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
  pConfidences conf = &grid[y][x].conf;
  float minVal = conf->metal;
  if (minVal > conf->yellow){
    minIdx = 1; 
    minVal = conf->yellow;
  }
  if (minVal > conf->boundary){
    minIdx = 2; 
    minVal = conf->boundary;
  }
  
  USART_puts("Conf: "); 
  USART_putFloat(grid[y][x].conf.metal);
  USART_puts("\t\t");
  USART_putFloat(conf->yellow);
  USART_puts("\t\t");
  USART_putFloat(conf->boundary);
  USART_puts("\n");
  return minIdx;
}

void sendGuesses(void){
  int x, y, i;
  volatile char buff[84];
  sendBuff.numMeas = 0; 
  sendBuff.numDefect = 0; 
  USART_puts("Send Guesses\n");
  for(y = 0; y < NROWS; y++){
    for(x = 0; x < NCOLS; x++){
      if(grid[y][x].count > 0){
        sendBuff.numMeas++; 
      }else{
        sendBuff.cells[y][x] = 2; 
        continue;
      }
      
      USART_puts("Guess\n");
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
      buff[i] = sendBuff.cells[y][x]; 
      i++;
    }
  }
  buff[83] = 0;
  USART_write(buff, 83);
}

void applyConfidence(int x, int y, pConfidences pConf){
  y = RMAX - y; 
  if ( x < 0 || x >= RMAX || y < 0 || y >= RMAX)
    return;

  grid[y][x].count++;
  pConfidences confs = &grid[y][x].conf;
  confs->metal += pConf->metal; 
  confs->yellow += pConf->yellow; 
  confs->boundary += pConf->boundary; 
  
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

