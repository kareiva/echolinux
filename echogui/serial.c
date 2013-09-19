#define _POSIX_SOURCE 1

#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFSIZE 256

struct termios t;
static int chan = -1;             /* I/O Descriptor for the
                                   * terminal port.
                                   */

/*
 * Setup the communications port
 */
int comm_init(const char *device)
{
  struct termios t;


  chan = open(device, O_RDWR|O_NOCTTY);
  if (chan == -1){
    perror("Openning serial port.");
    return(-1);
  }
  if (tcgetattr(chan, &t) != 0) return(-1);
  t.c_cc[VMIN] = 1;            /* Wake up after 32
                                 * characters arrive.
                                 */
  t.c_cc[VTIME] = 1;            /* Wake up 0.1 seconds
                                 * after the first char
                                 * arrives.
                                 */
  /* The combination of 
   * VMIN/VTIME will cause
   * the program to wake up
   * 0.1 seconds after the
   * first character arrives
   * or after 32 characters 
   * arrive whichever comes
   * first.
   */
  t.c_iflag &= ~(BRKINT         /* Ignore break       */
                 | IGNPAR | PARMRK |          /* Ignore parity      */
                 INPCK |                  /* Ignore parity      */
                 ISTRIP |                 /* Don't mask         */
                 INLCR | IGNCR | ICRNL         /* No <cr> or <lf>    */
                 | IXON);                     /* Ignore STOP char   */
  t.c_iflag |= IGNBRK | IXOFF;  /* Ignore BREAK
                                 * send XON and XOFF for
                                 * flow control.
                                 */
  t.c_oflag &= ~(OPOST);        /* No output flags     */
  t.c_lflag &= ~(               /* No local flags.  In */
                 ECHO|ECHOE|ECHOK|ECHONL| /* particular, no echo */
                 ICANON |                 /* no canonical input  */
                 /* processing,         */
                 ISIG |                  /* no signals,         */
                 NOFLSH |                /* no queue flush,     */
                 TOSTOP);                /* and no job control.*/
  t.c_cflag &= (                        /* Clear out old bits  */
                CSIZE |                 /* Character size      */
                CSTOPB |                /* Two stop bits       */
                HUPCL                  /* Hangup on last close*/
                );                /* no Parity              */
  t.c_cflag |= CLOCAL | CREAD | CS8;
  /* CLOCAL => No modem
   * CREAD  => Enable
   *           receiver
   * CS8    => 8-bit data
   */

  /* Copy input and output speeds into
   * struct termios t
   */
  if (cfsetispeed(&t, B2400) == -1) return(-1);
  if (cfsetospeed(&t, B2400) == -1) return(-1);

  /* Throw away any input data (noise) */
  if (tcflush(chan, TCIFLUSH) == -1) return(-1);

  /* Now, set the termial port attributes */
  if (tcsetattr(chan,TCSANOW, &t) == -1) return(-1);
  
  return(chan);
}



