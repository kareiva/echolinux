/*****************************

Copyright 2002 Jeff Pierce wd4nmq.

This software is covered by the included GNU Public License, GPL.

$Log$

****************************/ 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

#include "serverglobals.h"
#include "threadFuncs.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

extern void finishList();

struct serverList {
  char domainName[80];
  struct in_addr addr;
  struct serverList *next;
};

struct recordLineT recordLine;
 
struct serverList *pServerHead = NULL, *pServer = NULL;
struct serverList *pServerCurrent = NULL;

struct stationData *pStation = NULL, *pStationListHead = NULL;

struct entry *addEntryList(struct entry *, struct stationData *);
struct entry *flushEntryList(struct entry *);

void printList(struct entry *);
int serverSocket = 3;
int recordCount;

struct entry        *pListHead = NULL;
struct entry        *entryPtrs[5];

void (*serverTask)(void);

enum serverPortStateT serverPortState = CLOSED;
enum serverStateT serverState = IDLE;
enum serverStateT serverNextState = IDLE;
enum getCallStatT getCallState;

FILE *servers;
FILE *userFd;
FILE *serverFd;

char commandLine[80];
char serverFileName[80];
extern char callsign[20], name[40], location[40], password[10];

fd_set readFds, testReadFds;
fd_set writeFds, testWriteFds;
fd_set exceptFds, testExceptFds;

int    doTimeout = 0;
struct timeval timeout;
struct tm *tm_ptr;
time_t    theTime;
char      logTime[20];

struct sockaddr_in adr_srvr;/* AF_INET */
int len_inet;               /* length  */

char *sServerState[]={
  "IDLE",
  "LOGON",
  "LOGOFF",
  "GETCALLS",
  "MAKE BUSY"};

/************************/
/**** Open a socket *****/

int  openSocket(void){

  int  z;
  struct hostent *hp;
  char *addr;

  /************
  printf("Attempting %s server.\n", pServerCurrent->domainName);  
  hp = gethostbyname(pServerCurrent->domainName);

  if(hp){
    addr = inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]);
 
  } else {
    return(-1);
  }
  ******************/
  (void) time(&theTime);
  tm_ptr = localtime(&theTime);
  strftime(logTime, 20, "%H:%M:%S", tm_ptr);
  // printf("%s Attempting %s server.\n", logTime, pServerCurrent->domainName);  
  memset(&adr_srvr,0,sizeof adr_srvr);
  adr_srvr.sin_family = AF_INET;
  adr_srvr.sin_port = htons(5200);
  adr_srvr.sin_addr.s_addr = pServerCurrent->addr.s_addr;
  // (*(unsigned long *)(pServerCurrent->addr));
    //    inet_addr(addr);

  if ( adr_srvr.sin_addr.s_addr == INADDR_NONE ){
    //    fclose(serverFd);
    return(-1);
  }
  len_inet = sizeof adr_srvr;

  /*
   * Create a TDP/IP socket to use :
   */
  serverSocket = socket(AF_INET,SOCK_STREAM,0);
  if ( serverSocket == -1 ){
    //    fclose(serverFd);
    return(serverSocket);
  }
    
  /*
   * Connect to the server:
   */

  fcntl(serverSocket, F_SETFL, O_NONBLOCK);
 
  /* Get the local time */
  /*************
  (void) time(&theTime);
  tm_ptr = localtime(&theTime);
  strftime(logTime, 20, "%H:%M:%S", tm_ptr);
  puts(logTime);
  *************/

  z = connect(serverSocket, (struct sockaddr *) &adr_srvr,len_inet);
  if ( z == -1){
    if(errno == EINPROGRESS) {
      serverPortState = CONNECTING;
      timeout.tv_sec = 60; // 1 minute timeout
      doTimeout = 1;
      //printf("%s Wait for server connection on socket %d.\n", 
      //     logTime, serverSocket);
      FD_SET(serverSocket, &writeFds);
      FD_SET(serverSocket, &exceptFds);
    }
    return(-1);
  }

  //puts("We openned server right away");
  serverPortState = CONNECTED;
    //  fclose(serverFd);
  return(serverSocket);

}

/************************************/

struct stationData *getEntry(int id){

  struct entry *item;

  item = pListHead;

  // printf("Searching %d records.\n", recordCount);

  while((item != NULL) &&
        ((item->station)->id != id)){
     item = item->next;
  }
  
