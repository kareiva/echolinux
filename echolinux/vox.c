/*****************************

This is an alpha release of echlinux.

Copyright 2002 Jeff Pierce wd4nmq.

This software is covered by the included GNU Public License, GPL.

$Log$

****************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/soundcard.h>
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
#include <string.h>
#include <sys/time.h>

#include "gsm.h"
#include "echolinux.h"
#include "echoGlobals.h"

#define SoundFileIn     "/dev/dsp"
#define SoundFileOut     "/dev/dsp"
#define MIXER           "/dev/mixer"
#define RATE 8000
#define CHANNELS 1
#define SIZE 16

#define XMIT 0
#define RECV 1

int audiofd;

gsm gsmh;

unsigned char  gsmBuff[33], tempLine[80], recvBuffer[2044];
unsigned short usbuff[700];
unsigned short seq;

short *sbuff, average;

struct gsmVoice_t voicePacket;

struct sockaddr_in destAddr, inAddr;
int inAudioSocket, outAudioSocket;
char *callsign = NULL;
int  leaveApp = 0, quit = 0, newConnect = 1;

struct stat infoStatus;
int  infoFile;
char *infoBuffer;
char echoLinuxDefaults[80];
int  voxTreshold = 1200;
int  voxDelay = 13; // about 1 second
int  vox = FALSE, sendStrength = FALSE;
int mode;

/**************** read info file ***********/

int sendInfoFile(void){
  int count = 0, result;
  char *temp, line[80];

  strcpy(line, echoLinuxDefaults);
  strcat(line, "info.txt");

  infoFile = open(line, O_RDONLY);
  if(infoFile < 0){
	  infoFile = open("/etc/echolinux/info.txt", O_RDONLY);
	  if(infoFile < 0){
	    perror("Openning info file.");
	    return(FALSE);
	  }
  }

  fstat(infoFile, &infoStatus);
  if(stat < 0){
    perror("Getting info file size.");
    close(infoFile);
    return(FALSE);
  }

  if(infoStatus.st_size <= 0){
    close(infoFile);
    return(FALSE);    
  }

  infoBuffer =(char *) malloc(infoStatus.st_size + 10);
  if(!infoBuffer){
    close(infoFile);
    return(FALSE);    
  }
  strcpy(&infoBuffer[0], "oNDATA\r");
  count = read(infoFile, &infoBuffer[7], infoStatus.st_size);
  if(count != infoStatus.st_size){
    close(infoFile);
    free(infoBuffer);
    return(FALSE);
  }
  count += 7;
  infoBuffer[count++] = 0x00;

  temp = infoBuffer;
  while(temp){
    temp = strchr(temp,'\n');
    if(temp){
      *temp = '\r';
      temp++;
    }
  }

  result = sendto(outAudioSocket,
		  infoBuffer,
		  count,
		  0,
		  (struct sockaddr *) &destAddr,
		  sizeof destAddr);
  
  
}

/****************************************/

int soundInput(void){

  // int arg, status;
  int arg, status;
  long frag_size  = 0x7FFF000B;

  audiofd = open(SoundFileIn, O_RDWR);
  if(audiofd < 0){
    perror(" opening audio input ");
    exit(1);
  }

  // printf("Opened /dev/audio fd = %d.\n", audiofd);

  if (ioctl(audiofd, SNDCTL_DSP_SETFRAGMENT, &frag_size) == -1) {
    perror("SNDCTL_DSP_SETFRAGMENT ioctl failed");          
  }


  arg = SIZE;
  status = ioctl(audiofd, SOUND_PCM_WRITE_BITS, &arg);
  if(status == -1)
    perror("SOUND_PCM_WRITE_BITS ioctl failed");
  if(arg != SIZE)
    perror("unable to set sample size");

  arg = CHANNELS;
  status =  ioctl(audiofd, SOUND_PCM_WRITE_CHANNELS, &arg);
  if(status == -1)
    perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
  if(arg != CHANNELS)
    perror("unable to set number of channels");

  arg = RATE;
  status = ioctl(audiofd, SOUND_PCM_WRITE_RATE, &arg);
  if(status == -1)
    perror("SOUND_PCM_WRITE_RATE ioctl failed");
  
  arg = AFMT_S16_LE; 
  status = ioctl(audiofd,  SOUND_PCM_SETFMT, &arg);
  if(status == -1)
    perror(" SOUND_PCM_SETFMT ioctl failed");
  
}

/****************** decodeNDATA ***************/

