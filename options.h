/*
**
** Author:	Neil Cherry
** Date:	10/16/99
** Version:	1.1 (sub alpha)
**
** $Id: options.h,v 1.1.1.1 2008/09/24 19:04:12 ncherry Exp $
**
** $Log: options.h,v $
** Revision 1.1.1.1  2008/09/24 19:04:12  ncherry
** Imported using TkCVS
**
** Revision 1.1.1.2  2000/03/18 14:22:04  ncherry
**
** Added hcs_dl.c which is a standalone program to download an events.bin file
** to an HCS II connected to a local serial port.
**
** Revision 1.1.1.1  2000/03/13 13:05:59  ncherry
** This is the inital base release. It is nothing more that some of my (njc)
** existing code from my HCS II project. It is the code without the pthread
** functions. It has problems with character drops on the receive serial port.
**
**
** Revision 1.6  1999/11/25 17:32:14  njc
** This file contains definitions for use with the options function.
**
** Revision 1.1  1999/10/16 15:46:43  njc
** Initial revision
**
*/

#define HCSDEVICE	'd'
#define EVENTSFILE	'f'
#define LOCK		'l'
#define PORT		'p'
#define SPEED		's'
#define DEBUG		'x'
#define VERSION		'v'
#define INTERVAL	'i'
#define HELP		'?'

#define OFF		0
#define SECOND		1
#define MINUTE		2

/*
** These 2 allow you to select the service and device
**
** Later I'll change this to command line options
*/

#ifndef SERVICE
  #define SERVICE "hcsd"
#endif

#ifndef DEVICE
  #define DEVICE "/dev/hcsd"
#endif

#ifndef HCSFILE
  #define HCSFILE "/usr/local/hcs/events.bin"
#endif

char line[MAXLINE];
char port[MAXLINE];
char lock[MAXLINE];
char hcsfile[MAXLINE];

int file_state = 0;

int speed=9600, debug=0;
int interval = MINUTE;
