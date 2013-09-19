
typedef struct serverQueueEntry {
  enum serverStateT command;
  struct serverQueueEntry *next;
} serverQueueEntryT, *pServerQueueEntryT;


extern int maxQueueSize;
extern int currQueueSize;
extern pServerQueueEntryT  pServerQueueHead;
extern pServerQueueEntryT  pServerQueueTail;
extern pthread_mutex_t serverQueueLock;
extern pthread_mutex_t serverHeadLock;
extern pthread_mutex_t serverLoggedOffLock;
extern pthread_cond_t  serverLoggedOff;
extern pthread_cond_t  serverQueueNotEmpty;
extern pthread_cond_t  serverQueueNotFull;
extern pthread_cond_t  serverQueueEmpty;
extern int serverQueueClosed;
extern int serverShutdown; 

extern int serverThreadInit(void);
extern int sendServerCommand(enum serverStateT);
extern void destroyApp(void);