void decodeNDATA(char *buffer){
  char *ptr;

  raw_tty(FALSE);

  if(buffer[strlen(buffer)-1] != '\n')
     // || (buffer[strlen(buffer)-1] == '\r')){
    strcat(buffer, "\n");
     
  /*********
  if((buffer[strlen(buffer)-1] == '\n') && 
     (strncmp(&buffer[2], "CONF", 4) != 0) &&
     (buffer[2] != '\r')){
  ***********/

  if((strncmp(&buffer[2], "CONF", 4) != 0) &&
     (buffer[2] != '\r')){
    buffer[0] = CHAT;
    buffer[1] = ' ';    
    fputs(buffer, stdout); 
  }
  else{
    buffer[0] = INFO;
    buffer[1] = ' ';

    //    buffer[strlen(buffer)-1] = '\n';
    fputs(buffer, stdout);

  }
  raw_tty(TRUE);
}

/******************** Send typed data *****************/

void sendNDATA(char *ndata){

  char *databuf;
  int result;

  databuf = calloc(1,strlen(ndata)+30);
  strcpy(databuf, "oNDATA");
  if(callsign)
    strcat(databuf, callsign);
  strcat(databuf,">");
  strcat(databuf, ndata);
  if(databuf[strlen(databuf) - 1] == '\n')
    databuf[strlen(databuf) - 1] = 0x00;
  strcat(databuf, "\r\n");
  result = sendto(outAudioSocket,
		  databuf,
		  strlen(databuf)+1,
		  0,
		  (struct sockaddr *) &destAddr,
		  sizeof destAddr);


  free(databuf);
}

/**************** Handle data from stdin ***********/

int fromControl(void){
  int i;
  char c;
  char line[80], fileName[80], globalFileName[80];
  int audioEffect;

  fgets(line, 80, stdin);
  if(line[0] == TOGGLE){
    quit = TRUE;
  } else if(line[0] == QUIT){
    quit = TRUE;
    leaveApp = TRUE;
  } else if(line[0] == CHAT){
    sendNDATA(&line[1]);
  } else if(line[0] == PLAY_WAV){
    strcpy(fileName, echoLinuxDefaults);
    line[strlen(line) - 1] = 0x00;
    strcat(fileName, &line[2]);
	strcpy(globalFileName, "/etc/echolinux/");
	strcat(globalFileName, &line[2]);
    //      strcat(fileName, "connect.wav");
    if(((audioEffect = open(fileName, O_RDONLY)) != -1) || (audioEffect = open(globalFileName, O_RDONLY)) != -1){
      do{
	i = read(audioEffect, recvBuffer, 320);
	if(i)
	  write(audiofd, recvBuffer, i); 
      }while(i == 320);
      ioctl(audiofd, SOUND_PCM_SYNC, 0);
      newConnect = 0;
    }
  }else if(line[0] == VOX){
    if(line[1] == VOX_ON)
      vox = TRUE;
    else if(line[1] == VOX_OFF){
      vox = FALSE;
      if(mode == XMIT){
	fileName[0] = TRANSMIT;
	fileName[1] = '\n';
	write(STDOUT_FILENO, fileName, 2);
      }
      else
	quit = TRUE;
    }
    else if(line[1] == VOX_DELAY)
      voxDelay = atoi(&line[3]); 
    else if(line[1] == 'T')
      voxTreshold = atoi(&line[3]); 

  } else if(line[0] == INFO)
    sendInfoFile();
  else if(line[0] == STRENGTH){
    if(line[1] == STRENGTH_ON)
      sendStrength = TRUE;
    else if(line[1] == STRENGTH_OFF)
      sendStrength = FALSE;
  }   

  else if(line[0] == QUIT)
    exit(1);
}

/******************* Audio sending routine **************/

