#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include "serverglobals.h"
#include "threadFuncs.h"

int maxQueueSize = 10;
int currQueueSize = 0;
pServerQueueEntryT  pServerQueueHead = NULL;
pServerQueueEntryT  pServerQueueTail = NULL;
pthread_mutex_t serverQueueLock;
pthread_mutex_t serverHeadLock;
pthread_mutex_t serverLoggedOffLock;
pthread_cond_t serverLoggedOff;
pthread_cond_t  serverQueueNotEmpty;
pthread_cond_t  serverQueueNotFull;
pthread_cond_t  serverQueueEmpty;
int serverQueueClosed = 0;
int serverShutdown = 0; 


pthread_t serverThread;

/***********************/

void destroyApp(void){

  pServerQueueEntryT pTemp;

  serverShutdown = 1;
  pthread_mutex_unlock(&serverQueueLock);
  pthread_cond_broadcast(&serverQueueNotEmpty);
  // puts("Waiting for server to shutdown");
  pthread_join(serverThread, NULL);
  // puts("Server has shutdown");
  while(pServerQueueHead){
    pTemp = pServerQueueHead->next;
    pServerQueueHead = pServerQueueHead->next;
    free(pTemp);
  }
}

/************************/

void sigHandler(int sig){

  // puts("In signal handler");
  if(sig = SIGINT){
    // puts("It is a SIGINT");
    destroyApp();
    exit(0);
  }
}

/*************************/

int sendServerCommand(enum serverStateT command){

  pServerQueueEntryT pServerCommand;

  pthread_mutex_lock(&serverQueueLock);

  if(currQueueSize == maxQueueSize){
    pthread_mutex_unlock(&serverQueueLock);
    return -1;
  }

  while(currQueueSize == maxQueueSize){
    pthread_cond_wait(&serverQueueNotFull, &serverQueueLock);
  }

  /* allocate a work structure */

  pServerCommand = (pServerQueueEntryT) malloc(sizeof(serverQueueEntryT));  

  pServerCommand->command = command;
  pServerCommand->next = NULL;

  if(currQueueSize == 0){
    pServerQueueHead = pServerQueueTail = pServerCommand;
    pthread_cond_broadcast(&serverQueueNotEmpty);
  }
  else {
    pServerQueueTail->next = pServerCommand; 
    pServerQueueTail = pServerCommand;
  }
  currQueueSize++;
  // printf("currQueueSize = %d.\n", currQueueSize);
  pthread_mutex_unlock(&serverQueueLock);
  return(1);
}

/********************************/

int serverThreadInit(void){

  int rtn, quit = 0;

  (void) signal(SIGINT, sigHandler);

  if((rtn = pthread_mutex_init(&serverQueueLock, NULL)) != 0)
     fprintf(stderr, "pthread_mutex_init %s", strerror(rtn)), exit(-1);
  if((rtn = pthread_mutex_init(&serverLoggedOffLock, NULL)) != 0)
     fprintf(stderr, "pthread_mutex_init %s", strerror(rtn)), exit(-1);
  if((rtn = pthread_cond_init(&serverLoggedOff, NULL)) != 0)
     fprintf(stderr, "pthread_cond_init %s", strerror(rtn)), exit(-1);
  if((rtn = pthread_cond_init(&serverQueueNotEmpty, NULL)) != 0)
     fprintf(stderr, "pthread_cond_init %s", strerror(rtn)), exit(-1);
  if((rtn = pthread_cond_init(&serverQueueNotFull, NULL)) != 0)
     fprintf(stderr, "pthread_cond_init %s", strerror(rtn)), exit(-1);
  if((rtn = pthread_cond_init(&serverQueueEmpty, NULL)) != 0)
     fprintf(stderr, "pthread_cond_init %s", strerror(rtn)), exit(-1);

  // puts("Launch the server thread.");

  rtn = pthread_create(&serverThread,
		       NULL,
		       (void *) &doServer,
		       NULL);

  if(rtn != 0)
    fprintf(stderr, "pthead_create %d", rtn), exit(-1);

}





