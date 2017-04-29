/*
** Author:	Neil Cherry
** Date:	11/25/99
** Version:	1.6 (alpha)
**
**      $Id: badclient.c,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
**      $Log: badclient.c,v $
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
**      This code is  used to close a clients connection  and remove them from
**      the available clients array.
**
*/

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "hcs_defs.h"
#include "clients.h"

extern int nclient;

extern struct clients *clients;

/*
** Close wayward client connections
*/

void
badclient(int fd, fd_set *rset)
{
  int c;

  close(clients[fd].s);                  /* What's the difference between this and shutdown */

  FD_CLR(clients[fd].s, rset);

  nclient--;

  for(c = fd; c < nclient; c++)
    clients[c] = clients[c + 1];
}