  if(item != NULL){
    return(item->station);
  }
  else{
    return(NULL);
  }
}


/************************************/

int sendLogon(void){

  struct tm *tm_ptr;
  time_t    theTime;
  char      logTime[6];
  unsigned char    sendBuf[50], recvBuf[50];
  int     i, s, z, result;

  /* Now send the 'l' command */

  serverFd = fdopen(serverSocket, "r"); 

  /* Get the local time */
  (void) time(&theTime);
  tm_ptr = localtime(&theTime);
  strftime(logTime, 6, "%H:%M", tm_ptr);
  
  sprintf(sendBuf,"\x6c%s\xac\xac%s\rONLINE3.38(%s)\r%s\r",
  	callsign, password, logTime, location);

  result = write(serverSocket, sendBuf, strlen(sendBuf));

  serverState = LOGON;

  FD_SET(serverSocket, &readFds);

  return(1);

}

/************************************/

int sendLogoff(void){

  unsigned char    sendBuf[50], recvBuf[50];
  int     i, result;

  /* Now send the 'l' command */
  serverFd = fdopen(serverSocket, "r"); 

  memset(sendBuf, 0x6c, 1);
  result = write(serverSocket, sendBuf, 1);

  strcpy(sendBuf, callsign);
  strcat(sendBuf,"\254\254");
  strcat(sendBuf, password);
  strcat(sendBuf,"\015OFF-V3.40\015");
  strcat(sendBuf, location);
  strcat(sendBuf, "\015");
  for(i=0;i<strlen(sendBuf);i++){
  }

  result = write(serverSocket, sendBuf, strlen(sendBuf));

  serverState = LOGOFF;
  FD_SET(serverSocket, &readFds);

  return(1);

}

/************************************/

// int makeMeBusy(char *station){
int makeMeBusy(void){

  struct tm *tm_ptr;
  time_t    theTime;
  char      logTime[6];
  unsigned char    sendBuf[50], recvBuf[50];
  int     i, s, result;

  /* Now send the 'l' command */

  serverFd = fdopen(serverSocket, "r"); 

  memset(sendBuf, 0x6c, 1);
  result = write(serverSocket, sendBuf, 1);

  /* Get the local time */
  (void) time(&theTime);
  tm_ptr = localtime(&theTime);
  strftime(logTime, 6, "%H:%M", tm_ptr);

  strcpy(sendBuf, callsign);
  strcat(sendBuf,"\254\254");
  strcat(sendBuf, password);
  strcat(sendBuf,"\015BUSY3.40(");
  strcat(sendBuf, logTime);
  strcat(sendBuf, ")\015");
  //  if(station == NULL)
  strcat(sendBuf,location);
    // else{
    //   strcat(sendBuf, "In QSO, ");
    //    strcat(sendBuf, station);
    //  }
  strcat(sendBuf,"\015");
  for(i=0;i<strlen(sendBuf);i++){
  }
  result = write(serverSocket, sendBuf, strlen(sendBuf));
  serverState = MAKE_BUSY;
  FD_SET(serverSocket, &readFds);
  return(1);
}

/***********************/

void addServerList(char *line){

  struct hostent *hp;

  hp = gethostbyname(line);

  if(hp){ 
    if(pServerHead == NULL){
      pServerHead = (struct serverList *) malloc(sizeof(struct serverList));
      pServerHead->next = pServerHead;
      strcpy(pServerHead->domainName, line);
      pServerHead->addr = *(struct in_addr *)hp->h_addr_list[0];
      // printf("%s %s\n", inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]),
      //     inet_ntoa(pServerHead->addr));
    }
    else {
      pServer = pServerHead;
      while(pServer->next != pServerHead)
	pServer = pServer->next;

      pServer->next = (struct serverList *) malloc(sizeof(struct serverList));
      (pServer->next)->next = pServerHead;
      strcpy((pServer->next)->domainName, line);
      (pServer->next)->addr = *(struct in_addr *)hp->h_addr_list[0];
      // printf("%s %s\n", inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]),
      //    inet_ntoa((pServer->next)->addr));
    }
  }
}

