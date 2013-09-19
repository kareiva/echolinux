/*****************************

Copyright 2002 Jeff Pierce wd4nmq.

This software is covered by the included GNU Public License, GPL.

$Log$

****************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <forms.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "testgui.h"
#include "serverglobals.h"
#include "threadFuncs.h"

#define VERSION "EchoLinux Version 0.17a"
#define VOXTHRESHOLD 1600

extern void readPipe(int, void *);
extern int atClose(FL_FORM *, void *);
extern void dblNodeList(FL_OBJECT *, long);
extern void getlist(FL_OBJECT *, long);
extern int searchClose(FL_FORM *, void *);

FILE *gpfd;

char callsign[40], name[40], location[40], password[20];
char server[80];
char echoLinuxDefaults[80];
char line[80];
int controlInPipe[2], controlOutPipe[2];
pid_t controlPid;
FD_testgui *fd_testgui;
FD_searchForm *fd_searchForm;

pthread_mutex_t browserLock;
struct timeval timeout;

FILE *controlInDesc;

int main(int argc, char *argv[])
{

  int vol;
  char *home;
  double value;

  // XInitThreads();

  home = getenv("HOME");
  strcpy(echoLinuxDefaults, home);
  strcat(echoLinuxDefaults, "/.echoLinux/");
  strcpy(line, echoLinuxDefaults);
  gpfd = fopen(strcat(line, "userdata.txt"), "r");
  if(!gpfd){
	  gpfd = fopen("/etc/echolinux/userdata.txt", "r");
	  if(!gpfd){
	    perror("userdata.txt");
	    exit(1);
	  }
  }

  fgets(callsign, 40, gpfd);
  callsign[strlen(callsign)-1] = 0x00;
  fgets(name, 40, gpfd);
  name[strlen(name)-1] = 0x00;
  fgets(location, 40, gpfd);
  location[strlen(location)-1] = 0x00;
  fgets(password, 40, gpfd);
  password[strlen(password)-1] = 0x00;
  fclose(gpfd);
 
  serverInit();
  serverThreadInit();

  sendServerCommand(LOGON);

  

  fl_initialize(&argc, argv, 0, 0, 0);
  fd_testgui = create_form_testgui();
  fd_searchForm =create_form_searchForm();
   fl_set_form_atclose(fd_searchForm->searchForm, searchClose, NULL);
 
  fl_set_timer(fd_testgui->timer1, LOGION_TIMEOUT);

  if((pipe(controlInPipe)== 0) && (pipe(controlOutPipe) == 0)){
    //    puts("Past pipe commands.");
    //puts("Forking control app.");
   controlPid = fork();
    if(controlPid == 0){

      close(fileno(stdin));
      dup(controlOutPipe[0]);
      close(controlOutPipe[1]);
      close(controlOutPipe[0]);
      close(fileno(stdout));
      dup(controlInPipe[1]);
      close(controlInPipe[1]);
      close(controlInPipe[0]);

      execlp("echolinux", "echolinux", 0, 0);
      printf("We didn't execute execlp().\n");
    }
  }
  else{
    perror("pipe");
    exit(1);
  }
  //  printf("controlPid = %d.\n", controlPid);
  close(controlInPipe[1]);
  close(controlOutPipe[0]);

  controlInDesc = fdopen(controlInPipe[0], "r");
  if(!controlInDesc){
    perror("fdopen");
    exit(1);
  }

  // serverInit();
  // serverThreadInit();

  /* fill-in form initialization code */
  fl_add_io_callback(controlInPipe[0], FL_READ, readPipe, 0);
  
  fl_set_browser_hscrollbar(fd_testgui->displayData, FL_OFF);
  fl_set_browser_fontsize(fd_testgui->displayData, ChatFont);
  fl_set_browser_fontsize(fd_testgui->nodeList, NodeFont);
  fl_set_object_lsize(fd_testgui->infoData, InfoFont);
  /*********
  fl_set_browser_fontsize(fd_testgui->displayData, FL_SMALL_SIZE);
  fl_set_browser_fontsize(fd_testgui->nodeList, FL_TINY_SIZE);
  fl_set_object_lsize(fd_testgui->infoData, FL_TINY_SIZE);
  ****/
  fl_set_browser_dblclick_callback(fd_testgui->nodeList, dblNodeList, 0); 
  value = 20.0 * (log10 ((1.0/32767.0)*VOXTHRESHOLD));
  value = 100 + value;
  fl_set_slider_value(fd_testgui->voxThreshold, value); 
  fl_set_focus_object(fd_testgui->testgui, fd_testgui->mainWindow);
  fl_hide_object(fd_testgui->txIndicator);
  fl_set_atclose(atClose, 0);
  //  getList(NULL, 0);
  /* show the first form */
  fl_show_form(fd_testgui->testgui,FL_PLACE_CENTERFREE,
	       FL_FULLBORDER, VERSION);
  // fl_add_browser_line(fd_testgui->nodeList, "Getting User List");
  getList(NULL, 0);
  fl_set_timer(fd_testgui->listTimer, GETLIST_TIMEOUT);
  // fl_do_forms();

  while(1){
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
    select(0, NULL, NULL, NULL, &timeout);
    pthread_mutex_lock(&browserLock);
    fl_check_forms();
    pthread_mutex_unlock(&browserLock);
  }

  atClose(NULL, NULL);
  return 0;
}






