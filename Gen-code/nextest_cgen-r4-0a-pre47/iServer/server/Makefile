#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk

OBJECTS 	= daemon.o pktsend.o net.o ipstring.o lex.addrtype.o
LIBMEMOBJS	= mem.o memdb.o routematch.o cache.o cacheinit.o lkup.o rhandles.o locks.o resutils.o gw.o hashtable.o triggers.o clist.o dbs.o
LIBDBOBJS 	= db.o 
LIBUTILSOBJS 	= log.o callutils.o export.o callid.o handles.o
LIBFLOCKOBJS = flock.o
LIBLOCKOBJS	= locks.o

all: $(SERVERLIBS) $(LIBUTILS)

lib install:  $(LIBSRVR) $(LIBUTILS) $(LIBMEM) $(LIBDB) $(LIBLOCK) $(LIBFLOCK)

$(LIBSRVR):	$(OBJECTS)
	$(AR) ruv $(LIBSRVR) $(OBJECTS)

$(LIBMEM): $(LIBMEMOBJS)
	$(AR) -r $@ $^

$(LIBDB): $(LIBDBOBJS)
	$(AR) -r $@ $^

$(LIBUTILS): $(LIBUTILSOBJS)
	$(AR) -r $@ $^

$(LIBLOCK): $(LIBLOCKOBJS)
	$(AR) -r $@ $^

$(LIBFLOCK): $(LIBFLOCKOBJS)
	$(AR) -r $@ $^

lex.addrtype.c: addrtype.l
	flex -CFr -Paddrtype $^

test:
	gcc $(CFLAGS) -g test.c
clean:	
	$(RM) $(OBJECTS) $(LIBMEMOBJS) $(LIBDBOBJS) $(LIBUTILSOBJS) $(LIBLOCKOBJS) $(LIBFLOCKOBJS)

depend:
	$(MAKEDEPEND) $(CFLAGS) *.c


# DO NOT DELETE
