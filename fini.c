/*
** Author:	Neil Cherry
** Date:	11/25/99
** Version:	1.6 (alpha)
**
**      $Id: fini.c,v 1.1.1.1 2008/09/24 19:04:12 ncherry Exp $
**
**      $Log: fini.c,v $
**      Revision 1.1.1.1  2008/09/24 19:04:12  ncherry
**      Imported using TkCVS
**
**      Revision 1.1.1.2  2000/03/18 14:21:59  ncherry
**
**      Added hcs_dl.c which is a standalone program to download an events.bin file
**      to an HCS II connected to a local serial port.
**
**      Revision 1.1.1.1  2000/03/13 13:05:54  ncherry
**      This is the inital base release. It is nothing more that some of my (njc)
**      existing code from my HCS II project. It is the code without the pthread
**      functions. It has problems with character drops on the receive serial port.
**
**
**      Revision 1.6  1999/11/25 17:32:13  njc
**      This code handle the shutdown of the daemon. Later it will contain the
**      code to clean up the uucp locking files plus whatever else may need to
**      be tidied up.
**
*/

#include <stdlib.h>
#include <syslog.h>
#include <signal.h>

#ifdef UUCP
#include <unistd.h>

extern char *lock;
#endif

void
fini(int sig)
{
  /*
  ** I'll need to add the code to remove the uucp lock file
  ** as well as shutting down the daemon cleanly
  */

  /*
  ** Done!
  */

  switch(sig) {
    case SIGHUP:
      syslog(LOG_DEBUG, "Fini");
      break;
      
    case SIGSEGV:
      syslog(LOG_DEBUG, "SIGSEGV");
      break;
      
    case SIGFPE:
      syslog(LOG_DEBUG, "SIGFPE - Floating point error");
      break;
      
    case SIGABRT:
      syslog(LOG_DEBUG, "SIGABRT - Core dump");
      break;

    case SIGTERM:
      syslog(LOG_DEBUG, "Terminated");
      break;

    default:
      syslog(LOG_DEBUG, "Unexpected signal (%d)", sig);
      break;
  }

#ifdef UUCP
  unlink(lock);
  free(lock);
#endif
  
  exit(0);
}

