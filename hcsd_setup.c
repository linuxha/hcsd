/*
**
** Author:      Neil Cherry
** Date:        10/31/99
** Version:     1.1 (sub alpha)
**
** $Id: hcsd_setup.c,v 1.1.1.1 2008/09/24 19:04:12 ncherry Exp $
**
** $Log: hcsd_setup.c,v $
** Revision 1.1.1.1  2008/09/24 19:04:12  ncherry
** Imported using TkCVS
**
** Revision 1.1.1.2  2000/03/18 14:22:03  ncherry
**
** Added hcs_dl.c which is a standalone program to download an events.bin file
** to an HCS II connected to a local serial port.
**
** Revision 1.1.1.1  2000/03/13 13:05:58  ncherry
** This is the inital base release. It is nothing more that some of my (njc)
** existing code from my HCS II project. It is the code without the pthread
** functions. It has problems with character drops on the receive serial port.
**
**
** Revision 1.6  1999/11/25 17:32:14  njc
** This file contains the function  to initialize, daemonize and open the
** HCS II device.  Later I may  add the routine to fork the read function
** for the HCS II. I still need to add the uucp lock code to this file.
**
** Revision 1.1  1999/10/31 17:05:05  njc
** Initial revision
**
**
*/

#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#ifdef UUCP
#include <stdlib.h>

#include "uufuncs.h"

extern int checklock(char *name);
extern int makelock(char *name);

char *lock;

#endif

extern void fini(int);

//extern int errno;
extern int on;

/*
** Housekeeping to startup the hcsd
*/

int
hcsd_setup(char *line)
{
  struct termios oldsb, newsb;

  int i, device;
  
#ifdef UUCP
  char *p;
#endif

  /*
  ** When this program comes out of Alpha I will enable these
  ** I've noticed some wierd behavior and having it print to stdout
  ** can be very useful (though disconcerting).
  */

  #ifdef NJC
  close(STDIN);
  close(STDOUT);
  close(STDOUT);
  #endif

  /*
  ** This should keep umount from saying that the device is busy
  */

  if(chdir("/")) {
    syslog(LOG_CRIT, "Can not cd to '/': %m");
    exit(errno);
  }

  /*
  ** The child process will drop into the background
  ** While the parent return to the shell (run as a daemon).
  */

  if(fork())
    exit(0);

  /*
  ** This is the POSIX (I hope) way of disassociating the process
  ** with a controlling terminal
  */

  if(setsid() < 0) {
    syslog(LOG_CRIT, "setsid error: %m");
    exit(errno);
  }

  /*
  ** I should probably switch my user id over to "nobody.uucp"
  ** This is so I'm not running as root. However, I must be careful of
  ** permissions.
  */

#ifdef UUCP
  /*
  ** UUCP lock file handling
  */
  
  if((lock = malloc((sizeof(char) * MAXLINE)+1)) == NULL) {
    syslog(LOG_CRIT, "lock file malloc error: %m");
    exit(1);
  }

  /*
  ** We need a function which strips out the basename
  */

  p = rindex(line, '/');        /* Operate like strrpbrk, returns a ptr to the last / */
  p++;                          /* points to the rest of the str after the / */

  (void) sprintf(lock, LOCK, p); /* build the lock filename w/path */

  /*
  ** Check for a lock file and fail if already locked
  */

  switch(i = checklock(lock))
  {
    case !FOUND:
      if(makelock(lock) == FAIL) {
        syslog(LOG_ERR, "makelock failed");
        exit(1);
      }
      break;
    
    case FOUND:
      syslog(LOG_ERR, "Lock file has been found\n");
      exit(1);
      break;

    case OURS:
      syslog(LOG_ERR, "Lock file has been found and it is our process\n");
      unlink(lock);
      exit(1);
      break;

    case DEAD:
      if(unlink(lock))
      {
        syslog(LOG_ERR, "Lock file has been found and is a dead process\n");
        exit(errno);
      }
      else
      {
        makelock(lock);
      }
      break;

    default:
      syslog(LOG_ERR, "We got an errno code back (%03x)", i);
      exit(1);
      break;
  }

#endif
  
  if((device=open(line, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
    syslog(LOG_ERR, "%s: %m", line);
    exit(errno);
  }

  /*
  ** This works for heyu (NJC)
  */
 
  i = tcgetattr(device, &oldsb);
 
  if (i < 0) {
    syslog(LOG_CRIT, strerror(errno));
    exit(errno);
  }
 
  newsb = oldsb;
          
  newsb.c_iflag = IGNBRK | IGNPAR;
  newsb.c_oflag = 0;
  newsb.c_lflag = ISIG;
  newsb.c_cflag = (CLOCAL | B9600 | CS8 | CREAD);

  for (i = 0; i < NCC; i++)
    newsb.c_cc[i] = 0;

  newsb.c_cc[VMIN]   = 1;
  newsb.c_cc[VTIME]  = 0;
 
  tcsetattr(device, TCSADRAIN, &newsb);
  
  /*
  ** Various signals I need handled
  **
  ** I need to convert these over to sigaction and its family
  ** This will be more inline with POSIX.
  */
  
  signal(SIGPIPE, SIG_IGN);
  signal(SIGHUP,  fini);        /* needed for uucp lock files */
  signal(SIGSEGV, fini);        /* needed for uucp lock files */
  signal(SIGFPE,  fini);        /* needed for uucp lock files */
  signal(SIGABRT, fini);        /* needed for uucp lock files */
  signal(SIGTERM, fini);        /* needed for uucp lock files */
  signal(SIGCHLD, fini);        /* The SIO failed!            */

  return(device);
}

