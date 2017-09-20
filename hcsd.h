/*
**
** Author:      Neil Cherry
** Date:        10/16/99
** Version:     1.1 (sub alpha)
**
** $Id: hcsd.h,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
** $Log: hcsd.h,v $
** Revision 1.1.1.1  2008/09/24 19:04:13  ncherry
** Imported using TkCVS
**
** Revision 1.1.1.2  2000/03/18 14:22:03  ncherry
**
** Added hcs_dl.c which is a standalone program to download an events.bin file
** to an HCS II connected to a local serial port.
**
** Revision 1.1.1.1  2000/03/13 13:05:58  ncherry
** This is the inital base release. It is nothing more that some of my (njc)
** existing code from my HCS II project. It is the code without the pthread
** functions. It has problems with character drops on the receive serial port.
**
**
** Revision 1.6  1999/11/25 17:32:14  njc
** I've added quite a few  routines definitions to this file.  This files
** purpose is to remove the clutter from the hcsd (main) code.
**
** Revision 1.1  1999/10/31 14:41:41  njc
** Initial revision
**
**
*/

#ifndef HCSD_H
#define HCSD_H

//static struct sockaddr_in s_in = { PF_INET };
static struct sockaddr address;

/*[ tcpd/tcp wrappers stuff ]************************************************/

#ifdef TCPD
  #include <tcpd.h>

  int     allow_severity = LOG_INFO; 	/* These are needed for libwrap.a and */
  int     deny_severity  = LOG_WARNING; /* must be defined here .             */
#endif

#ifndef TRUE
#define TRUE 1
#endif

/*[ Explicit declarations ]**************************************************/

void new_client(fd_set *rset);
void existing_client(fd_set *rset, fd_set *probe, struct clients *clients, int device);
void do_device(int device);
void parse(char *str, int c, int device, fd_set *rset);

extern void fini(int);
extern void load(int device, int user);
extern void xload(int device, int user, int size);
extern void getlog(int device, int user) ;
extern void sethcsclock(int device);
extern void hcsoptions(int argc, char **argv);
extern void help(int user);
extern void putxpress(int user, int device);
extern void getxpress(int user, int device);
extern void badclient(int fd, fd_set *rset);
extern void conmsg(int device);

extern int htoi(char *cmd, char *str);
extern int hosts_ctl(char *port, char *, char *ip_addr, char *);
extern int waitfor_rd(int fd, int nsec, int usec);
extern int waitfor_wr(int fd, int nsec, int usec);
extern int stoi(char c);
extern int fdprintf(int fd, const char *format, ...);
extern int dwrite(int device, char *str);
extern int hcsd_setup(char *xline);

extern ssize_t xread(int fd, char *buf, size_t count);
extern ssize_t clwrite(const char *rbuf, size_t count);

extern char line[];
extern char port[];
extern char *lock;

extern int interval;
#endif
