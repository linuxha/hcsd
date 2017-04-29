/*
**
** Author:      Neil Cherry
** Date:        10/16/99
** Version:     1.1 (sub alpha)
**
** $Id: globals.h,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
** $Log: globals.h,v $
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
** Revision 1.6  1999/11/25 17:32:14  njc
** No changes for this file for now.  Later the structure of clients will
** change.
**
** Revision 1.2  1999/10/31 14:10:45  njc
** I've started to work on the arrays and trying to eliminate them were
** possible. I found one in globals.h (here). Because a change here would
** affect so many lines of code I've decided to double itss size but
** limit input to MAXLINE (if I can).
**
**
*/

/*[ Various global declarations ]********************************************/

char rbuf[MAXLINE], buf[MAXLINE];	/* possible point for buffer overflows ??? */

int lsocket, n, nclient, maxclient = 20, firmware;

const int on = 1, off = 0;

#define NON_BLOCKING &on

static struct sockaddr_in sin = { PF_INET };
struct linger linger = { 1, 0 };
static fd_set xprobe;

char *ctime();
void *memchr(), *malloc();
char *xdate = __DATE__;
