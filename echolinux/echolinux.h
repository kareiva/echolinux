#include <termios.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define PORT_BASE 5198


struct termios term_params, old_term_params;

void raw_tty(int flag){

  if(flag){
  tcgetattr(fileno(stdin), &old_term_params);
  term_params = old_term_params;
  cfmakeraw(&term_params);
  tcsetattr(fileno(stdin), TCSAFLUSH, &term_params);
  } else{
  tcsetattr(fileno(stdin), TCSAFLUSH, &old_term_params);
  }
}


struct gsmVoice_t {
  unsigned char version;
  unsigned char pt;
  unsigned short seqNum;
  unsigned long time;
  unsigned long ssrc;
  unsigned char data[33*4];
};



