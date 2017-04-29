/*
** Author:	Neil Cherry
** Date:	11/29/99
** Version:	1.1 (sub alpha)
**
** The idea behind this section of code is to just handle the incoming traffic,
** process it (bin->ASCII hex) and buffer it. It just keeps going round and
** round overwritting the old data. It doesn't care about overruns, that's the
** job of the client print routine.
**
**      $Id: hcsd_sio.c,v 1.1.1.1 2008/09/24 19:04:12 ncherry Exp $
**
**      $Log: hcsd_sio.c,v $
**      Revision 1.1.1.1  2008/09/24 19:04:12  ncherry
**      Imported using TkCVS
**
**      Revision 1.1.1.2  2000/03/18 14:22:04  ncherry
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
**      Revision 1.1  1999/11/30 00:02:13  njc
**      Initial revision
**
**
*/

#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <stdlib.h>

#include "hcs_defs.h"

extern size_t xread(int fd, char *buf, size_t count);

/*
** I should never need more than 4K at any one time. But a log dump
** can be as large as 32K. So I'll go for 36K.
*/

#define BUFSIZE (36*1024)

fd_set lprobe;

char *rdbuf, *bottom, *current, *ptr, *top;

void intr_service(int device)
{
  int  x;
  char *rdbuf, *ptr, *top;

  if((rdbuf = malloc(BUFSIZE)) == NULL) {
    syslog(LOG_CRIT, "rdbuf malloc error: %m");
    /*
    ** I guess I should signal that the child just died
    */
    
    exit(errno);
  }

  ptr = rdbuf;
  top = rdbuf+BUFSIZE;

  /*
  ** Ok here are a few questions I need answered:
  **
  ** How will we know when this routine clobbers the read routine
  ** (ie how do we know where the pointer is at any one time).
  ** I want this to be the job of the client read routine not the
  ** device read routine.
  */

  while(1) {
    /*
    ** Wait for a character
    */

    x = select(device + 1, &lprobe, FD_NULL, FD_NULL, TIMEVAL_NULL);

    if(x < 0) {
      syslog(LOG_ERR, "select read error: %m");
    } else {
      read(device, ptr, 1);
      ptr++;
      
      if(ptr > top) {
        ptr = rdbuf;
      }
    }
  }
}

/*
** A user (or program) sends a !<CMD>[<DATA>....] and gets back a $<CMD> with
** the exception of $00 (reset), $02 (full status), $06 (load XPRESS) and $09
** (get log).
**
** Here is the list of what the user can do:
**
** --------------------------------------------------------------------------
** Hex
** Cmd Description                                        Data Reply
** === ================================================== ==== ===== 
** 00  Reset system                                       -    *
** 01  Pause all status until next command                -    -
** 02  Request full system status information             -    *
** 03  Select time display frequency                      A    -
** 04  Get time once                                      -    A-H
** 05  Set time                                           A-H  -
** 06  Load new XPRESS program                            XXX  *
** 07  Clear log memory                                   -    -
** 08  Get size of logged data                            -    A-B
** 09  Get logged data                                    -    *
** 0A  Put data in log memory                             A-C  - 
**  ~
** 10  Select X10 display                                 A-B  -
** 11  Get X10 status once                                A    A-B
** 12  Get X10 module                                     A-C  -
** 13  Select digitial input display                      A-D  -
** 14  Get digital input status once                      A    A-B
** 15  Set digital input state                            A-B  -
** 16  Select digital output display                      A-D  -
** 17  Get digital output status once                     A    A-B
** 18  Set digital output state                           A-B  -
** 19  Select ADC display                                 A-C  -
** 1A  Get ADC value once                                 A    A-C
** 1B  Set ADC value                                      A-C  -
** 1C  Select DAC display                                 A    -
** 1D  Get DAC value once                                 A    A-B
** 1E  Set DAC value                                      A-B  -
** 1F  Select net module display                          A    -
** 20  Get net module status once                         A    A-B
** 21  Select netbit display                              A-E  -
** 22  Get net module status once                         A-B  A-C
** 23  Set netbit state                                   A-C  -
** 24  Get variable value once                            A    A-C
** 25  Set variable value                                 A-C  -
**  ~
** 30  Send string to network                             str  -
** 31  Send string to voice                               str  -
** 
*/

/*
** Process will take what's been received and convert it from binary
** to ASCII hex. It then stores it in a buffer to be sent out to the
** connected clients.
*/

