/*
**
** Author:      Neil Cherry
** Date:        10/30/99
** Version:     1.6 (sub alpha)
**
** $Id: clients.h,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
** $Log: clients.h,v $
** Revision 1.1.1.1  2008/09/24 19:04:13  ncherry
** Imported using TkCVS
**
** Revision 1.1.1.2  2000/03/18 14:21:59  ncherry
**
** Added hcs_dl.c which is a standalone program to download an events.bin file
** to an HCS II connected to a local serial port.
**
** Revision 1.1.1.1  2000/03/13 13:05:54  ncherry
** This is the inital base release. It is nothing more that some of my (njc)
** existing code from my HCS II project. It is the code without the pthread
** functions. It has problems with character drops on the receive serial port.
**
**
**
*/

#define C_QUIET 0x001

struct clients {
  int  s,                       /* clients fd                    */
       cnt,                     /* n bytes waiting in que        */
       flags,                   /* Flags such as the quiet flag  */
       perm;                    /* Permissions either 0=R or 1=W */
  char buf[MAXLINE];
  char id[MAXLINE];
  int  uid;
} *clients;
