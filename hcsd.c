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
** I should probably provide a way to permit the user to select between ASCII
** converted data and raw data.
**
*/

char vcid[]  = "$Id: hcsd.c,v 1.2.1 2017/09/19 19:04:12 ncherry Exp $";   /* Just the ident string   */
char hcsid[] = "$Revision: 1.2.1 $";   			    /* Just the version string */

#define IPV6	1
#define SERVER_PORT     (4098) // @FIXME: 4099 used by mochad but this needs to be a passed option

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

//#if defined(IPV6)
#if IPV6
    //struct sockaddr_in6 servAddr, cliaddr;
    struct sockaddr_in6 servAddr;
#else
    //struct sockaddr_in cliaddr, servAddr;
    struct sockaddr_in servAddr;
#endif

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

#if 0
    /*
    ** this section (socket/bind/listen) was stolen from Dan Lanciani's
    ** X10D. I had a hard time modifying it because I couldn't grok
    ** it. Now I have a few pointers so I can rewrite it. I'll hope to
    ** be doing that in the near future!
    */

    servAddr.sin_family = PF_INET;
    servAddr.sin_port   = sp->s_port;
    memset(&(servAddr.sin_zero), 0, 8);        /* zero the rest of the struct */
#else
#if IPV6
    /* -------------------------------------------------------------------- */
    /* Listen socket for IPv4/IPV6 clients */

    // http://publib.boulder.ibm.com/infocenter/iseries/v6r1m0/topic/rzab6/xacceptboth.htm
    // http://tldp.org/HOWTO/html_single/Linux+IPv6-HOWTO/#CHAPTER-PROGRAMMING
    // may be of interest: network programming
    // http://www.cs.utah.edu/~swalton/listings/sockets/programs/

    /*
    ** Careful! This part is not ready yet!
    */

    int on = 1;

    /*********************************************************************/
    /* After the socket descriptor is created, a bind() function gets a  */
    /* unique name for the socket.  In this example, the user sets the   */
    /* address to in6addr_any, which (by default) allows connections to  */
    /* be established from any IPv4 or IPv6 client that specifies port   */
    /* SERVER_PORT. (that is, the bind is done to both the IPv4 and IPv6 */
    /* TCP/IP stacks).  This behavior can be modified using the          */
    /* IPPROTO_IPV6 level socket option IPV6_V6ONLY if required.         */
    /*********************************************************************/

    // As per:
    // http://publib.boulder.ibm.com/infocenter/iseries/v6r1m0/topic/rzab6/xacceptboth.htm

    memset(&servAddr, 0, sizeof(servAddr));

    servAddr.sin6_family = AF_INET6;
    servAddr.sin6_addr   = in6addr_any;
    servAddr.sin6_port   = htons(SERVER_PORT); // Same server port as IPv4
#else
    /* -------------------------------------------------------------------- */
    /* Listen socket for IPv4 */
    // see lsocket below
    //listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servAddr, 0, sizeof(servAddr));

    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port        = htons(SERVER_PORT);
#endif
#endif

    /*
    ** To accept connections, a socket is first created with socket(2),
    ** a willingness to accept incoming connections and a queue limit
    ** for incoming connections are specified with listen, and then the
    ** connections are accepted with accept(2).  The listen call applies
    ** only to sockets of type SOCK_STREAM or SOCK_SEQPACKET.
    */

    //if((lsocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    if((lsocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	syslog(LOG_ERR, "socket: %m");
	exit(errno);
    }

    /********************************************************************/
    /* The setsockopt() function is used to allow the local address to  */
    /* be reused when the server is restarted before the required wait  */
    /* time expires.                                                    */
    /********************************************************************/
    if(setsockopt(lsocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
	perror("setsockopt(SO_REUSEADDR) failed");
	exit(errno);
    }

    // sin should be struct socket_addr * but I have it as struct
    // sockaddr_in * Weird thing is that both structure are byte-wise
    // the same just built differently

    if(bind(lsocket, (struct sockaddr *)&servAddr, sizeof(address))) { // &sin gives incompatible pointer type, ignore it
	syslog(LOG_ERR, "bind: %m");
	exit(errno);
    }

    // @FIXME: Here we allow 5 simultaneous connections but elsewhere we support 20+ clients.
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
new_client(fd_set *rset) {
    socklen_t l;
  int c;
  
  l = sizeof(sin);

  //if((c = accept(lsocket, &sin, &l)) >= 0) {
  if((c = accept(lsocket, (struct sockaddr *)&servAddr, &l)) >= 0) {
    ioctl(c, FIONBIO, NON_BLOCKING);

#ifdef TCPD
    /*
    ** TCPD or TCP Wrappers - man hosts_access
    ** for further details
    */
#error change sin to servAddr, correct, and fix for IPV6
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
	// @FIXME: support for 20 more clients? What's going on here?
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
existing_client(fd_set *rset, fd_set *probe, struct clients *clients, int device)
{
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
do_device(int device)
{
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
    // @FIXME: We're not doing any parsing, this is the client parsing
}
