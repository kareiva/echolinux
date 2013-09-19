/*****************************

Copyright 2002 Jeff Pierce wd4nmq.

This software is covered by the included GNU Public License, GPL.

$Log$

****************************/ 

#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <forms.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>

#include "testgui.h"
#include "serverglobals.h"
#include "threadFuncs.h"
#include "../echolinux/echoGlobals.h"

extern int controlInPipe[2], controlOutPipe[2];
extern pid_t controlPid;
extern pthread_mutex_t browserLock;

extern FD_testgui *fd_testgui;
extern FD_searchForm *fd_searchForm;
extern FILE *controlInDesc;

int  connected = 0;
char infoLine[1024];
char connectedStation[20];

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

/**********************************/

void getList(FL_OBJECT *ob, long data)
{

  fl_deactivate_object(fd_testgui->nodeList);
  //  strcpy(line, "Attempting server ");
  //  strcat(line, server);
  //  fl_set_object_label(fd_testgui->serverInfo, line);
  fl_set_object_color(fd_testgui->nodeList,FL_RED,FL_GREEN);

  sendServerCommand(GETCALLS);
}

/******************************/

void finishList(void){ 
  struct entry  *temp;
  char line[132];
  int  topLine;

  pthread_mutex_lock(&browserLock);
  topLine = fl_get_browser_topline(fd_testgui->nodeList);
  fl_freeze_form(fd_testgui->testgui);
  fl_clear_browser(fd_testgui->nodeList);
  if(pListHead){
    temp = pListHead;
    do{
      sprintf(line, "%-9s%-39s%6d", (temp->station)->call,
            (temp->station)->data, (temp->station)->id);
      fl_add_browser_line(fd_testgui->nodeList, line);
      temp = temp->next;
    }while(temp != NULL);
    fl_set_browser_topline(fd_testgui->nodeList, topLine);
  }
  fl_set_object_color(fd_testgui->nodeList,FL_WHITE,FL_GREEN);
  fl_set_object_label(fd_testgui->serverInfo, "");
  fl_activate_object(fd_testgui->nodeList);
  fl_unfreeze_form(fd_testgui->testgui);
  pthread_mutex_unlock(&browserLock);
}

/*** callbacks and freeobj handles for form testgui ***/

void getConnection(FL_OBJECT *ob, long data)
{
  char line[80];

  //  strcpy(line, "C ");
  line[0] = CONNECT;
  line[1] = ' ';
  line[2] = 0x00;
  strcat(line, fl_get_input(ob));
  strcat(line, "\n");
  fl_addto_browser(fd_testgui->displayData, line);
  write(controlOutPipe[1], line, strlen(line));
  fl_set_input(ob, "");
  fl_set_focus_object(fd_testgui->testgui, fd_testgui->mainWindow);

}

/*********************************/

void typedData(FL_OBJECT *ob, long data)
{
  char line[80];

  line[0] = CHAT;
  line[1] = 0x00;
  strcat(line, fl_get_input(ob));
  strcat(line, "\n");
  // fl_addto_browser(fd_testgui->displayData, line);
  write(controlOutPipe[1], line, strlen(line));
  fl_set_input(ob, "");
  fl_set_focus_object(fd_testgui->testgui, ob);

}

/*********************************/

void displayData(FL_OBJECT *ob, long data)
{

}

/*********************************/

void nodeList(FL_OBJECT *ob, long data)
{

}

/*********************************/

void voxToggle(FL_OBJECT *ob, long data)
{
  char line[5];

  if(fl_get_button(ob))
    strcpy(line, "VN\n");
  else
    strcpy(line, "VF\n");

  write(controlOutPipe[1], line, strlen(line));


}

/*********************************/

void strengthToggle(FL_OBJECT *ob, long data)
{
  char line[5];

  if(fl_get_button(ob))
    strcpy(line, "SN\n");
  else
    strcpy(line, "SF\n");

  write(controlOutPipe[1], line, strlen(line));

}

/*********************************/

void voxThreshold(FL_OBJECT *ob, long data)
{
  double  value;
  int     x;
  char line[40];

  value = fl_get_slider_value(ob);
  value = value - 100.0;
  value =(value+90.309)/20.0; 
  value = pow(10.0, value);
  x = rint(value);
  sprintf(line, "VT %d\n", x);
  write(controlOutPipe[1], line, strlen(line));
}

/*********************************/

void dblNodeList(FL_OBJECT *ob, long data)
{
  int i, lineNo;
  struct entry *temp;
  char line[132];

  lineNo = fl_get_browser(ob);

  temp = pListHead;
  if(lineNo != 1)
    for(i=1; i<lineNo; i++){
      temp = temp->next;
    }
/********* 
   sprintf(line, "%-9s%-39s%6s %s", (temp->station)->call,
            (temp->station)->data, (temp->station)->id, (temp->station)->ip);
    puts(line);
*****/

  line[0] = CONNECT;
  line[1] = ' ';
  line[2] = 0x00;
  strcat(line, (temp->station)->ip );
  strcat(line, "\n");
  fl_addto_browser(fd_testgui->displayData, line);
  write(controlOutPipe[1], line, strlen(line));
  fl_set_focus_object(fd_testgui->testgui, fd_testgui->mainWindow);
}

/*******************************/

void sayGoodbye(FL_OBJECT *ob, long data)
{
  char line[80];

  line[0] = DISCONNECT;
  line[1] = '\n';
  line[2] = 0x00;
  fl_addto_browser(fd_testgui->displayData, "Sending disconnect");
  write(controlOutPipe[1], line, strlen(line));
}

/*******************************/

void PTT(FL_OBJECT *ob, long data)
{
  char line[80];

  strcpy(line, " \n");
  // printf("Sending PTT");
  write(controlOutPipe[1], line, strlen(line));
}