/************************/
int getCalls(){

  unsigned char sendBuf[10], recvBuf[2000];
  int i, result;

  for(i=0;i<5;i++)
    entryPtrs[i] = NULL;

  serverFd = fdopen(serverSocket, "r");
  if(serverFd == NULL){
    perror("Openning rx stream");
    close(serverSocket);
    return(-1);
  }

  sendBuf[0] = 's';
  result = write(serverSocket, sendBuf, 1);

  serverState = GETCALLS;
  getCallState = PREAMBLE;
  timeout.tv_sec = 5;
  FD_SET(serverSocket, &readFds);
  return(1);

}

/************* find end of list **********/

struct entry *findEnd(struct entry *list){

  struct entry *temp;

  if(list == NULL){ 
    return(NULL);
  }

  temp = list;
  while(temp->next != NULL){
    temp = temp->next;
  }

  return(temp);
}

/******** Flush entry list *************/

struct entry *flushEntryList(struct entry *list){

  struct entry  *temp;
  int           counter;

  if(list == NULL)
    return(0);

  counter = 0;
  do{
    counter++;
    temp = list->next;
    if(list->station != NULL)
      free(list->station);
    free(list);
    list = temp;
  }while(list != NULL);
  return(NULL);
}

/******* Add to entry list ***********/

struct entry *addEntryList(struct entry *list, struct stationData *station){

  struct entry  *next, *previous, *temp, *present;
  int           result;

  if(list == NULL){
    list = malloc(sizeof (struct entry));
    if(list != NULL){
      list->station = station;
      list->next = NULL;
    }
    return list;
  }

  if(station->call[0] == ' '){
    present = findEnd(list);
    present->next = malloc(sizeof(struct entry));
    (present->next)->next = NULL;
    (present->next)->station = station;
    return(list);
  }
    
  present = previous = list;

  while(1){
    result = strcmp(station->call, (present->station)->call);
    if(result > 0){
      if(present->next == NULL){
	present->next = malloc(sizeof(struct entry));
	(present->next)->next = NULL;
	(present->next)->station = station;
	return(list);
      }
      else {
	previous = present;
	present = present->next;
      }
    } else if(result < 0){
      temp = malloc(sizeof(struct entry));
      temp->station = station;
      temp->next = present;
      if(previous != present){
	previous->next = temp;
	return(list);
      }
      else
	return(temp);
    } else if(result == 0)
      return(list);
  }

}

/*********** Print stations list **********/

void printList(struct entry *list){

  char line[132], c;

  while(list != NULL){
    printf("%-9s%-39s%6d %s\n", (list->station)->call,
	    (list->station)->data, (list->station)->id, (list->station)->ip);
    list = list->next;
  }
}
/***********************************/

int serverInit(void){

  char userFileName[80];
  char echoLinuxDefaults[80];

	  
  strcpy(echoLinuxDefaults, getenv("HOME"));
  strcat(echoLinuxDefaults, "/.echoLinux/");
  strcpy(userFileName, echoLinuxDefaults);
  strcat(userFileName, "userdata.txt");
  userFd = fopen(userFileName, "r");
  if(!userFd){
	  userFd = fopen("/etc/echolinux/userdata.txt", "r");
	  if(!userFd){
	    perror("Openning usersdata.txt");
	   	exit(1);
	  }
  }

  fgets(callsign, 20, userFd);
  if(callsign[strlen(callsign)-1] == '\n')
    callsign[strlen(callsign)-1] = 0x00;
  fgets(name, 40, userFd);
  if(name[strlen(name)-1] == '\n')
    name[strlen(name)-1] = 0x00;
  fgets(location, 40, userFd);
  if(location[strlen(location)-1] == '\n')
    location[strlen(location)-1] = 0x00;
  fgets(password, 20, userFd);
  if(password[strlen(password)-1] == '\n')
    password[strlen(password)-1] = 0x00;
  
  strcpy(serverFileName, echoLinuxDefaults);
  strcat(serverFileName, "servers.txt");
  if(!(servers = fopen(serverFileName, "r"))){
	  servers = fopen("/etc/echolinux/servers.txt", "r");
	  if(!servers){
	    perror("Openning servers.txt");
	    exit(1);
	  }
  }

  while(fgets(commandLine, 79, servers)){
    if(commandLine[strlen(commandLine)-1] == '\n')
      commandLine[strlen(commandLine)-1] = 0x00;
    if((commandLine[0] != 0x00) && (commandLine[0] != '#')){
      // printf("Adding %s to server list.\n", commandLine);
      addServerList(commandLine);
    }
  }
  
  /**************
  if(pServerHead){
    printf("%s\t%s\n",
	   pServerHead->domainName, inet_ntoa(pServerHead->addr));
    pServer = pServerHead->next;
    do{
      printf("%s\t%s\n",
	     pServer->domainName, inet_ntoa(pServer->addr));
      pServer = pServer->next;
    }while(pServer != pServerHead);
  }
  ********************/
}