int sendAudio(void){

  static short seq = 1;
  short count = 0, total = 0;
  short i, result, *pTemp, j;
  char c, *temp, line[20];
  fd_set readfds, testfds;
  int len_inet;
  //   struct timeval timeout;
  time_t  startTime, timeNow;
  long average;
  int voxCounter;

  voxCounter = voxDelay;

  FD_ZERO(&readfds);

  raw_tty(1);
 
  if(!vox){
    line[0] = TRANSMIT;
    line[1] = '\n';
    write(STDOUT_FILENO, line, 2);
  }

  mode = XMIT;

  soundInput();

  FD_SET(STDIN_FILENO, &readfds);
  FD_SET(audiofd, &readfds);
  FD_SET(inAudioSocket, &readfds);

  //  timeout.tv_sec = 10;
  //  timeout.tv_usec = 0;
  startTime = time((time_t *)0);
  quit = FALSE;
  while(!quit){

    testfds = readfds;

    result = select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

    if(result < 0){
      perror("select");
      raw_tty(FALSE);
      exit(1);
    }

    timeNow = time((time_t *)0);

    if(timeNow - startTime > TX_TIMEOUT)
      quit = TRUE;

    if(result == 0){
      // We are to long winded, time out 
      quit = TRUE;
    }
   
    if(FD_ISSET(audiofd, &testfds)){
      count = read(audiofd, usbuff, 1280);
      if(count < 0){
	perror("reading /dev/dsp  ");
	raw_tty(0);
	close(audiofd);
      }
      average = 0;
      for(i=0;i<4;i++){
	temp = voicePacket.data;
	gsm_encode(gsmh, usbuff+(i*160), temp+(i*33));
      }
      for(j=0;j<640;j+=2){
	pTemp = &usbuff[j];
	average = average + abs(*pTemp);
      }
      average = average / 320;
      if(vox){
	if(average > voxTreshold){
	  voxCounter = 0;
	  line[0] = TRANSMIT;
	  line[1] = '\n';
	  write(STDOUT_FILENO, line, 2);
	}
	else{
	  if(++voxCounter > voxDelay){
	    mode = RECV;
	    voxCounter = voxDelay;
	    line[0] = RECEIVE;
	    line[1] = '\n';
	    write(STDOUT_FILENO, line, 2);
	  }
	}
      }
      if(sendStrength){
	sprintf(line, "%C %d", STRENGTH, average);
	puts(line);
      }

      voicePacket.seqNum = htons(seq++);

      if(vox){
	if((average > voxTreshold) ||
	   (voxCounter < voxDelay)){
	  mode = XMIT;
	  result = sendto(outAudioSocket,
			  &voicePacket,
			  sizeof voicePacket,
			  0,
			  (struct sockaddr *) &destAddr,
			  sizeof destAddr);
	  if(result < 0){
	    perror("sendto");
	    close(audiofd);
	    raw_tty(0);
	    exit(1);
	  }
	}
      } 
      else {
	result = sendto(outAudioSocket,
			&voicePacket,
			sizeof voicePacket,
			0,
			(struct sockaddr *) &destAddr,
			sizeof destAddr);
	if(result < 0){
	  perror("sendto");
	  close(audiofd);
	  raw_tty(0);
	  exit(1);
	}


      }
    }

    if(FD_ISSET(inAudioSocket, &testfds)){
      len_inet = sizeof inAddr;
      result = recvfrom(inAudioSocket,
			recvBuffer,
			2048,
			0,
			(struct sockaddr *)&inAddr,
			&len_inet);

      if(result < 0){
	perror("recvfrom(2) in sendAudio ");
	close(audiofd);
	raw_tty(FALSE);
	exit(1);
      }
      if(recvBuffer[0] != 0xc0){
	// puts("E got a NDATA packet.");
	if(strncmp(&recvBuffer[1], "NDATA", 5) == 0)
	  decodeNDATA(&recvBuffer[4]);
      } else if((recvBuffer[0] == 0xc0) &&
		(voxCounter == voxDelay)){
	quit = TRUE;
      }
    }

    if(FD_ISSET(STDIN_FILENO, &testfds)){
      fromControl();
    }
  }    

  close(audiofd);
  raw_tty(0);
}

/************ Audio Receiving Function *******/

