/*
** hcs.h - Various window position defines and other strange items
**
** HCS - HCS II Interface software of Unix
**
** Original author is unknown and I didn't like the way he formated
** the code. So the first thing I did was to reformat it and remove
** a bunch of useless defines (redundant) and other oddities. It was
** also basically uncommented. This was written to work with V2.1 of
** the HCS Express code. I currently have a V3.x code, so that's where
** I'll base my code.
**
** Author:	Neil Cherry
** Date:	10/16/99
** Version:	1.3 (sub alpha)
**
**      $Id: hcs.h,v 1.1.1.1 2008/09/24 19:04:13 ncherry Exp $
**
**	$Log: hcs.h,v $
**	Revision 1.1.1.1  2008/09/24 19:04:13  ncherry
**	Imported using TkCVS
**	
**	Revision 1.1.1.2  2000/03/18 14:22:01  ncherry
**	
**	Added hcs_dl.c which is a standalone program to download an events.bin file
**	to an HCS II connected to a local serial port.
**	
**	Revision 1.1.1.1  2000/03/13 13:05:56  ncherry
**	This is the inital base release. It is nothing more that some of my (njc)
**	existing code from my HCS II project. It is the code without the pthread
**	functions. It has problems with character drops on the receive serial port.
**	
**	
**	Revision 1.0  1999/11/28 00:12:22  njc
**	Initial revision
**
**
*/

#include <errno.h>
//extern int errno;

#if !defined(word)
typedef unsigned short word;
#endif

#ifndef uchar
typedef unsigned char uchar;
#endif

#ifndef CHARPN
#  define CHARPN (char *) NULL
#endif
#ifndef FILEPN
#  define FILEPN (FILE *) NULL
#endif

WINDOW *statwin;
WINDOW *infowin;

#define TITLEROW	0

#define TIMEWIDTH   34
#define TIMECOL     (COLS-TIMEWIDTH-1)
#define TIMEROW     0

#define DIGINROW    1
#define DIGINCOLP   0
#define DIGINCOL(a) (11+(((a)%3)*12))

#define DIGOUTROW   2
#define DIGOUTCOLP  0
#define DIGOUTCOL(a) (11+(((a)%3)*12))

#define X10ROW      1
#define X10COL     48

#define ANINROW     3
#define ANINCOL     0

#define NETROW      4
#define NETCOL      0


struct hcsin_struct {
  struct {
    uchar hsec, sec, min;
    uchar hr, dow, day, mon, yr;
  } daytime;

  struct {
    uchar lo, hi;
  } x10[16];

  struct {
    uchar modnum:4;
    uchar modtyp:4;
    uchar stat;
  } net[17];

  struct {
    uchar stat;
  } din[32];

  struct {
    uchar stat;
  } dout[32];

  struct {
    word chan[16];
  } ain[3];
} hcsin;

#define CMD	'!'
#define REPLY	'$'

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

#define SETTIME		't'
#define LOAD		'l'
#define CLRINFO		'c'
#define CLRSTAT		'C'
#define QUIT		'q'
#define HELP		'?'

#define DEVICE		'1'
#define PORT		'2'
#define MYLINE		'l'
#define SPEED		's'
#define PIPE		'p'
#define INTERP		'i'
#define DEBUG		'd'
