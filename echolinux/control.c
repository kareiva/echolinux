/*****************************

This is an alpha release of echlinux.

Copyright 2002 Jeff Pierce wd4nmq.

This software is covered by the included GNU Public License, GPL.

$Log$

****************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

#include "echolinux.h"
#include "rtp.h"
#include "rtpacket.h"
#include "echoGlobals.h"

extern int sendLogon(char *);

#define CONNECT_TRY 5

struct sockaddr_in destAddr, inAddr, activeNode;
int outControlSocket, inControlSocket;
int connectTimeout, sdesLength;
struct hostent *hp;
char *remoteNode = NULL;
struct timeval timeout;

int  connected = 0, atemptingConnect = 0, quit = 0;
pid_t  audioPid;
 
unsigned char  **sdesPacket;
unsigned char  *udpPacket;
unsigned char  inBuffer[2048];
char callsign[20], name[40], location[40], password[10];
char server[80];
fd_set readfds, testfds;

FILE *userData;

int audioInPipe[2], audioOutPipe[2];
FILE *audioOutFd;

/**************************/

void printHex(unsigned char *buff, int size){

  int i, j, k;
  char line[80];

  for(i=0;i<size;i+=16){
    if((k = size - i) >= 16)
      k = 16;
    for(j=0;j<k;j++)
      printf("%02x ", buff[j+i]);
    if(k < 16)
      for(j = 16;j>k;j--)
        printf("   ");
    for(j=0;j<k;j++){
      if((buff[i+j] <= '~') && (buff[i+j] > ' ')) 
        printf("%c", buff[j+i]);
      else
        printf(".");
    }
    printf("\n");
  }
}

/*************** Signal handler ************/

static void sigChild( int signo){
  pid_t pid;
  int status;

  if(signo == SIGCHLD){
    pid = wait(&status);
    printf("Child process %d has termninated./n", pid);
    if(pid == audioPid)
      audioPid = 0;
    signal(SIGCHLD, sigChild);
  }
}
     
/************** Make connection *************/

int makeConnection(char *host){

  remoteNode = host;
  hp = gethostbyname(remoteNode);

  if(hp){
    remoteNode = inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]);
    //    printf("IP address for %s is %s..\n", host, 
    //   inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]));
    // printf("IP address for %s is %s..\n", host,
    //   remoteNode);
  } else
    return(0);

  if((pipe(audioInPipe)== 0) && (pipe(audioOutPipe) == 0)){
    audioPid = fork();
    if(audioPid == 0){

      close(STDIN_FILENO);
      dup(audioOutPipe[0]);
      close(audioOutPipe[1]);
      close(audioOutPipe[0]);
      close(STDOUT_FILENO);
      dup(audioInPipe[1]);
      close(audioInPipe[1]);
      close(audioInPipe[0]);

      execlp("echoaudio", "echoaudio", remoteNode, callsign, 0);
      perror("echoaudio");
      exit(1);
    }
  }
  else{
    perror("pipe");
    exit(1);
  }

  close(audioInPipe[1]);
  close(audioOutPipe[0]);

  audioOutFd = fdopen(audioOutPipe[1], "w");
  if(!audioOutFd){
    perror("fdopen");
    exit(1);
  }

  if(setvbuf(audioOutFd, NULL, _IOLBF, 0) != 0){
    perror("setvbuf audioOutFd");
    exit(1);
  }

  FD_SET(audioInPipe[0], &readfds);

  memset(&destAddr, 0, sizeof destAddr);
  destAddr.sin_family = AF_INET;
  destAddr.sin_port = htons(PORT_BASE+1);
  destAddr.sin_addr = *(struct in_addr *)hp->h_addr_list[0];
  //  destAddr.sin_addr.s_addr = inet_addr(remoteNode);


  if(destAddr.sin_addr.s_addr == INADDR_NONE){
    perror("bad address");
    exit(1);
  }
  // Start the ball rolling
  usleep(300000);  // delay to give duplex time to start
  sendto(outControlSocket,
	 *sdesPacket,
	 sdesLength,
	 0,
	 (struct sockaddr *)&destAddr,
	 sizeof destAddr);
  //  atemptingConnect = 1;
  connectTimeout = 0;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

}

