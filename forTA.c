#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>  /* String function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

char buff1[4]; 
char buff2[84];
int portFd; 

void main(void){
  int count = 0;
  portFd = open("/dev/ttyACM0",  O_RDWR | O_NOCTTY | O_NDELAY);
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
    while(count < 83){
      count += read(portFd, buff2 + count, 83 - count);
    }
    printf("Got Header bytes: %d, %d\n", buff2[0], buff2[1]);
    printf("Got Map: ");
    for(count = 0; count < 81; count++){
      if(count % 9 == 0)
        printf("\n");
      printf("%d ", buff2[count + 2]); 
    }
    printf("\n\n\n");
  }
  close(portFd);
}