/**********************************/
/************** serverThread **************/
/**********************************/

int doServer(void *ptr){

  pServerQueueEntryT pCommand;
  int quit = 0, result, i;
  int fieldLoop;
  int startCount, runningCount;
  char recvBuf[150];
  struct stationData  *station;
  struct entry *pLast;

  serverTask = NULL;

  FD_ZERO(&readFds);
  FD_ZERO(&writeFds);
  FD_ZERO(&exceptFds);

  serverState = IDLE;

  pServerCurrent = pServerHead;

  for(;;){
    pthread_mutex_lock(&serverQueueLock);
    // puts("doServer waiting for work");
    while((currQueueSize == 0) && !serverShutdown)
      pthread_cond_wait(&serverQueueNotEmpty, &serverQueueLock);

    if(serverShutdown){
      // puts("Exiting doServer()");
      if(pListHead)
	flushEntryList(pListHead);	
      pthread_mutex_unlock(&serverQueueLock);
      pthread_exit(NULL);
    }

    pCommand = pServerQueueHead;
    currQueueSize--;
    if(currQueueSize == 0)
      pServerQueueHead = pServerQueueTail = NULL;
    else
      pServerQueueHead = pCommand->next;

    // printf("currQueueSize = %d.\n", currQueueSize);

    pthread_mutex_unlock(&serverQueueLock);
    // printf("Server is doing %d: %s.\n", pCommand->command,
    //   sServerState[pCommand->command]);

    if(pCommand->command == LOGON){
      serverTask = (void *)sendLogon;
      serverState = LOGON;
      quit = FALSE;
      openSocket();
    }
    else if(pCommand->command == LOGOFF){
      serverTask = (void *)sendLogoff;
      serverState = LOGOFF;
      quit = FALSE;
      openSocket();
    }
    else if(pCommand->command == MAKE_BUSY){
      serverTask = (void *)makeMeBusy;
      serverState = MAKE_BUSY;
      quit = FALSE;
      openSocket();
    }
    else if(pCommand->command == GETCALLS){
      serverTask = (void *)getCalls;
      serverState = GETCALLS;
      quit = FALSE;
      openSocket();
    }

    while(!quit){
 
      testReadFds = readFds;
      testWriteFds = writeFds;
      testExceptFds = exceptFds;

      if(doTimeout)
	result = select(FD_SETSIZE, &testReadFds, &testWriteFds,
			&testExceptFds, &timeout);
      else
	result = select(FD_SETSIZE, &testReadFds, &testWriteFds,
			&testExceptFds, NULL);

      if(!result){   // timeout
	if(serverPortState == CONNECTING){
	  (void) time(&theTime);
	  tm_ptr = localtime(&theTime);
	  strftime(logTime, 20, "%H:%M:%S", tm_ptr);
	  // printf("%s Server connect timeout to %s.\n", 
	  // logTime, pServerCurrent->domainName);
	  FD_CLR(serverSocket, &exceptFds);
	  FD_CLR(serverSocket, &writeFds);
	  close(serverSocket);
	  serverPortState == CLOSED;
	  if(serverShutdown)
	    quit = 1;
	  else{
	    pServerCurrent = pServerCurrent->next;
	    openSocket();
	  }
	}
	doTimeout = 0;
      }

   
      if(FD_ISSET(serverSocket, &testExceptFds)){
	(void) time(&theTime);
	tm_ptr = localtime(&theTime);
	strftime(logTime, 20, "%H:%M:%S", tm_ptr);
	//printf("%s In exception, error, %d: %s.\n", 
	//     logTime, errno, strerror(errno));
	FD_CLR(serverSocket, &exceptFds);
	(void) time(&theTime);
	tm_ptr = localtime(&theTime);
	strftime(logTime, 20, "%H:%M:%S", tm_ptr);
	puts(logTime);
	doTimeout = 0;
      }

      if(FD_ISSET(serverSocket, &testWriteFds)){
	serverPortState = CONNECTED;
	FD_CLR(serverSocket, &writeFds);
	serverTask();
      }

      if(serverState != IDLE){    
	if(FD_ISSET(serverSocket, &testReadFds)){
	  if(serverState == LOGON ||
	     serverState == MAKE_BUSY ||
	     serverState == LOGOFF){
	    fgets(recvBuf, 149, serverFd);
	    /************
	    if(serverState == LOGOFF){
	      pthread_cond_broadcast(&serverLoggedOff);
	      puts("we're logging off");
	    }
	    ***************/
	    FD_CLR(serverSocket, &readFds);
	    FD_CLR(serverSocket, &exceptFds);
	    close(serverSocket);
	    // puts(recvBuf);
	    if(serverState == LOGOFF){
	      pthread_cond_broadcast(&serverLoggedOff);
	    }
	    serverState = IDLE;
	    serverTask = NULL;
	    quit = TRUE;
	  }
	  else if(serverState == GETCALLS){
	    if(getCallState == READ_RECORDS){
	      do{
		fgets(recvBuf, 149, serverFd);
		recvBuf[strlen(recvBuf)-1] = 0x00;
		if(fieldLoop == 0)
		  pStation = malloc(sizeof (struct stationData));
		switch(fieldLoop){
		case 0 : strcpy(pStation->call, recvBuf);
		  break;
		case 1 : strcpy(pStation->data, recvBuf);
		  break;
		case 2 : pStation->id = atoi(recvBuf);
		  break;
		case 3 : strcpy(pStation->ip, recvBuf);
		}
		fieldLoop++;
		if(fieldLoop == 4){
		  // printf("%s %s %d %s\n",pStation->call,
		  //	 pStation->data, pStation->id, pStation->ip);
		  fieldLoop = 0;
		  runningCount++;

		  while(pStation->call[strlen(pStation->call)-1] ==' ')
		    pStation->call[strlen(pStation->call)-1] = 0x00;  
		  if(strstr(pStation->call, "*"))
		    entryPtrs[3] = addEntryList(entryPtrs[3], pStation);
		  else if(strstr(pStation->call, "-")){
		    if(pStation->call[strlen(pStation->call)-1] == 'R')
		      entryPtrs[0] = addEntryList(entryPtrs[0], pStation);
		    else if(pStation->call[strlen(pStation->call)-1] == 'L')
		      entryPtrs[1] = addEntryList(entryPtrs[1], pStation);
		  }
		  else if(pStation->call[0] == ' ')
		    entryPtrs[4] = addEntryList(entryPtrs[4], pStation);
		  else 
		    entryPtrs[2] = addEntryList(entryPtrs[2], pStation);
		}
		if(runningCount == recordCount){
		  pListHead = NULL;
		  i = 0;
		  while((pListHead == NULL) &&
			(i < 5)){
		    if(i < 5){
		      if(entryPtrs[i] != NULL){
			pListHead = entryPtrs[i];
			i++;
		      }
		      else
			i++;
		    }
		  }
		  if(pListHead){
		    while(i<5){
		      if(entryPtrs[i]){
			pLast = findEnd(pListHead);
			pLast->next = entryPtrs[i];
		      }
		      i++;
		    }
		  }
		  finishList();
		  pthread_mutex_unlock(&serverHeadLock);
		  FD_CLR(serverSocket, &readFds);
		  FD_CLR(serverSocket, &exceptFds);
		  fclose(serverFd);
		  // close(serverSocket);
		  serverState = IDLE;
		  serverTask = NULL;
		  quit = TRUE;
		  // printf("Wanted %d records and got %d records.\n",
		  // startCount, runningCount);
		  i = 0;
		}  
		else
		  ioctl(serverSocket, FIONREAD, &i);
	      }while(i>0);

	    } else if(getCallState == NUMBER_RECORDS){
	      fgets(recvBuf, 149, serverFd);
	      // puts("Reading number of records");
	      recordCount = atoi(recvBuf);
	      startCount = recordCount;
	      pthread_mutex_lock(&serverHeadLock);
	      flushEntryList(pListHead);
	      runningCount = 0;
	      getCallState = READ_RECORDS; 
	      fieldLoop = 0;
	    } else if(getCallState == PREAMBLE){
	      fgets(recvBuf, 149, serverFd);
	      //puts("Reading preamble");
	      getCallState = NUMBER_RECORDS;
	    }
	  }
	}
      }
    }
  }
} 





