void
process(int device)
{
  int c;

  char rbuf[MAXLINE];
  
  if((c = read(device, rbuf, 1))) {

    /*
    ** OK here is where we actually do some work and monitor the HCS II
    **
    ** First wait for a '$' (start of a Response). Now the fun part. If a user
    ** issues a command the reponse may be a '$' and the command and nothing else
    ** an example is !^C^B (Select Time once per minute) the response is $^C.
    ** Yet issue a Reset !^@ and you get no response. So we have to figure out
    ** to get the HCS and hcsd in sync and keep it in sync. Hmm how do I do
    ** this ???
    */

    while(c) {
      if(MASK(rbuf[0]) == REPLY) {

        c = read(device, rbuf, 1);

        /*
        ** This is probably a very good candidate for a rewrite. The reason is
        ** we're not getting the full reponse to commands issued. Only the
        ** $<CMD> (default: case).
        */
        
        switch(MASK(rbuf[0])) {
          case TIME:                  /* Current time (8 bytes) */
            c = xread(device, rbuf+1, 8);
            break;

          case X10:                   /* X10 Modules (3 bytes) */
            c = xread(device, rbuf+1, 3);
            break;

          case DigIn:                 /* Digital Inputs (2 bytes) */
            c = xread(device, rbuf+1, 2);
            break;

          case DigOut:                /* Digital Outputs (2 bytes) */
            c = xread(device, rbuf+1, 2);
            break;

          case AnalIn:                /* Analog Inputs (2-17 bytes) */
            c = xread(device, rbuf+1, 17);
            break;

          case AnalOut:               /* Analog Outputs (2-5 bytes) */
            c = xread(device, rbuf+1, 5);
            break;

          case NetMods:               /* Network Modules (2 bytes) */
            c = xread(device, rbuf+1, 2);
            break;

          case NetBits:               /* Netbits (2 bytes) */
            c = xread(device, rbuf+1, 2);
            break;

          case CONMSG:                /* Console message (NULL terminated string) */
            // Requires special handling
            break;

          /*
          ** These are responses to commands sent not status messages
          */
            
          case RESET:           /* No response */
            break;
          
          case PAUSE:           /*  */
            break;
          
          case FULLSTATUS:      /* no response */
            break;
          
          case SELECTTIME:
            break;
          
          case GETTIME:         /* get time once */
            c = xread(device, rbuf+1, 8);
            break;

          case LOAD:            /* SPECIAL HANDLING */
            break;
          
          case CLRLOG:          /*  */
            break;
          
          case SIZELOG:         /* get size of log */
            c = xread(device, rbuf+1, 2);
            break;

          case GETLOG:         /* SPECIAL HANDLING */
            break;
            
          case PUTLOG:
            break;
          
          case SELECTX10:
            break;
          
          case GETX10:            /* get x10 status once */
            c = xread(device, rbuf+1, 2);
            break;

          case SETX10:
            break;
          
          case SELECTDI:
            break;
          
          case GETDI:            /* get DigIn status once */
            c = xread(device, rbuf+1, 2);
            break;

          case SETDI:
            break;
          
          case SELECTDO:
            break;
          
          case GETDO:            /* get DigOut status once */
            c = xread(device, rbuf+1, 2);
            break;

          case SETDO:
            break;
          
          case SELECTADC:
            break;
          
          case GETADC:            /* get ADC value once */
            c = xread(device, rbuf+1, 3);
            break;

          case SETADC:
            break;
          
          case SELECTDAC:
            break;
          
          case GETDAC:            /* get DAC value once */
            c = xread(device, rbuf+1, 2);
            break;

          case SETDAC:
            break;
          
          case SELECTNETMOD:
            break;
            
          case GETNETMOD:            /* get net mod status once */
            c = xread(device, rbuf+1, 2);
            break;

          case SELECTNETBIT:
            break;
          
          case GETNETBIT:            /* get net bit status once */
            c = xread(device, rbuf+1, 3);
            break;

          case SETNETBIT:
            break;
          
          case GETVAR:            /* get variable value once */
            c = xread(device, rbuf+1, 3);
            break;
            
          case SETVAR:
            break;

          case NETSTR:
            break;
          
          case VOICESTR:
            break;
          
          default:
            break;

        } /* End of switch */
      } else {
        /*
        ** Hmmm, what do I do about this ????
        **
        ** Why, you might ask, because I triggered something in the HCS that
        ** sent a continous string of characters! Hmmm....
        */

        syslog(LOG_INFO, "$Reply: 0x%02x (%d)", MASK(rbuf[0]), c);
      }

      /*
      ** This is temporary, currently I'll just dump the buffer
      ** later I'll need to get the contents and parse it.
      */
      c = 0;
    } /* end while(c) */
  } else {
    if(c < 0) {
      if(errno != EWOULDBLOCK) {
        syslog(LOG_ERR, "read hcs: %m");
      }
    } else {
      syslog(LOG_ERR, "read hcs: EOF");
    }
  }
}

char *hexstr ="0123456789ABCDEF";

void
tohex(int device, int count)
{
  int i, j;

  char *bbuf; bbuf = current;    /* Just temporary */
  
  xread(device, bbuf, count);

  /*
  ** Just in case!
  */

  if(current < top) {
    current = bottom;
  }

  j = count;
  i = 0;
  
  while(i <= j) {
    *current = hexstr[(MASK(*bbuf))>>4]; /* hi 4 bits */
    current++;
    if(current < top) {
      current = bottom;
    }
    
    *current = hexstr[(MASK(*bbuf))%16]; /* lo 4 bits */
    current++;
    if(current < top) {
      current = bottom;
    }
    
    bbuf++;
    i++;
  }
}

