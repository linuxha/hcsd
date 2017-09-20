/*
**
** Author:      Neil Cherry
** Date:        10/16/99
** Version:     1.1 (sub alpha)
**
** $Id: funcs.c,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
** $Log: funcs.c,v $
** Revision 1.1.1.1  2008/09/24 19:04:13  ncherry
** Imported using TkCVS
**
** Revision 1.1.1.2  2000/03/18 14:22:01  ncherry
**
** Added hcs_dl.c which is a standalone program to download an events.bin file
** to an HCS II connected to a local serial port.
**
** Revision 1.1.1.1  2000/03/13 13:05:56  ncherry
** This is the inital base release. It is nothing more that some of my (njc)
** existing code from my HCS II project. It is the code without the pthread
** functions. It has problems with character drops on the receive serial port.
**
**
** Revision 1.6  1999/11/25 17:32:13  njc
** This file  is supposed  to contain various  functions used  with hcsd.
** I'm not very  pleased with the way the functions  have been working so
** this file may eventually go away.  Currently it contains the binary to
** hex and hex to binary routines.  These will stay but in modified form.
** This file  also contains  the get  log routine and  the shell  for the
** getxpress routine.
**
** Revision 1.1  1999/10/16 17:50:52  njc
** Initial revision
**
**
*/

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "hcs_defs.h"

/****************************************************************************/
extern int fdprintf(int fd, const char *format, ...);
extern int dwrite(int device, char *str);

extern ssize_t xread(int fd, char *buf, size_t count);
extern ssize_t clwrite(const char *buf, size_t count);

void sethcsclock(int device);

int htoi(char *cmd, char *str);
int stoi(char c);
int logsize(int device);

void getlog(int device, int user) ;

/****************************************************************************/
extern int errno;
/****************************************************************************/

/**********************************************************************/

/*
** htoi and stoi take a string of ascii hex characters and converts them
** to byte values. So "11" (2 bytes) becomes 0x11 (1 byte), "1122" becomes
** 0x11 0x22 and so on. Anything that is not hex (ie "jj" become 0x0).
*/

int
htoi(char *cmd, char *str)
{
  int a, i = 0;

  while(*str) {
    a = stoi(*str++) * 16;
    a = stoi(*str++) + a;
    cmd[i] = MASK(a);
    i++;
  }
  return(i);
}

int
stoi(char c)
{
  int a;
  
  a = toupper(c);
  if (a < 'G' && a > '@') {
    a = a - 'A' + 10;
  } else if ( a < ':' && a > '/') {
    a = a - '0';
  } else {
    a = 0;
  }
  
  return(a);
}

void
help(int user)
{
  fdprintf(user, "\r\nHelp message\r\n\n");
  fdprintf(user, "!<CMD><DATA><DATA>...\r\n");
  fdprintf(user, "L xxxx - Load an XPRESS file where xxxx is the size in hex\r\n");
  fdprintf(user, "T - Set HCS II time to Current Unix time\r\n");
  fdprintf(user, "S - hcsd Status\r\n");
  fdprintf(user, "G - Get current stored HCS II XPRESS file\r\n");
  fdprintf(user, "P - Put a new XPRESS program into the HCS II\r\n");
  fdprintf(user, "R - Reload existing XPRESS program into the HCS II\r\n");
  fdprintf(user, "Q - Quiet, send no response back to the user\r\n");
  fdprintf(user, "X - Exit, disconnect\r\n");
  fdprintf(user, "\r\n");
}

/*
** Get the locally stored events.bin and send it to the user.
*/

void
getxpress(int user, int device)
{
  ;
}

/*
** Get the log and send it to the user.
**
** Each entry consists of 8 bytes as follows:
**
** 0 - Reference ID (0-254)
** 1 - Data Lo byte (0-255)
** 2 - Data Hi byte (0-255)
** 3 - Month        (1-12)
** 4 - Day          (1-31)
** 5 - Hour         (0-23)
** 6 - Minute       (0-59)
** 7 - Second       (0-59)
*/

extern char *hexstr;

void
getlog(int device, int user) 
{
  int i, j;
  char tmp;
  char buf[32*1024+5];
  
  j = logsize(device);          /* get the size of the log */
  dwrite(device, "09");         /* Now get the log itself  */

  i = xread(device, buf, j+5);

  write(user, (char *) "$", 1);
  
  for(j = 1; j < i; j++) {
    tmp = hexstr[(MASK(buf[j]))>>4]; /* hi 4 bits */
    write(user, &tmp, 1);
    tmp = hexstr[(MASK(buf[j]))%16]; /* lo 4 bits */
    write(user, &tmp, 1);
  }
  
  write(user, EOL, 2);
}

int
logsize(int device)
{
  int i = 0;
  char buf[MAXLINE];

  buf[0] = 0x08;
  
  write(device, "!", 1);
  write(device, buf, 1);

  xread(device, buf, 4);
  i = ((MASK(buf[3]))<<8) + MASK(buf[2]);
  syslog(LOG_DEBUG, "Log Size %d", i);
  
  return(i*8);
}

#ifdef NJC
/*
** I'm just using this as temporary storage.
*/

char *xpress_buf, *xline, *xbuf;

  /*
  ** Should probably malloc the xline and bline arrays in main
  */
  
  if((xpress_buf = malloc(64*1024)) == NULL) {
    fprintf(stderr, "XPRESS malloc error (%d)\n", errno);
    syslog(LOG_CRIT, "XPRESS malloc error: %m");
    exit(errno);
  }
  
  /*
  ** Should probably malloc the xline and bline arrays in main
  */
  
  if((xline = malloc(2*MAXLINE)) == NULL) {
    fprintf(stderr, "XPRESS malloc error (%d)\n", errno);
    syslog(LOG_CRIT, "XPRESS malloc error: %m");
    exit(errno);
  }

  /*
  ** Should probably malloc the xline and bline arrays in main
  */
  
  if((xbuf = malloc(2*MAXLINE)) == NULL) {
    fprintf(stderr, "XPRESS malloc error (%d)\n", errno);
    syslog(LOG_CRIT, "XPRESS malloc error: %m");
    exit(errno);
  }
#endif
