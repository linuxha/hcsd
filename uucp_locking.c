/*
**	uufuncs.c
**
**	functions needed for uucp style line locking.
**
**	Directly stolen from GETTY_PS by Kris Gleason.
**	Modified by Neil Cherry.
**
**	--
**
** Author:	Neil Cherry
** Date:	11/25/99
** Version:	1.6 (alpha)
**
**      $Id: uucp_locking.c,v 1.6 1999/11/25 17:32:14 njc alpha njc $
**
**      $Log: uucp_locking.c,v $
**      Revision 1.6  1999/11/25 17:32:14  njc
**      This file  contains the uucp locking  library I created back  in 95. I
**      haven't started playing with it yet.  It may need quite a bit of work.
**      I also need to find out where the current lock files are being kept.
**
**
** njc 09/10/95 adding to the library so that I can create programs to be used
**              by scripts. There is one problem though, I really need to do
**		better error checking. For instance, what if the lock file I'm
**		trying to look at has its permissions set such that I'm not
**		allowed to look at it.
*/

#define UUFUNC 1

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "uufuncs.h"

/*
** makelock
**
** attempt to make a lockfile
** Returns FAIL if lock could not be made (line in use).
*/

int makelock(name)
char *name;
{
  int fd, pid;
#if 0
  char *temp;
#endif
  char buf[MAXLINE+1];
  
  char apid[16];
  
  int getpid();
  
  errno = 0;
  
  /*
  ** first make a temp file
  */
  (void) sprintf(buf, LOCK, "TM.XXXXXX");
#if 0
  if ((fd = creat((temp=mktemp(buf)), 0444)) == FAIL) {
    fprintf(stderr,"create failed on temp lockfile \"%s\": %s",
	    temp, strerror(errno));
    return(FAIL);
  }
#else
  if((fd = mkstemp(buf)) == FAIL) {
    fprintf(stderr,"create failed on temp lockfile \"%s\": %s", buf, strerror(errno));
    return(FAIL);
  }
#endif
  /*
  ** put my pid in it
  */
  
  (void) sprintf(apid, "%09d", getpid());
  (void) write(fd, apid, strlen(apid));
  
  (void) close(fd);
  
  /*
  ** link it to the lock file
  */

  while (link(buf, name) == FAIL)
  {
    if (errno == EEXIST) 	/* lock file already there */
    {
      if ((pid = readlock(name)) == FAIL)
	continue;

      if ((kill(pid, 0) == FAIL) && errno == ESRCH)
      {
	/* pid that created lockfile is gone */
	(void) unlink(name);
	continue;
      }
    }
    (void) unlink(buf);
    return(FAIL);
  }
  (void) unlink(buf);
  return(SUCCESS);
}

/*
** checklock
**
** test for presense of valid lock file
**
** Returns FALSE (0) is no file is found, other values otherwise.
**
** Returns	if ...
** -----------  -----------------------------------------------------------
** !FOUND (0)	There is no such file.
** errno  (x)	The file exists but we have no permisions to unlock.
** OURS	  (-2)	The file exists and it belongs to us
** LIVE   (-3)	The file exists and it belongs to a live process.
** FOUND  (1)	The file exists.
*/

int checklock(name)
char *name;
{
  int pid, a;
  struct stat st;

  if ((a = stat(name, &st) == FAIL) && (errno == ENOENT))
  {
    return(NOT_FOUND);	/* File doesn't exist */
  }

  if ((pid = readlock(name)) == FAIL)
  {
    return(DONTKNOW);	/* For some reason file exist but we can't read it. */
  }

  if (pid == getpid())
  {
    return(OURS);	/* File exists and its ours */
  }

  if ((kill(pid, 0) != 0))
  {
    if(errno == ESRCH)
      return(DEAD);	/* File exists, not ours, and PID no longer exists */
    else
      return(DONTKNOW);	/* File exists, not ours, and can not be signaled */
  }

  return(LIVE);		/* File exists, not ours, and can be signaled */
}

/*
** readlock
**
** read contents of lockfile
** Returns pid read or FAIL on error.
*/

int readlock(name)
char *name;
{
  int fd, pid, n=0;
  char apid[16];

  if ((fd = open(name, O_RDONLY)) == FAIL)
    return(FAIL);

  (void) read(fd, apid, sizeof(apid));
  n = sscanf(apid, "%d", &pid);

  if (n != 1){
    (void) close(fd);
    fd = open(name, O_RDONLY);
    (void) read(fd, (char *)&pid, sizeof(pid));
  }

  (void) close(fd);
  return(pid);
}

void
removelock(name)
char *name;
{
  (void) unlink(name);
}

