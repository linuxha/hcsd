/*
**
** Author:	Neil Cherry
** Date:	10/16/99
** Version:	1.1 (sub alpha)
**
** $Id: options.c,v 1.1.1.1 2008/09/24 19:04:12 ncherry Exp $
**
** $Log: options.c,v $
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
** I've added further options such as the filename/path of the events.bin
** file.  I still need to add the supporting functions to take advantage
** of these options (such as the time interval).
**
** Revision 1.3  1999/10/17 15:26:52  njc
** I've added the version information into the hcsd.c (main()) file so I
** can print out the version from the command line. I'm also going to add
** exits to HELP, VERSION, and the invalid options.
**
** Revision 1.2  1999/10/16 16:33:17  njc
** This really is the initial release of the options function. I've now
** turned on the -Wall in the makefile and I've cleaned up the options
** file so it compiles with no errors. I now need to perform some testing
** to make sure it's working properly. I've also removed the SERVICE and
** DEVICE defines from hcsd.c. I've replaced them with port and line
** respectively. We can now pass command line options to change the
** device we're using and the tcp/ip port we want to use. I haven't
** tested it with the port number instead of the port name.
**
*/

#include <getopt.h>             /* as referenced in the getop man page */
#include <stdlib.h>             /* used by atoi */
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "hcs_defs.h"
#include "options.h"

extern char hcsid[];
extern char *xdate;

void
clihelp() {
  fprintf(stderr,"Help!\ndevice\nfile\nportlock\nspeed\ndebug\nverbose\ninterval\nversion\n");
}

void
hcsoptions(int argc, char **argv) {
  int c, option_index = 0;

  static struct option long_options[] = {
    {"device",	1, 0, HCSDEVICE},
    {"file",	1, 0, EVENTSFILE},
    {"port",	1, 0, PORT},
    {"lock",    1, 0, LOCK},
    {"speed",	1, 0, SPEED},
    {"debug",	1, 0, DEBUG},
    {"verbose",	1, 0, DEBUG},
    {"interval",1, 0, INTERVAL},    
    {"version",	0, 0, VERSION},
    {"help",	0, 0, HELP},      
    {0,		0, 0, 0}
  };

  strcpy(line, DEVICE);
  strcpy(port, SERVICE);
  strcpy(hcsfile, HCSFILE);
  
  /*
  ** handle options here before we fork
  **
  ** The following options should be available:
  ** --device -d - device name to connect to
  **               defaults to /dev/hcsd
  ** --port -p   - port to listen on
  **               defaults to hcsd
  ** 
  ** --help -h -?  Issues help
  */

  while(1) {                    /* Forever loop */
    c = getopt_long (argc, argv, "d:s:p:x:i:vh?", long_options, &option_index);
    if(c == (-1))
      break;

    switch(c) {
      case PORT:                /* The TCP/IP port name or number */
        if(strlen(optarg) < MAXLINE)
          strcpy(port, optarg);
        break;
        
      case HCSDEVICE:           /* the /dev/HCSDEVICE anme */
        if(strlen(optarg) < MAXLINE)
          strcpy(line, optarg);
        break;

      case EVENTSFILE:          /* default: */
        if(strlen(optarg) < MAXLINE)
          strcpy(hcsfile, optarg);
        break;
        
      case LOCK:
        if(strlen(optarg) < MAXLINE)
          strcpy(lock, optarg);
        break;
        
      case SPEED:               /* The speed at which to communicate */
        speed = atoi(optarg);
        break;

      case DEBUG:
        debug = atoi(optarg);
        if(!debug)
          debug++;
        break;

      case VERSION:
        printf("Version: %s %s\n", hcsid, xdate);
        exit(EPERM);
        break;

      case INTERVAL:
        interval = atoi(optarg);
        switch(interval) {
          case OFF:
            interval = OFF;
            break;

          case SECOND:
            interval = SECOND;
            break;

          case MINUTE:
            interval = MINUTE;
            break;

          default:
            interval = OFF;
            break;
        }
        break;
        
      default:                  /* Bad option */
        fprintf(stderr,"Bad option '%c'\n",**argv);
      case 'h':                 /* Help screen */
      case HELP:
	clihelp();
        exit(EPERM);
        break;
    }
  }
}
