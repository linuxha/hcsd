/*
** Author:	Neil Cherry
** Date:	11/25/99
** Version:	1.6 (alpha)
**
**      $Id: hcsclock.c,v 1.1.1.1 2008/09/24 19:04:12 ncherry Exp $
**
**      $Log: hcsclock.c,v $
**      Revision 1.1.1.1  2008/09/24 19:04:12  ncherry
**      Imported using TkCVS
**
**      Revision 1.1.1.2  2000/03/18 14:22:01  ncherry
**
**      Added hcs_dl.c which is a standalone program to download an events.bin file
**      to an HCS II connected to a local serial port.
**
**      Revision 1.1.1.1  2000/03/13 13:05:56  ncherry
**      This is the inital base release. It is nothing more that some of my (njc)
**      existing code from my HCS II project. It is the code without the pthread
**      functions. It has problems with character drops on the receive serial port.
**
**
**      Revision 1.6  1999/11/25 17:32:14  njc
**      This file contains the functions needed to set the clock from the Unix
**      servers time.  Later I may add  a further routine to allow the user to
**      set the HCS  II to whatever time they wish to  use. The clock routines
**      should verify that it gets a  response to it's commands. And I need to
**      add the command line options to change the time intervals.
**
*/

#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>

#include "hcs_defs.h"

/*
** Routine need to set the HCS clock to match that of the System
*/

extern int interval;
/**********************************************************************/

/*
** This routine convert BCD (Binary Coded Decimal) and converts it to
** an interger value.
*/

uint
bcd2b(uint v) {
  return(((v>>4)*10)+(v&0xf));
}

/*
** This routine converts an integer value to BCD.
*/

uint
b2bcd(uint v) {
  return(((v/10)<<4)|(v%10));
}

void
sethcsclock(int device) {
  int i;
  
  time_t t;
  struct tm *tms;
  
  char tmp[MAXLINE];
  char buf[10];

  /*
  ** First we need to tell the HCS how often to update to update the time
  */
  
  buf[0] = CMD;                 /*  */
  buf[1] = SELECTTIME;
  buf[2] = interval;

  if((i = write(device, buf, 3)) != 3) { /* OK for now I'll do nothing on error */
    syslog(LOG_ERR, "Write error (%d) != 9 (Err: %c)", i, errno);
    return;
  }
  
  /*
  ** Here is where we should wait to get the time back if interval is not OFF
  ** This gives us a way of telling weather we are talking to the HCS.
  */
  
  if(interval) {
    ;
  }
  
  buf[0] = CMD;                 /* ! */
  
  if((i = write(device, buf, 1)) != 1) { /* OK for now I'll do nothing on error */
    syslog(LOG_ERR, "Write error (%d) != 9 (Err: %c)", i, errno);
    return;
  }
  
  time(&t);
  tms    = localtime(&t);
  buf[0] = SETTIME;
  buf[1] = 0;                     /* Hundredths of seconds */
  buf[2] = b2bcd(tms->tm_sec);    /* seconds               */
  buf[3] = b2bcd(tms->tm_min);    /* minutes               */
  buf[4] = b2bcd(tms->tm_hour);   /* hours                 */
  buf[5] = b2bcd(tms->tm_wday+1); /* day of the week       */
  buf[6] = b2bcd(tms->tm_mday);   /* day of the month      */
  buf[7] = b2bcd(tms->tm_mon+1);  /* month                 */
  buf[8] = b2bcd(tms->tm_year);   /* year (2 digits)       */
  buf[9] = 0;

  sprintf(tmp,"Time set to: %04u/%02u/%02u %02u:%02u:%02u\n",
          tms->tm_year+1900, tms->tm_mon+1, tms->tm_mday,
          tms->tm_hour,      tms->tm_min,   tms->tm_sec);

  syslog(LOG_INFO, tmp);

  if((i = write(device, buf, 9)) != 9) { /* OK for now I'll do nothing on error */
    syslog(LOG_ERR, "Write error (%d) != 9 (Err: %c)", i, errno);
  }
}

