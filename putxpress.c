/*
** Author:	Neil Cherry
** Date:	11/21/99
** Version:	1.1 (sub alpha)
**
**      $Id: putxpress.c,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
**      $Log: putxpress.c,v $
**      Revision 1.1.1.1  2008/09/24 19:04:13  ncherry
**      Imported using TkCVS
**
**      Revision 1.1.1.2  2000/03/18 14:22:05  ncherry
**
**      Added hcs_dl.c which is a standalone program to download an events.bin file
**      to an HCS II connected to a local serial port.
**
**      Revision 1.1.1.1  2000/03/13 13:05:59  ncherry
**      This is the inital base release. It is nothing more that some of my (njc)
**      existing code from my HCS II project. It is the code without the pthread
**      functions. It has problems with character drops on the receive serial port.
**
**
**      Revision 1.6  1999/11/25 17:32:14  njc
**      This file contains the code to read from the events.bin file and write
**      the contents to the HCS II. I still need to create the CRC check code.
**      Later I may also tear out a function to write the code to the HCS so I
**      can this drop it into the load function.
**
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>

#include "hcs_defs.h"

extern int fdprintf(int fd, const char *format, ...);
extern int xread(int fd, char *buf, int size);
extern int waitfor_rd(int fd, int nsec, int usec);
extern int waitfor_wr(int fd, int nsec, int usec);
extern int dwrite(int device, char *str);

/*
** Take the locally stored events.bin file and send it to the HCS II
*/

void
putxpress(int user, int device) 
{
  int evfile, j, k, bytes, count;

  char *xpress_buf, *ptr, chbuf[MAXLINE];
  
  struct stat stbuf;
  
  if((evfile = open(hcsfile, O_RDONLY)) == (-1)) {
    syslog(LOG_ERR, "open error %s: %m", EVENTS_BIN);
    fdprintf(user, "?\r\n");
    return ;
  }

  if((j = fstat(evfile, &stbuf)) == (-1)) {
    syslog(LOG_ERR, "stat error %s: %m", EVENTS_BIN);
    fdprintf(user, "?\r\n");
    close(evfile);
    return ;
  }

  if((xpress_buf = malloc(stbuf.st_size)) == NULL) {
    syslog(LOG_CRIT, "XPRESS malloc error: %m");
    fdprintf(user, "?\r\n");
    close(evfile);
    return ;
  }

  if((j = read(evfile, xpress_buf, stbuf.st_size)) < 0) {
    syslog(LOG_CRIT, "read error (%s): %m", hcsfile);
    fdprintf(user, "?\r\n");
    close(evfile);
    free(xpress_buf);
    return ;
  }

  if(j == stbuf.st_size) {
    fdprintf(user, "Read all (%d) bytes\r\n", j);
    fdprintf(user, "CRC = %02x\r\n", MASK(*(xpress_buf+j-1)));
    
    /*
    ** CRC check
    **
    ** 1) Add up ASCII values
    ** 3) Take two's compliment
    **
    ** 2's compliment (Invert plus 1)
    */

    count = 0;
    bytes = 0;
    
    ptr = xpress_buf;

    while(count < j) {
      bytes = bytes + MASK(*ptr);
      ptr++;
      count++;
    }
    bytes = (~bytes) + 1;
    
    fdprintf(user, "CRC = %02x\r\n", MASK(bytes));
    
  } else {
    fdprintf(user, "Bad read count %d != %d\r\n", j, stbuf.st_size);
    free(xpress_buf);
    close(evfile);
    return ;
  }
  
  
  /*
  ** Write it to the device
  */

  
  dwrite(device, "06");

  /*
  ** I'll wait 30 seconds (x3) for a response.
  */

  if((count = waitfor_rd(device, 30, 0)) > 0 ) {
    if((count = read(device, chbuf, 1)) == 1) {
      fdprintf(user, "[%c (%02x)]\r\n", chbuf[0], chbuf[0]);
    } else {
      // handle error
      fdprintf(user, "read error ?\r\n");
      //free(xpress_buf);
      //close(evfile);
    }
  } else {
    // Handle error or timeout
    fdprintf(user, "waitfor_rd error ?\r\n");
    //free(xpress_buf);
    //close(evfile);
  }
  
  count = j;
  ptr = xpress_buf;

  while(count) {
    /*
    ** Select code (wait until we're able to write the stuff to the prot.
    */

    k = waitfor_wr(device, 0, 10000);

    if(k < 0) {
      syslog(LOG_ERR, "waitfor_wr error: %m");
      fdprintf(user, "waitfor_wr (%d) < 0\r\n", errno);
      return ;
    }
    
    if(k == 0) {
      fdprintf(user, "waitfor_wr timeout\r\n");
    }

    if(count < 8) {
      bytes = count;
    } else {
      bytes = 8;
    }

    if((k = write(device, ptr, bytes)) < 0) {
      /*
      ** Catch the errors
      **
      ** Some errors may be ignores (well, maybe)
      */
      
      fdprintf(user, "write error\r\n");
      
    } else {
      fdprintf(user, "wrote %d\r\n", k);
      
      count = count - k;
      ptr = ptr + k;
    }
  }
  
  /*
  ** I'll wait 30 seconds (x3) for a response.
  */

  if((count = waitfor_rd(device, 30, 0)) > 0 ) {
    if((count = read(device, chbuf, 1)) == 1) {
      fdprintf(user, "[%c (%02x)]\r\n", chbuf[0], chbuf[0]);
    } else {
      // handle error
      fdprintf(user, "?\r\n");
    }
  } else {
    // Handle error or timeout
    fdprintf(user, "?\r\n");
  }

  free(xpress_buf);
  close(evfile);
}
