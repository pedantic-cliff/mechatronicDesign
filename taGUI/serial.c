#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#define error_message(...) fprintf(stderr, __VA_ARGS__)
#define BUFFERSIZE 300

void saveBuf(char *buf, int n);

/*
 * The values for speed are B115200, B230400, B9600, B19200, B38400, B57600,
 * B1200, B2400, B4800, etc. The values for parity are 0 (meaning no parity),
 * PARENB|PARODD (enable parity and use odd), PARENB (enable parity and use even),
 * PARENB|PARODD|CMSPAR (mark parity), and PARENB|CMSPAR (space parity).
 */
    int
set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof(tty));
    if (tcgetattr (fd, &tty) != 0)
    {
        error_message ("error %d from tcgetattr\n", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // ignore break signal
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        error_message ("error %d from tcsetattr\n", errno);
        return -1;
    }
    return 0;
}

    void
set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof(tty));
    if (tcgetattr (fd, &tty) != 0)
    {
        error_message ("error %d from tggetattr\n", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        error_message ("error %d setting term attributes\n", errno);
}


void main(int argc, char* argv[])
{

    FILE *pf;
    int speed = B9600;
    if(argc < 2)
    {
        printf("Usage: %s path_to_device baud_rate\n", argv[0]);
        printf("List of baud rates: 38400: %d, 19200: %d, 9600: %d, 4800: %d\n", B38400, B19200, B9600, B4800);
        return;
    }

    if(argc > 2)
        speed = atoi(argv[2]);
    switch(speed)
    {
        case B38400:
        case B19200:
        case B9600:
        case B4800:
            break;
        default:
            printf("baud rate not valid!\n");
            return;
    }


    printf("Opening %s on %d\n", argv[1], speed);
    int fd = open (argv[1], O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        error_message ("error %d opening %s: %s\n", errno, argv[1], strerror (errno));
        return;
    }


    usleep(3500000);
//    fcntl(fd, F_SETFL, 0);
   
       set_interface_attribs (fd, speed, 0);  // set speed to 115,200 bps, 8n1 (no parity)
       set_blocking (fd, 0);                // set no blocking
  
    char send_buf[100] = "";
    char rev_buf[100] = "";
    char *buf = (char*)calloc(BUFFERSIZE, sizeof(char));
    int running = 1;
    char start_cmd[10] = "start";
    char con_cmd[10] = "con";
    char exit_str[10] = "exit";
    srand(time(NULL)); // initialize random seed
    char randID = -1;
    int past = 0;

    while(running)
    {
        printf("Input the command: ");
        gets(send_buf);
        if(strcmp(send_buf,exit_str) == 0)
            break;
        else if(strcmp(send_buf, con_cmd) == 0)
        {
            past = 1;
        }
        else if(strcmp(send_buf, start_cmd) != 0)
        {
            printf("%s was not a valid command!\n", send_buf);
            continue;
        }
        else
        {
            char buffer[10] = {0};
            randID = rand();
            sprintf(buffer, "%c%c%c", 0xff, randID, 1);
            printf("From serial.c: %x, %x, %x\n", buffer[0] & 0xff,buffer[1] & 0xff,buffer[2] & 0xff);
            tcflush(fd, TCIFLUSH);
            printf("Buffer length: %d, written size: %d\n", strlen(buffer), write(fd, buffer,  strlen(buffer)));
        }

        if(!past){
            int k = 0;
            while((k += read(fd, rev_buf+k, BUFFERSIZE)) < 2);  // read up to 100 characters if ready to read
            // Authorize
            printf("read[%d]: \n", k);
            printf("random ID: %x, received: %x,%x\n", randID&0xff, rev_buf[0]&0xff, rev_buf[1]&0xff);
            if((randID != rev_buf[0]) || (rev_buf[1] != 1))
                continue;
            else
            {   
                pf = fopen("result", "w");
                if(pf == NULL)
                {
                    printf("Error opening file!\n");
                }
                else
                {
                    fprintf(pf, "%d,%s", (int)time(NULL), rev_buf);
                    fclose(pf);
                }
            }
        }

        printf("Authorized!\n");
        usleep(350000);
        while(1) 
        {
            int sum_n = 0;
            while((sum_n += read (fd, buf+sum_n, BUFFERSIZE)) < 83);  // read up to 100 characters if ready to read
            if(sum_n >= 83)
            {
                printf("Received packet[%d]\n", sum_n);
                saveBuf(buf, 83);
            }
            else
            {
                printf("Received packet[%d]\n", sum_n);
                puts(buf);
//                printf("\nDone with transmission!\n");
            }
        }
    }
    free(buf);
}

void saveBuf(char *buf, int n)
{
    char a;
    FILE *pf = fopen("result", "r");
    FILE *pf_b = fopen("output.bak", "w");
    if(pf_b == NULL)
    {
        printf("Cannot open backup file!");
        return;
    }
    if(pf == NULL)
    {
        pf = fopen("result", "w");
        if(pf == NULL)
        {
            printf("Cannot open result file\n");
            return;
        }
    }

    fprintf(pf_b, "%d", (int)time(NULL));
//    printf("%d", (int)time(NULL));
    /*
       if(buf[0] < 10)
       {
       printf(",%c", buf[0]+0x30);
       fprintf(pf_b, ",%c", buf[0]+0x30);
       }
       else
       {
       int dex = buf[0]/10;
       int rem = buf[0]%10;
       printf(",%c%c", dex+0x30, rem+0x30);
       fprintf(pf_b, ",%c%c", dex+0x30, rem+0x30);
       }
     */
    int i;
    
    printf("%d", buf[0]);
    fprintf(pf_b, ",%d", buf[0]);
    printf(",%d", buf[1]);
    fprintf(pf_b, ",%d", buf[1]);
 
    for(i=2;i<n;i++){
        if(!((i-2)%9))
        {
            printf("\n");
        }
        printf(",%d", buf[i]);

        fprintf(pf_b, ",%d", buf[i]);
    }
    fprintf(pf_b, "\n");
    printf("\n");

    do
    {
        a=fgetc(pf);
        fputc(a,pf_b);
    }while(a!=EOF);

    fclose(pf);
    fclose(pf_b);

    char cmd[] = "cp output.bak result";
    system(cmd);
}