/**** disconnect from remote node *********/

int disconnect(int sendBye){

  int i, length;
  unsigned char *bye;
  char line[5];

  line[0] = QUIT;
  line[1] = '\n';
  line[2] = 0x00;
  fputs(line, audioOutFd);
  // fputs("Q\n", audioOutFd);
  // write(audioOutPipe[1], "Q", 1);
  do{
    i = waitpid(audioPid, NULL, WNOHANG);
  }while(i != audioPid);
  if(sendBye){
    bye = malloc(50);
    length = rtp_make_bye(bye, 0, "jan2002", 1);
    sendto(outControlSocket,
	   bye,
	   length,
	   0,
	   (struct sockaddr *)&destAddr,
	   sizeof destAddr);
    free(bye);
  }
  //	while(audioPid); // wait for audio to die
  connected = 0;
  audioPid = 0;
  close(audioInPipe[0]);
  close(audioOutPipe[1]);
  FD_CLR(audioInPipe[0], &readfds);
  printf("D\n");
}

/*************** read from stdin *****************/

void readStdin(void){

  char line[80], addr[80];
  int  i, length;
  unsigned char *bye;

  fgets(line, 80,stdin);

  if(toupper(line[0]) == QUIT){
    quit = 1;
    if(audioPid){
      disconnect(TRUE);
    }
  }
  else if(toupper(line[0]) == DISCONNECT){
    if(audioPid)
      disconnect(TRUE);
  }
  else if(toupper(line[0]) == CONNECT){
    if(!audioPid){
      sscanf(&line[1],"%s", addr);
      atemptingConnect = 1;
      makeConnection(addr);
    }
  }
  else{
    if(audioPid)
      fputs(line, audioOutFd);
  }
}

/**************************************/
/************ Main Program ************/


