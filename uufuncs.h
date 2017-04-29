/*
** Author:	Neil Cherry
** Date:	11/25/99
** Version:	1.6 (alpha)
**
**      $Id: uufuncs.h,v 1.1.1.1 2008/09/24 19:04:12 ncherry Exp $
**
**      $Log: uufuncs.h,v $
**      Revision 1.1.1.1  2008/09/24 19:04:12  ncherry
**      Imported using TkCVS
**
**      Revision 1.1.1.2  2000/03/18 14:22:05  ncherry
**
**      Added hcs_dl.c which is a standalone program to download an events.bin file
**      to an HCS II connected to a local serial port.
**
**      Revision 1.1.1.1  2000/03/13 13:06:00  ncherry
**      This is the inital base release. It is nothing more that some of my (njc)
**      existing code from my HCS II project. It is the code without the pthread
**      functions. It has problems with character drops on the receive serial port.
**
**
**      Revision 1.6  1999/11/25 17:32:14  njc
**      This file contains  the definitions need for the  uucp locking code in
**      uucp_locking.c.
**
*/

/*
** function prototypes
*/

#include <sys/wait.h>

#ifndef UUFUNC
extern int	makelock();
extern int	readlock();
extern int	checklock();
#else
int	makelock();
int	readlock();
int	checklock();
#endif

/*
** globals
*/

extern char	*lock;
extern int	errno;

#define TRUE	1
#define FALSE	!TRUE

#define SUCCESS (0)
#define FAIL	(-1)

#define MAXLINE	8192

#ifndef LOCK
  #ifdef __linux__
    #define LOCK	"/var/lock/LCK..%s"
  #else
    #define LOCK	"/usr/spool/uucp/LCK..%s"
  #endif
#endif

#define FOUND	 TRUE	/* Lock file has been found and is a dead process */
#define OURS	 (-2)	/* Lock file has been found and it is our process */
#define DEAD	 (-3)	/* Lock file has been found and is a dead process */
#define DONTKNOW errno	/* Lock file has been found and I don't know what */
#define LIVE	 TRUE	/* Lock file has been found and I don't know what */
#define NOT_FOUND (0)	/* Lock file has not been found                   */
