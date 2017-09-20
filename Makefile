# Linux HA (Lhapso)
# 03/12/00
#
# I started this project as a derivitive of the HCSD project

CFLAGS	=	-g -O3 -Wall
LFLAGS	=	-g

# By default the TCP/IP SERVICE name is "hcsd"
# to change it set SERVICE = -DSERVICE="port_name"
SERVICE =

# By default the DEVICE name is "/dev/hcsd"
# to change it set DEVICE = -DDEVICE="/dev/device_name"
DEVICE  =

# Hey TCPD wants yp! What's up with that?
#DEFS	= -DTCPD -DUUCP $(DEVICE) $(SERVICE)
DEFS	= -DUUCP $(DEVICE) $(SERVICE)
#LIB	=	-lwrap -lnss_nis
#LIB	=	-lnss_nis
LIB	=
NOM	=	hcsd

ALLSRC	=	hcsd.h hcsd.c fini.c hcsd_setup.c  badclient.c options.c \
		xload.c hcsd_io.c hcsclock.c uucp_locking.c putxpress.c \
		conmsg.c hcsd_sio.c Makefile

# HCSD software (unrelated to HCS client software)
HCS_OBJS=	hcsd.o funcs.o options.o fini.o xload.o badclient.o hcsd_setup.o \
		hcsd_io.o hcsclock.o uucp_locking.o putxpress.o conmsg.o hcsd_sio.o

# ***************************************************************************
# This is currently not even alpha, it at least compiles

hcsd:	$(HCS_OBJS)
	cc $(CFLAGS) -o hcsd $(HCS_OBJS) $(LIB)

# ***************************************************************************
# Type in just make and this will compile (this is temporary)

all:	$(NOM)

clean:
	rm -rf a.out *.o old hcs hcsd core foo bar *~

# ***************************************************************************
# Various junk I need, probably not much use to others

tar:	clean
	echo 'cd .. ; bar=`find lha -type f|fgrep -v bak|fgrep -v RCS|fgrep -v "#"` ; tar zcvf lha.tgz $${bar}' | sh

bak:	$(ALLSOURCE) clean
	echo 'cd .. ; bar=`find lha -type d` ; date=`date "+%Y%m%d"` ; tar zcvf lha.$${date}.tgz $${bar}'  | sh

# Use this to check in
# cat descriptive_file.txt | ci -u1.0 -s<state> -t-<package name> file 
# ***************************************************************************

hcsd.o:		hcsd.c hcsd.h hcs_defs.h globals.h
	cc $(CFLAGS) $(DEFS) -c $*.c

funcs.o:	funcs.c  hcs_defs.h
	cc $(CFLAGS) $(DEFS) -c $*.c

options.o:	options.c options.h hcs_defs.h
	cc $(CFLAGS) $(DEFS) -c $*.c

fini.o:		fini.c
	cc $(CFLAGS) $(DEFS) -c $*.c

hcsd_setup.o:	hcsd_setup.c uufuncs.h
	cc $(CFLAGS) $(DEFS) -c $*.c

hcsclock.o:	hcsclock.c hcs_defs.h
badclient.o:	badclient.c hcs_defs.h
hcsd_io.o:	hcsd_io.c hcs_defs.h
xload.o:	xload.c hcs_defs.h
uucp_locking.o:	uucp_locking.c uufuncs.h
putxpress.o:	putxpress.c hcs_defs.h
hcsd_sio.o:	hcsd_sio.c
