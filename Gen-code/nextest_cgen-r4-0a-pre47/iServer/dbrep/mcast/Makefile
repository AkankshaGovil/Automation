
include $(BASE)/global.mk
include $(DBREPDIR)/Make.defines

INCLDIR = -I./include -I../include -I../../include -I../../../cli -I-
LIBDIR = -L$(DBREPDIR)/lib/$(TARGET) 
MYBINDIR = $(DBREPDIR)/bin/$(TARGET)
BINSNIFF = $(BASE)/bin/$(TARGET)/sniff

ISRVRLIBS = \
			-lsconfig \
			-lcli \
			-ldb \
			-lshm \
			-lavl \
			-ltavl \
			-ltst \
			-lsconfig \
			-lsrvr \
			-lcommon \
			-losd \
			-lxml \
			-lutils \
			-lslocks \
			-lexecd \
			-lcrypto \
			$(DBMALLOCLIB)\


ifeq ("X$(PLATFORM)", "Xlinux")
MYLIBS = 	-lresolv -lhello -lmcast -lgen -lrt\
			-L$(BASE)/lib/$(TARGET) -lrsd $(ISRVRLIBS) $(LDLIBS)\
			-ldummy
else
MYLIBS = 	-lresolv -lhello -lmcast -lgen -lrt\
			-L$(BASE)/lib/$(TARGET) -lrsd -lrsdrpc $(ISRVRLIBS) $(LDLIBS)\
			-ldummy
endif

CFLAGS += ${INCLDIR} ${LIBDIR} ${PROFFLAGS}

VPATH = $(CURRDIR)/../src:$(BASE)/cli
SRCDIR = .

CLEANFILES += $(SRCDIR)/*.o 

LIBRSD_NAME = $(BASE)/lib/$(TARGET)/librsd.a

PROGS =  rsd

LIBRSD = librsd.a

#FUNKY_OBJS	=	clilog.o
LOCAL_OBJS	=	main.o process.o rsd_rpc.o pkt.o $(BASE)/lm/$(TARGET)/ex_data.o $(FUNKY_OBJS)
LIBRSD_OBJS =   hist.o flock.o
HISTDB_OBJS =	histdb.o

exec all:	${LIBRSD} ${PROGS} 

lib: 	${LIBRSD}

rsd:	${LOCAL_OBJS}
		${CC} ${CFLAGS} ${LOCAL_OBJS} ${MYLIBS} -o $@
		cp rsd $(BINRSD)
		mv rsd $(MYBINDIR)

sniff:	test.o	
		${CC} ${CFLAGS} test.o ${MYLIBS} -o $@
		cp sniff $(BINSNIFF)
		mv sniff $(MYBINDIR)

histdb:	${HISTDB_OBJS}
		${CC} ${CFLAGS} ${HISTDB_OBJS} ${MYLIBS} -o $@
		cp histdb $(BINHISTDB)
		mv histdb $(MYBINDIR)

test:	test.c
		${CC} ${CFLAGS} -o test.o test.c ${MYLIBS}
		mv test $(MYBINDIR)

librsd.a:	${LIBRSD_OBJS}
			ar rv ${LIBRSD_NAME} $?
			${RANLIB} ${LIBRSD_NAME}

clean:
		rm -f ${PROGS} ${CLEANFILES}