/**********************************/

void mainWindow(FL_OBJECT *ob, long data)
{
  fl_set_focus_object(fd_testgui->testgui, ob);
}

/**********************************/
void readPipe(int fd, void *data){

  char line[1024], typeLine[20], *ptr;
  int  i, k;
  char keeper, *pos;
  double value;

  fgets(line, 1023, controlInDesc);
  //  if(line[0] != 'S')
  //  fputs(line, stdout);
  if(line[0] == CONNECT){
    fl_set_object_label(fd_testgui->ConnectedNode, &line[2]);
    i = strstr(&line[2]," ") - &line[2];
    if(i > strlen(&line[2]))
       i = strlen(&line[2]);
    connectedStation[i] = 0x00;
    strncpy(connectedStation,&line[2], i);
    if(!connected){
      strcpy(typeLine, "SN\n");
      write(controlOutPipe[1], typeLine, strlen(line));
      voxThreshold(fd_testgui->voxThreshold, 0);
      sendServerCommand(MAKE_BUSY);
      fl_set_timer(fd_testgui->timer1, LOGION_TIMEOUT);
    }
    connected = 1;
  }  else if(line[0] == TRANSMIT){
    fl_show_object(fd_testgui->txIndicator);
  } else if(line[0] == RECEIVE){
    fl_hide_object(fd_testgui->txIndicator);
  } else if((line[0] == CHAT) || (line[0] == ECHO_ERROR)){

    // Need to break up long lines because xforms
    // browser display doesn't wrap. 

    line[strlen(line)-1] = 0x00;
   if(strlen(line) <= 37)
      fl_addto_browser(fd_testgui->displayData, &line[2]);
    else {
      pos = &line[2];
      while(strlen(pos) > 37){
	  keeper = pos[36];
	  pos[36] = 0x00;
	  fl_addto_browser(fd_testgui->displayData, pos);
	  pos = strlen(line) + line;
	  *pos = keeper;	  
      }
      fl_addto_browser(fd_testgui->displayData, pos);
    }
  } else if(line[0] == DISCONNECT){
    sendServerCommand(LOGON);
    fl_set_timer(fd_testgui->timer1, LOGION_TIMEOUT);
    fl_addto_browser(fd_testgui->displayData,"Got Disconnect");
    fl_set_object_label(fd_testgui->infoData, "");
    fl_set_object_label(fd_testgui->ConnectedNode, "");
    fl_hide_object(fd_testgui->txIndicator);
    fl_set_button(fd_testgui->voxButton, 0);
    getList(NULL, 0);
    fl_set_timer(fd_testgui->listTimer, GETLIST_TIMEOUT);
    connected = 0;
  } else if(line[0] == INFO){
    ptr = &line[2];
    while(ptr != NULL){
      ptr = strchr(ptr, '\r');
      if(ptr != NULL)
	*ptr = '\n';
    }
    fl_set_object_label(fd_testgui->infoData, &line[2]);
  } else if(line[0] == STRENGTH){
    // printf("%s", line);
    //    value = (double)(atoi(&line[2])/32765.0*100.0);
    value = 20.0 * (log10 ((1.0/32767.0)*atoi(&line[2])));
    value = 100 + value;
    //    printf("Strength = %f.\n", value);
    fl_set_slider_value(fd_testgui->strength, value); 
  }
}

/**********************/
int atClose(FL_FORM *form, void *data)
{
  int     error;
  char    line[3];
  struct timeval timenow;
  struct timespec timeout;

  //  kill(controlPid, SIGKILL);
  //  while(!waitpid(controlPid, 0, WNOHANG));

  line[0] = QUIT;
  line[1] = '\n';
  write(controlOutPipe[1], line, 2);
  
  gettimeofday(&timenow, NULL);
  timeout.tv_sec = timenow.tv_sec + 5;
  timeout.tv_nsec = 0;
  pthread_mutex_lock(&serverLoggedOffLock);
  sendServerCommand(LOGOFF);
  pthread_cond_timedwait(&serverLoggedOff, 
  	    &serverLoggedOffLock,
  	    &timeout); 

  destroyApp();
  return(FL_OK);
}

/**********************/

void timer1Call(FL_OBJECT *ob, long data)
{

  if(connected)
    sendServerCommand(MAKE_BUSY);
  else
    sendServerCommand(LOGON);

  fl_set_timer(ob, LOGION_TIMEOUT);
}

/**********************/

void listTimerCB(FL_OBJECT *ob, long data)
{

  if(!connected)
    getList(NULL, 0);
 
   fl_set_timer(ob, GETLIST_TIMEOUT);
}

/**********************/

void searchCall(FL_OBJECT *ob, long data)
{
  struct entry *temp;
  int counter;
  char *input;

  // printf("In searchCall(), data = %d\n", data);
  fl_hide_form(fd_searchForm->searchForm);

  if(data == 1){
    input = fl_get_input(ob);
    // puts(input);
    /*****************
    counter = 0;
    pthread_mutex_lock(&serverHeadLock);
    temp = pListHead;
    while((temp->next != NULL) &&
      (strstr((temp->station)->call, input) != NULL)){
      temp = temp->next;
      counter++;
    }
    if(temp)
      fl_select_browser_line(fd_testgui->nodeList, counter);
    pthread_mutex_unlock(&serverHeadLock);
    ********************/
  }
}

/**********************/

int searchClose(FL_FORM *form, void *data){

  // puts("In searchclose()");
  fl_hide_form(fd_searchForm->searchForm);
}

/**********************/

void showSearch(FL_OBJECT *ob, long data)
{
  /* fill-in code for callback */
  fl_show_form(fd_searchForm->searchForm, FL_PLACE_FREE, 
	       FL_FULLBORDER, "Search");
}














