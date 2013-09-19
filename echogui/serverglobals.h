#define MAXCALL 15
#define MAXDATA 45
#define MAXID   15
#define MAXIP   20

#define LOGION_TIMEOUT 360.0
#define GETLIST_TIMEOUT 210.0

/*********************
struct serverList {
  char domainName[80];
  struct in_addr addr;
  struct serverList *next;
};
*********************/

struct recordLineT{
  char line[4][80];
};

struct stationData {
  char  call[MAXCALL];
  char  data[MAXDATA];
  int   id;
  char  ip[MAXIP];
};

enum serverStateT{
  IDLE, LOGON, LOGOFF, GETCALLS, MAKE_BUSY
};

/*****************
char *sServerState[]={
  "IDLE",
  "LOGON",
  "LOGOFF",
  "GETCALLS",
  "MAKE BUSY"};
*****************/

enum serverPortStateT{
  CLOSED, CONNECTING, CONNECTED
};

enum getCallStatT{
  PREAMBLE, NUMBER_RECORDS, READ_RECORDS
};

struct entry {
  struct stationData  *station;
  struct entry        *next;
};

extern void addServerList(char *);
extern struct stationData *getEntry(int);
extern int doServer(void *);
extern void printList(struct entry *);

int sendLogon(void);
int sendLogoff(void);
int makeMeBusy(void);
int getCalls(void);
int handleServer(void);

extern enum serverStateT serverState;
extern char callsign[], name[], location[], password[];
extern struct serverList *pServerHead, *pServer;
extern int serverSocket;
extern struct entry *pListHead;


















