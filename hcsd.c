/*
** hcsd.c - A daemon that uses to TCP/IP socket to allow bidirectional
** 	    communication to the Circuit Cellar HCS II Home Controller.
**
** HCSD will use a TCP/IP port to permit users to communicate with the HCS
** boards. A person (or program) could telnet (use TCP) to connect to the
** monitored port. Multiple users are permitted to connect simultaneously. A
** level of security will be provided by using tcpd though this should not be
** the only level of security (I suggest adding ipchains for Linux or something
** similar for BSD).
**
** I should probably provide a way to permit the user to select between ascii
** converted data and raw data.
**
*/

char vcid[]  = "$Id: hcsd.c,v 1.1.1.1 2008/09/24 19:04:12 ncherry Exp $";   /* Just the ident string   */
char hcsid[] = "$Revision: 1.1.1.1 $";   			    /* Just the version string */

#include <ctype.h>              /* needed for toupper */
#include <fcntl.h>              /* used by open       */
#include <sys/stat.h>           /* used by open       */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <termios.h>
#include <assert.h>

#include "hcs_defs.h"
#include "globals.h"
#include "clients.h"
#include "hcsd.h"

/*[ Code starts here ]*******************************************************/

int
main(int argc, char **argv) {
  int device;
  
  struct servent *sp;
  fd_set rset, probe;

  hcsoptions(argc, argv);
  openlog("hcsd", LOG_PID, LOG_LOCAL0);

  syslog(LOG_INFO, "%s start", port);

  /*
  ** Here is where we handle all the setup for the hcsd
  ** We also handle the opening up of the port
  */

  device = hcsd_setup((char *) line);

  /*
  ** I should malloc rbuf and buf here
  */
  
  if(!(clients = CLIENTS malloc(maxclient * sizeof(struct clients)))) {
    syslog(LOG_CRIT, "no memory for clients");
    exit(errno);
  }

  FD_ZERO(&rset);
  FD_ZERO(&xprobe);

  if(!(sp = getservbyname(port, "tcp"))) {
    syslog(LOG_ERR, "%s: unknown service", port);
    exit(errno);
  }

  sin.sin_port = sp->s_port;

  /*
  ** To  accept  connections,  a  socket  is first created with socket(2), a willingness to
  ** accept incoming connections and a queue limit for incoming connections  are  specified
  ** with  listen,  and  then the connections are accepted with accept(2).  The listen call
  ** applies only to sockets of type SOCK_STREAM or SOCK_SEQPACKET.
  */

  if((lsocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    syslog(LOG_ERR, "socket: %m");
    exit(errno);
  }

  if(bind(lsocket, &sin, sizeof(sin))) {
    syslog(LOG_ERR, "bind: %m");
    exit(errno);
  }

  listen(lsocket, 5);
  /*
  ** FIONBIO ioctl function controls nonblocking I/O on a socket.
  **   The Enable argument is a pointer to an integer that specifies
  **   whether nonblocking I/O should be enabled or disabled.  A
  **   value of 1 enables nonblocking I/O, and a value of 0 disables
  **   nonblocking I/O.  By default, nonblocking I/O is disabled when
  **   a socket is created.
  */
  ioctl(lsocket, FIONBIO, NON_BLOCKING);
  FD_SET(lsocket, &rset);

  FD_SET(device, &rset);
  sethcsclock(device);

  /*
  ** select does the job of 'forking' the process for us.
  ** everytime a new client attaches they run a copy of this
  ** process. (need to double check that this is true).
  */
  
  while(TRUE) {
    probe = rset;

    if(select(FD_SETSIZE, &probe, FD_NULL, FD_NULL, TIMEVAL_NULL) <= 0) {
      syslog(LOG_ERR, "select: %m");
      sleep(2);
      continue;
    }

    /************************************************************************/

    /*
    ** here is where we do our work if the device contacts us **************
    */
    
    if(FD_ISSET(device, &probe)) {
      do_device(device);
    }

    /*
    ** Here is where we do our work if someone new connects on the
    ** listening socket
    */
    
    if(FD_ISSET(lsocket, &probe)) {
      new_client(&rset);
    } /* End of listening code */
    
    /*
    ** This is for the existing connections ********************************
    */
    
    existing_client(&rset, &probe, clients, device);
    
    /************************************************************************/
  }
}

void
new_client(fd_set *rset)
{
  int l, c;
  
  l = sizeof(sin);

  if((c = accept(lsocket, &sin, &l)) >= 0) {
    ioctl(c, FIONBIO, NON_BLOCKING);

#ifdef TCPD
    /*
    ** TCPD or TCP Wrappers - man hosts_access
    ** for further details
    */

    if(hosts_ctl(port, STRING_UNKNOWN, inet_ntoa(sin.sin_addr),
                 STRING_UNKNOWN) == ALLOWED) {
#else
    /*
    ** This is so a user who doesn't have tcp wrappers can compile
    ** the code.
    */

    if(TRUE) {
#endif
      if(nclient >= maxclient) {
        maxclient += 20;
        if(!(clients = (struct clients *)realloc(
          (char *)clients, maxclient * sizeof(struct clients)))) {
          syslog(LOG_CRIT,"out of client memory");
          exit(ENOMEM);
        }
      }

      setsockopt(c, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
      clients[nclient].flags = clients[nclient].cnt = 0;
      clients[nclient].s = c;

      /*
      ** The hello message and no prompt.
      ** I've removed the prompt because it was actually more confusing than
      ** not. This behavior is now more like that of the smtp port daemon.
      */

      fdprintf(clients[nclient++].s, "HCSD\r\n"); /* Print the title */

#ifdef LOGIN
      /*
      ** The login/passwd code goes here
      */

      /*
      ** The Read/Write access code goes here.
      */
#endif
      FD_SET(c, rset);
    } else {
      close(c);
    }
  }
  
/*[ End of Security ]**************************************************/
}

void
existing_client(fd_set *rset, fd_set *probe, struct clients *clients, int device) {
  int i, count, l;
  char *p;
  
  for(i = 0; i < nclient; i++) {
    if(FD_ISSET(clients[i].s, probe)) {
      count = sizeof(clients[i].buf) - clients[i].cnt - 1;
      
      if(count <= 0) {
        badclient(i, rset);
        i--;
      }

      count = read(clients[i].s, clients[i].buf + clients[i].cnt, count);

      if(count < 0) {
        if(errno != EWOULDBLOCK) {
          badclient(i, rset);
          i--;
        }
        continue;
      }

      if(count == 0) {
        badclient(i, rset);
        i--;
        continue;
      }
      clients[i].cnt += count;

      while((p = memchr(clients[i].buf, '\n', clients[i].cnt))) {
        count = p - clients[i].buf + 1;
        bcopy(clients[i].buf, rbuf, count);

        if(clients[i].cnt -= count) {
          bcopy(clients[i].buf + count, clients[i].buf, clients[i].cnt);
        }
        
        /*
        ** Looks like a space eater and toupper routine!
        ** Also eats control characters
        **
        ** The only characters I should receive are \r,\n, <SP>
        ** \t, \f (FF), \v (VT) and the normal ASCII characters '!' - '~'
        ** I don't think the extra space characters will hurt the program.
        */

        for(p = buf, l = count, count = 0; count < l; count++) {
          if(isprint(rbuf[count])) {
            *p++ = rbuf[count]; /* Leaves me with just printable chars and space */
          } else {
            *p = 0;
          }
        }

        parse(buf, i, device, rset);  /* Here is where we parse the clients request */
      } 			/* End of while */
    }                           /* End of if(FD_ISSET(...)) */
  } 				/* End of for loop */
}

void
do_device(int device) {
  int c;

  if((c = read(device, rbuf, 1))) {
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

void parse(char *str, int c, int device, fd_set *rset) {

}