int recvAudio(void){
  struct gsmVoice_t *voicePacket;
  fd_set readfds, testfds;
  short i, result, j, *pTemp;
  int len_inet;
  audio_buf_info info;
  unsigned char *temp;
  char c, line[20];
  int audioEffect;
  struct timeval timeout;
  long average;

  mode = RECV;

  FD_ZERO(&readfds);

  line[0] = RECEIVE;
  line[1] = '\n';
  write(STDOUT_FILENO, line, 2);

  soundInput();

  raw_tty(1);

  voicePacket = (struct gsmVoice_t *) recvBuffer;

  FD_SET(STDIN_FILENO, &readfds);
  FD_SET(inAudioSocket, &readfds);

  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  quit = FALSE;
  while(!quit){

    testfds = readfds;

    if(vox)
      result = select(FD_SETSIZE, &testfds, NULL, NULL, &timeout);
    else
      result = select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

    if((result == 0) && vox){ // timeout
      quit = TRUE;
      break;
    }
    len_inet = sizeof inAddr;

    if(FD_ISSET(inAudioSocket, &testfds)){
      result = recvfrom(inAudioSocket,
			recvBuffer,
			2048,
			0,
			(struct sockaddr *)&inAddr,
			&len_inet);

      if(result < 0){
	perror("recvfrom(2) in recvAudio");
	raw_tty(FALSE);
	exit(1);
      }
      if(recvBuffer[0] != 0xc0){
	if(strncmp(&recvBuffer[1], "NDATA", 5) == 0)
	  decodeNDATA(&recvBuffer[4]);
      }else{
	average = 0;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	for(i=0;i<4;i++){
	  temp = (unsigned char *)&(voicePacket->data);
	  gsm_decode(gsmh, temp+(i*33), sbuff);
	  ioctl(audiofd, SNDCTL_DSP_GETOSPACE, &info);
	  while(info.bytes < 320){
	    ioctl(audiofd, SNDCTL_DSP_GETOSPACE, &info);
	  }
	  write(audiofd, sbuff, 320);
	  for(j=0;j<640;j+=2){
	    pTemp = &sbuff[j];
	    average = average + abs(*pTemp);
	  }
	  average = average / 320;
	  if(sendStrength){
	    sprintf(line, "%C %d", STRENGTH, average);
	    puts(line);
	  }
	}
      }
    }
    if(FD_ISSET(STDIN_FILENO, &testfds)){
      // strcpy(tempLine,"Data from stdin.\n");
      // write(STDOUT_FILENO, tempLine, strlen(tempLine));
      fromControl();
    }
  }

  raw_tty(0);
  close(audiofd);
}

/************ Main Program ************/

int main(int argc, char *argv[]){

  long total, count;
  int status, result, len_inet, i;
  audio_buf_info info;
  char c, commandLine[80], *home;
  int  mixerfd;

  sbuff = (short *) usbuff;

  if(argc >= 3)
    callsign = argv[2];

  if(setvbuf(stdin, NULL, _IOLBF, 0) != 0){
    perror("setvbuf stdin");
    exit(1);
  }

  if(setvbuf(stdout, NULL, _IOLBF, 0) != 0){
    perror("setvbuf stdout");
    exit(1);
  }

  home = getenv("HOME");
  strcpy(echoLinuxDefaults, home);
  strcat(echoLinuxDefaults, "/.echoLinux/");

  /**************** Make mic record input *************/

  /************************************
  mixerfd = open(MIXER, O_RDONLY);
  if (mixerfd == -1) {
    perror("unable to open /dev/mixer");
    exit(1);
  }

  i = 1;
  i = i << SOUND_MIXER_MIC;
  status = ioctl(mixerfd, SOUND_MIXER_WRITE_RECSRC, &i);
  if (status == -1){
    perror("SOUND_MIXER_WRITE_RECSRC ioctl failed");
    exit(1);
  }
  close(mixerfd);
  ******************************/
  /***************** Make audio output socket ***************/

  memset(&destAddr, 0, sizeof destAddr);
  destAddr.sin_family = AF_INET;
  destAddr.sin_port = htons(PORT_BASE);
  destAddr.sin_addr.s_addr = inet_addr(argv[1]);

  if(destAddr.sin_addr.s_addr == INADDR_NONE){
    perror("bad address");
    exit(1);
  }

  outAudioSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if(outAudioSocket == -1){
    perror("socket");
    exit(1);
  }

  /************** Make audio input socket **********/
  memset(&inAddr, 0, sizeof inAddr);
  inAddr.sin_family = AF_INET;
  inAddr.sin_port = htons(PORT_BASE);
  inAddr.sin_addr.s_addr = INADDR_ANY;

  inAudioSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if(inAudioSocket== -1){
    perror("inAudioSocket");
    exit(1);
  }

  status = bind(inAudioSocket, (struct sockaddr *)&inAddr, sizeof inAddr);
  if(status == -1){
    perror("duplex-bind");
    exit(1);
  }

  /******************************************/

  gsmh = gsm_create();  
  voicePacket.version = 0xc0;
  voicePacket.pt = 0x03;
  voicePacket.time = htonl(0);
  voicePacket.ssrc = htonl(0);


  while(!leaveApp){
    if(!leaveApp)
      recvAudio();
    if(!leaveApp)
      sendAudio();
  }

}