int main(int argc, char *argv[]){

  int length, i, len_inet;
  char c, remoteName[40], line[80], *home;
  struct rtcp_sdes_request sdesItems;

  if(setvbuf(stdin, NULL, _IOLBF, 0) != 0){
    perror("setvbuf stdin");
    exit(1);
  }

  if(setvbuf(stdout, NULL, _IOLBF, 0) != 0){
    perror("setvbuf stdout");
    exit(1);
  }

  home = getenv("HOME");
  strcpy(line, home);
  strcat(line, "/.echoLinux/userdata.txt");
  userData = fopen(line, "r");
  if(userData == NULL){
	  userData = fopen("/etc/echolinux/userdata.txt", "r");
	  if(userData == NULL){
	    perror("userData");
	    exit(1);
	  }
  }

  fgets(callsign, 40, userData);
  callsign[strlen(callsign)-1] = 0x00;
  fgets(name, 40, userData);
  name[strlen(name)-1] = 0x00;
  fgets(location, 40, userData);
  location[strlen(location)-1] = 0x00;
  fgets(password, 40, userData);
  password[strlen(password)-1] = 0x00;
  fclose(userData);

  // Now get server address

  //  (void) signal(SIGCHLD, sigChild);  

  /************* create audio task ************/


  outControlSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if(outControlSocket == -1){
    perror("inControlSocket");
    exit(1);
  }

  /**************** Make input socket ***************/
  memset(&inAddr, 0, sizeof inAddr);
  inAddr.sin_family = AF_INET;
  inAddr.sin_port = htons(PORT_BASE+1);
  //  inAddr.sin_addr.s_addr = inet_addr("192.168.2.205");
  inAddr.sin_addr.s_addr = INADDR_ANY;

  inControlSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if(inControlSocket== -1){
    perror("inControlSocket");
    exit(1);
  }

  i = bind(inControlSocket, (struct sockaddr *)&inAddr, sizeof inAddr);
  if(i == -1){
    perror("control-bind");
    exit(1);
  }

  sdesPacket = &udpPacket;
  sdesLength = rtp_make_sdes(sdesPacket, 0, 1);
  if(sdesLength <= 0){
    perror("rtp_make_sdes");
    exit(1);
  }
  // raw_tty(1);

  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);
  FD_SET(inControlSocket, &readfds);
  // prompt();
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  while(!quit){

    testfds = readfds;

    i = select(FD_SETSIZE, &testfds, NULL, NULL, &timeout);
    if(i < 0){
      perror("select");
      exit(1);
    }

    if(i == 0){
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    }
    
    if((i == 0) && (audioPid > 0)){  // timeout & connected 
      connectTimeout++;
      if(connectTimeout == CONNECT_TRY){
	// We've not received a stay alive in four time frames
	// connected node has disappeared.
	disconnect(0);
      }
      else{  // connected node still there
	sendto(outControlSocket,
	       *sdesPacket,
	       sdesLength,
	       0,
	       (struct sockaddr *)&destAddr,
	       sizeof destAddr);      
      }
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    }
    if(FD_ISSET(inControlSocket, &testfds)){
      len_inet = sizeof inAddr;
      length = recvfrom(inControlSocket,
			inBuffer,
			2048,
			0,
			(struct sockaddr *)&inAddr,
			&len_inet);

      if(isRTCPByepacket(inBuffer, length) && audioPid){  // it's a bye packet
	// Is from connected node
	if(inAddr.sin_addr.s_addr == destAddr.sin_addr.s_addr) 
	  disconnect(FALSE);
      } 
      if(isRTCPSdespacket(inBuffer, length)){
	if (audioPid > 0) {
	  sendto(outControlSocket,
		 *sdesPacket,
		 sdesLength,
		 0,
		 (struct sockaddr *)&destAddr,
		 sizeof destAddr);
	  if(inAddr.sin_addr.s_addr == destAddr.sin_addr.s_addr){
	    sdesItems.nitems = 1;
	    sdesItems.item[0].r_item = RTCP_SDES_NAME;
	    sdesItems.item[0].r_text = NULL;
	    remoteName[0] = 0;
	    parseSDES(inBuffer, &sdesItems);
	    if(sdesItems.item[0].r_text != NULL){
	      copySDESitem(sdesItems.item[0].r_text, &remoteName[2]);
	    }
	    remoteName[0] = CONNECT;
	    remoteName[1] = ' ';
	    puts(remoteName);
	    if(atemptingConnect){
	      atemptingConnect = 0;
	      fputs("P connect.wav\n", audioOutFd);
	      fputs("I\n", audioOutFd);
	    }
	    connectTimeout = 0;  // reset the counter
	  } else { /* audioPid <= 0 */
	    sdesItems.nitems = 1;
	    sdesItems.item[0].r_item = RTCP_SDES_NAME;
	    sdesItems.item[0].r_text = NULL;
	    remoteName[0] = 0;
	    parseSDES(inBuffer, &sdesItems);
	    if(sdesItems.item[0].r_text != NULL){
	      copySDESitem(sdesItems.item[0].r_text, &remoteName[2]);
	    } 
	    remoteName[0] = CONNECT;
	    remoteName[1] = ' ';
	    puts(remoteName);
	    makeConnection(inet_ntoa(inAddr.sin_addr));
	    fputs("P connect.wav\n", audioOutFd);
	    fputs("I\n", audioOutFd);
	  }
	}
      }
    }
    if(FD_ISSET(STDIN_FILENO, &testfds)){
      readStdin();
      // prompt();
    }
    if(FD_ISSET(audioInPipe[0], &testfds)){
      i = read(audioInPipe[0], inBuffer, sizeof inBuffer);
      // if(line[0] == 'I')
      // printf("Received info packet %d bytes.\n", i);
      if(i > 0){
	//	line[i] = 0x00;
	write(STDOUT_FILENO, inBuffer, i);
      }
    }


  } // while(!quit)
  free(*sdesPacket);
}















