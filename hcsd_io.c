/*
** Author:	Neil Cherry
** Date:	11/25/99
** Version:	1.6 (alpha)
**
**      $Id: hcsd_io.c,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
**      $Log: hcsd_io.c,v $
**      Revision 1.1.1.1  2008/09/24 19:04:13  ncherry
**      Imported using TkCVS
**
**      Revision 1.1.1.2  2000/03/18 14:22:03  ncherry
**
**      Added hcs_dl.c which is a standalone program to download an events.bin file
**      to an HCS II connected to a local serial port.
**
**      Revision 1.1.1.1  2000/03/13 13:05:58  ncherry
**      This is the inital base release. It is nothing more that some of my (njc)
**      existing code from my HCS II project. It is the code without the pthread
**      functions. It has problems with character drops on the receive serial port.
**
**
**      Revision 1.6  1999/11/25 17:32:14  njc
**      This  file  is  supposed  to  contain all  the  read  write  routines.
**      Currently many  direct reads occur  elsewhere. I will correct  this in
**      future releases  of the code.  I've added a  lot of select  to various
**      sections of this code and I'm  not sure whether this has caused me the
**      serial buffer overrun problems I've experienced so far.
**
*/

#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>

#include "hcs_defs.h"
#include "clients.h"

extern int stoi(char c);
extern int waitfor_rd(int fd, int nsec, int usec);
extern int waitfor_wr(int fd, int nsec, int usec);

extern void clwrstr(const char *buf, int count);

/*
** This will give me the option to snoop
**
** xread will loop until it get read count
** characters. which it simply puts into buf
*/

/*
** Dan does the following:
**
** FD_SET(n, &xprobe);
**
** tv.tv_sec  = 0;
** tv.tv_usec = count*1500;     // We get a character ~once/1500 usec
**
** c = select(n + 1, &xprobe, FD_NULL, FD_NULL, &tv);
**
** if(c < 0) {
**   syslog(LOG_ERR, "select addrreceive: %m");
**   return (-1);
** }
**
** if(c == 0) {
**   syslog(LOG_ERR, "select addrreceive timeout");
**   return (-1);
** }
** c = read(n, buf + cnt, 2 - cnt);
**
** This would be a good way for me to cut back on endless looping or read hangs
** (good for slower machines).
**
** This is a future item for now.
*/

ssize_t
xread(int fd, char *buf, size_t count)
{
  int i, j, k, x, usec;

  char *ptr;

  i = 0;
  k = 0;

  usec = count * 2000000;
  
  /*
  ** Malloc the memory
  */

  if((ptr = malloc(count+4)) == NULL) {
    fprintf(stderr, "XPRESS malloc error (%d)\n", errno);
    syslog(LOG_CRIT, "XPRESS malloc error: %m");
    return((-1));
  }

  /*
  ** count down
  */

  while(i < count) {

    /*
    ** Here is where the select code belongs. We can figure out the time
    ** it will take to get x bytes at 9600 (assumed because we're dealing
    ** with the serial port as the slowest device).
    **
    ** This should allow us to correctly handle timeouts and other read mishaps
    */

    j = 0;

    /*
    ** retry 3 time, then out
    */

    while(((k = waitfor_rd(fd, 0, usec)) <= 0) && (j < 3)) {
      j++;
    }
    

    if(k < 1) {
      *buf = '\0';
      return (-1);
    }
    
    if((j = read(fd, ptr, count)) > 0) {
      for(x = 0; x < j; x++) {
        *buf = *(ptr+x);
        buf++;
        i++;
      }
    } else {
      if(!(j == 0 || errno == EAGAIN)) {
        syslog(LOG_ERR, "(%d) Read error (%d)", j, errno);
        free(ptr);
        return((-1));
      }
    }
  }
  free(ptr);

  return(i);
}

extern int nclient;
extern struct clients *clients;

extern char *hexstr;

ssize_t
clwrite(const char *buf, size_t count)
{
  char tmp[MAXLINE];
  
  int i, j;

  j = (count * 2) + 1;

  tmp[0] = '$';

  i = 1;
  while(i <= j) {
    tmp[i++] = hexstr[(MASK(*buf))>>4]; /* hi 4 bits */
    tmp[i++] = hexstr[(MASK(*buf))%16]; /* lo 4 bits */
    buf++;
  }
#ifdef NJC
  for(i = 0; i < nclient; i++) {
    if(clients[i].flags & C_QUIET)
      continue;
    
    if(write(clients[i].s, tmp, j) != j)
      shutdown(clients[i].s, 2);
    else
      write(clients[i].s, EOL, 2);
  }
#else
  clwrstr(tmp, j);
#endif
  
  return(count);
}

void
clwrstr(const char *buf, int count)
{
  int i;
  
  for(i = 0; i < nclient; i++) {
    if(clients[i].flags & C_QUIET)
      continue;
    
    if(write(clients[i].s, buf, count) != count)
      shutdown(clients[i].s, 2);
    else
      write(clients[i].s, EOL, 2);
  }
}

/*
** I wrote this because I am getting too lazy to keep typing in the same
** lines. Besides this reads better when viewing the code. Readability is
** important too.
*/

int
fdprintf(int fd, const char *format, ...)
{
  va_list ap;

  char p[4*MAXLINE];
  int  j, x;
  
  va_start(ap, format);
  (void) vsprintf(p, format, ap);
  j = strlen(p);
  x = write(fd, p, j);
  va_end(ap);
  return(x);
}


int
dwrite(int device, char *str)
{
  char cmd[MAXLINE];             /* Later Malloc this! */

  int a, i = 0;

  while(*str) {
    a = stoi(*str++) * 16;
    a = stoi(*str++) + a;
    cmd[i] = a;
    i++;
  }

  write(device, (char *) "!", 1);
  write(device, cmd, i);

  return(i);
}


/*
** Just return whatever select returns
*/

int
waitfor_rd(int fd, int nsec, int usec)
{
  int c;
  
  struct timeval tv;

  fd_set lprobe;

  FD_SET(fd, &lprobe);
  tv.tv_sec  = nsec;             /* wait for n seconds */
  tv.tv_usec = usec;             /* wait for n useconds */

  c = select(fd + 1, &lprobe, FD_NULL, FD_NULL, &tv);
  if(c < 0) {
    syslog(LOG_ERR, "select read error: %m");
  }

  return(c);
}

int
waitfor_wr(int fd, int nsec, int usec)
{
  int c;
  
  struct timeval tv;
  fd_set lprobe;

  FD_SET(fd, &lprobe);
  tv.tv_sec  = nsec;             /* wait for n seconds */
  tv.tv_usec = usec;             /* wait for n useconds */

  c = select(fd + 1, FD_NULL, &lprobe, FD_NULL, &tv);
  if(c < 0) {
    syslog(LOG_ERR, "select write error: %m");
  }

  return(c);
}
