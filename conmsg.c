/*
** Author:	Neil Cherry
** Date:	11/25/99
** Version:	1.6 (alpha)
**
**      $Id: conmsg.c,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
**      $Log: conmsg.c,v $
**      Revision 1.1.1.1  2008/09/24 19:04:13  ncherry
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
**      This function handles Console messages  from the HCS II. Currently it
**      receives them  but does nothing else  with them. I'm not  sure how I'm
**      going to handle this when I add the new r/w code.
**
*/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>

extern int waitfor_rd(int fd, int nsec, int usec);

void
conmsg(int device) 
{
  int count;
  //char *ptr;
  char *buf;

  if((buf = malloc(256)) == NULL) {
    syslog(LOG_CRIT, "conmsg malloc error: %m");
    return ;
  }

  count = 0;

  //ptr = buf;
  
  while(count < 256) {
    /*
    ** I'll wait 30 seconds (x3) for a response.
    */

    if((count = waitfor_rd(device, 2, 0)) > 0 ) {
      if((count = read(device, buf, 1)) == 1) {
        if(*buf == '\0') {
          *buf = (char ) '\0';
/*
** This needs to be fixed!
*/
//          fdprintf(user, "$80%s\r\n", ptr);
          return ;
        }
        buf++;
        count++;
      } else {
        // handle error
      }
    } else {
      // Handle error or timeout
    }
  }
  *buf = (char ) '\0';
//  fdprintf(user, "$80%s\r\n", ptr);
}
