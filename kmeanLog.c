#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>  /* String function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

char buff1[4]; 
union { 
  char bytes[32]; 
  struct {
    float r;
    float b; 
    char id;
    char sen; 
    char guess;
  } vals; 
} buff;

int portFd; 

void main(int argc, char *argv[]){
  int count = 0;
  printf("Open %s\n", argv[1]);
  portFd = open(argv[1],  O_RDWR | O_NOCTTY | O_NDELAY);
  if(portFd == -1){
    printf("Whoops, couldn't open the port!\n");
    return ;
  }
  fcntl(portFd, F_SETFL, 0);
  buff1[0] = 0xFF; 
  buff1[1] = 42; 
  buff1[2] = 1;
  write(portFd,buff1,3);

  count = 0; 
  while(count < 2){
    count += read(portFd, buff1 + count, 2 - count);
  }
  printf("Got Echo: 0x%x 0x%x 0x%x\n", buff1[0], buff1[1], buff1[2]);

  while(1){
    count = 0; 
    while(count < 11){
      count += read(portFd, buff.bytes + count, 11 - count);
    }
    printf("%d\t%d\t%f\t%f\t%d\n", buff.vals.id, buff.vals.sen, buff.vals.r, 
                                       buff.vals.b, buff.vals.guess); 
  }
  close(portFd);
}
