/*
**
** Author:      Neil Cherry
** Date:        10/16/99
** Version:     1.1 (sub alpha)
**
** $Id: hcs_defs.h,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
** $Log: hcs_defs.h,v $
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
** I've added a few defines for the client commands and not much else.  I
** will have to  change hcsd.c so the it uses the  hcs defines instead of
** the hex numbers.
**
** Revision 1.1  1999/10/16 16:37:32  njc
** Initial revision
**
**
*/

/*[ tcpd/tcp wrappers stuff ]************************************************/

#ifdef TCPD
  #ifndef TRUE
    #define TRUE	1
  #endif

  #ifndef FALSE
    #define FALSE	(!TRUE)
  #endif

  #define NOT_ALLOWED	(!TRUE)
  #define ALLOWED	(TRUE)

#endif

/*[ Various definitions ]****************************************************/

#ifndef STDIN
  #define STDIN 0
#endif

#ifndef STDOUT
  #define STDOUT 1
#endif

#ifndef STDERR
  #define STDERR 2
#endif

#ifndef	LOG_LOCAL0
  #define LOG_LOCAL0 0
#endif

/*
** I should really use mallocs instead of this
*/

#ifndef MAXLINE
  #define MAXLINE 256
#endif

#define SUCCESS		'^'
#define BADVERSION	'#'
#define TOOBIG		'@'

#define TIME		0x80
#define X10		0x81
#define DigIn		0x82
#define DIGITALIN	0x82
#define DigOut		0x83
#define DIGITALOUT	0x83
#define AnalIn		0x84
#define ANALOGIN	0x84
#define AnalOut		0x85
#define ANALOGOUT	0x85
#define NetMods		0x86
#define NETMODULES	0x86
#define NetBits		0x87
#define NETBITS		0x87
#define CONMSG		0x88
#define CONSOLE		0x88

/*
** Used by hcs.c
*/

#define USETTIME	't'
#define ULOAD		'l'
#define CLRINFO		'c'
#define CLRSTAT		'C'
#define QUIT		'q'

/*
** Used by hcsd
*/

#define xQUIET		'Q'
#define xSTATUS		'S'
#define xHELP		'?'
#define xTIME		'T'
#define xGET		'G'
#define xPUT		'P'
#define xLOAD		'L'
#define xRELOAD		'R'
#define xEXIT		'X'

#define CMD		'!'
#define REPLY		'$'

#define EOL		"\r\n"
#define DEL		0x7F

#define RESET		0x00
#define PAUSE		0x01
#define FULLSTATUS	0x02
#define SELECTTIME	0x03
#define GETTIME		0x04
#define SETTIME		0x05
#define LOAD		0x06
#define CLRLOG		0x07
#define SIZELOG		0x08
#define GETLOG		0x09

#define PUTLOG		0x0A

#define SELECTX10	0x10
#define GETX10		0x11
#define SETX10		0x12

#define SELECTDI	0x13
#define GETDI		0x14
#define SETDI		0x15

#define SELECTDO	0x16
#define GETDO		0x17
#define SETDO		0x18

#define SELECTADC	0x19
#define	GETADC		0x1A
#define SETADC		0x1B

#define SELECTDAC	0x1C
#define GETDAC		0x1D
#define SETDAC		0x1E

#define SELECTNETMOD	0x1F
#define GETNETMOD	0x20

#define SELECTNETBIT	0x21
#define GETNETBIT	0x22
#define SETNETBIT	0x23

#define GETVAR		0x24
#define SETVAR		0x25

#define NETSTR		0x30
#define VOICESTR	0x31

/*
** Maybe this should be a typedef instead
*/

#define CLIENTS (struct clients *)

#define MASK(s)         (0x000000FF & s)


/*[ Linux definitions ]******************************************************/

#ifndef __linux__               /* I think this should really be gcc not linux */
  /*
  ** Maybe this should be a typedef instead
  */

  #define FD_NULL	(struct fd_set *) 0
  #define TIMEVAL_NULL	(struct timeval *) 0
#else
  #define FD_NULL	(fd_set *) 0
  #define TIMEVAL_NULL	(struct timeval *) 0
#endif


extern char hcsfile[];

#define EVENTS_BIN hcsfile

#define LOAD_OK	"^\r\n"
