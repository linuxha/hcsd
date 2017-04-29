/*
** Author:	Neil Cherry
** Date:	11/25/99
** Version:	1.6 (alpha)
**
**      $Id: xload.c,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
**      $Log: xload.c,v $
**      Revision 1.1.1.1  2008/09/24 19:04:13  ncherry
**      Imported using TkCVS
**
**      Revision 1.1.1.2  2000/03/18 14:22:06  ncherry
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
**      This file  contains the code to load  an XPRESS file into  the HCS II.
**      Currently it just stores the  file locally.  I need to incorporate the
**      putxpress code into the xload function.
**
*/

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>

#include "hcs_defs.h"

extern int fdprintf(int fd, const char *format, ...);
extern int waitfor_rd(int fd, int nsec, int usec);
extern int waitfor_wr(int fd, int nsec, int usec);
extern int stoi(char c);
extern int dwrite(int device, char *str);

/*
** load XPRESS program into the HCS II AND store it locally in events.bin
**
** -> to HCS II
** <- from HCS II
**
** 1) -> !06			We send
** 2) <- ^			We wait for a '^'
** 3) -> XXXXXX.....\r\n	We send binhex, when the file finishes
**				we send <CR><LF>
** 4) <- ^ or # or @		We wait for '^' (Success) or
**				            '#' (Version mismatch)
**					    '@' (Program too big)
**
** This routine has a couple of unique problems. First when we get a file open
** error all future attempts at access will fail but so will the read or the
** users input (???). Also this uses arrays to buffer the information. We
** _really_ need to switch to mallocs. Right now I'll lacking some of the info
** I to determine how large the malloc needs to be. Another problem is what
** to do (how to handle) if the user screws up half way through the d/l? First
** we need to recover (finish the disconnect), second we need to clean up the
** d/l, third we need to d/l the previous file (???). Hmmm.
**
** I've also added dummy start and stop stuff to simulate communication with
** the HCS II.
**
** The events.bin file is now being written correcly.
*/

void
load(int device, int user)
{
  int a, i = 0, evfile=0;

  char tmp[5], buf[32*1024];
  char *ptr;

  ptr = (char *) tmp;

  fdprintf(user, LOAD_OK);       /* I don't know if I want to keep the \r\n on here */

  /*
  ** wait for input from the user. If the user has manually connected there
  ** is a long delay between character. So I use select to wait for the first
  ** one.
  **
  ** I wonder if we should do this for each character of user input?
  **
  ** Ooops it works a little too well, user input is in line mode so we
  ** must wait nsec*(nchar+2)+20%
  */

  /*
  if(!(waitfor(user, 1, 0) > 0)) {
    fdprintf(user, "Select error\r\n");
    return ;
  }
  */
  do {
    buf[i] = 0;

    if((read(user, ptr, 1)) > 0) {
      if(isxdigit(*ptr)) {
        buf[i] = *ptr;
        i++;
      }
    }
  } while (*ptr != '\n');

  buf[i] = 0;

  if((evfile = open(EVENTS_BIN, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == (-1)) {
    syslog(LOG_ERR, "%s: %m", EVENTS_BIN);
  } else {
    ptr = &buf[0];

    while(*ptr) {
      a = stoi(*ptr++) * 16;
      a = stoi(*ptr++) + a;
      write(evfile, &a, 1);
    }
    close(evfile);
  }

  /*
  ** Here we wait for a response to the file sent, the response should ^, #, or @
  */

  fdprintf(user, LOAD_OK);       /* I don't know if I want to keep the \r\n on here */
}

extern int xread(int fd, char *buf, int size);
extern int htoi(char *cmd, char *str);

/*
** This code needs a rewrite, 2 reasons, first is it is long and lanky
** second is that I need to combine the u/l from the client with saving the
** file and writing to the HCS.
**
** A second kind of load
**
** => To hcsd
** <= from hcsd
** -> to HCS II
** <- from HCS II
**
** => L xxxx			User sends
** -> !06			We send to HCS
** <- ^				We wait for a '^'
** <= ^				We then forward it to the user
** => XXXXXXXXXXXXX....XX	We then send 128 ASCII characters at a time (+ "\r\n")
** -> xxxxx...xx		We send 64 binary characters to the HCS
** <= ^				We then forward it to the user
**          : :
** => XXXXXXXXXXXXX....XX	Final line
** -> xxxxx...xx		Final line
** <- ^ or # or @		We wait for '^' (Success) or
**				            '#' (Version mismatch)
**					    '@' (Program too big)
** <= ^ or # or @		We forward it to the user
**
*/

void
xload(int device, int user, int size)
{
  int r, x, y, loop;
  int loop_err = 0, evfile = 0; 
  
  char buf[MAXLINE], conv[MAXLINE];
  
  /*
  ** Careful with the logic here:
  **
  **  128 ASCII Hex characters == 64 Binary characters
  **
  ** Size is the number of binary characters to be read
  ** but we are actually reading ASCII hex characters
  */

  loop = size/64;               /* Number of times to loop  */

  if((evfile = open(EVENTS_BIN, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == (-1)) {
    fdprintf(user, "Can not open '%s' (%d)\r\n", EVENTS_BIN, errno);
    syslog(LOG_ERR, "%s: %m", EVENTS_BIN);
    fdprintf(user, "?\r\n");
    return ;
  }

  fdprintf(user, LOAD_OK);

  /*[ Get a line at a time ]***************************************************/

  while(loop) {
    /*
    ** Send ok and wait for the user.
    */

    if(xread(user, buf, 130) != 130) {      /* 128 characters plus \r\n */
      fdprintf(user, "?\r\n");
      return ;
    }
    
    *(buf+128) = '\0';          /* null terminate the \r    */

    if(htoi(conv, buf) != 64) {
      fdprintf(user, "@\r\n");
      loop_err++;

      if(loop_err > 10) {
        if(evfile) {
          close(evfile);
        }
        return ;
      }
    } else {
      if(evfile) {
        write(evfile, conv, 64);
      }
      
      fdprintf(user, LOAD_OK);
      loop--;
    }
  }

  /*
  ** Calculate the left over characters
  */
  
  r = size%64;                  /* Any left over characters */

  if(r) {                       /* Catch the remainder */
    x = r * 2;
    y = x + 2;

    if(xread(user, buf, y) != y) {      /* 128 characters plus \r\n */
      fdprintf(user, "?\r\n");
      if(evfile) {
        close(evfile);
      }
      return ;
    }

    *(buf+x) = '\0';              /* null terminate the \r */
  
    if(htoi(conv, buf) != r) {
      fdprintf(user, "@\r\n");
      if(evfile) {
        close(evfile);
      }
      return ;
    } else {
      if(evfile) {
        write(evfile, conv, x/2);
      }

      fdprintf(user, LOAD_OK);
    }
  }
  
  /*[ Wait for a reply ]*******************************************************/

  if(evfile) {
    close(evfile);
  }
  
   fdprintf(user, LOAD_OK);

  /*
  ** done
  */
}
