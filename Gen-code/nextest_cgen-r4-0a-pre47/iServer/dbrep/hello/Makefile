include $(BASE)/global.mk
include $(DBREPDIR)/Make.defines

INCLDIR = -I./include -I../include -I../../include -I-
LIBDIR = -L$(DBREPDIR)/lib/$(TARGET) 
MYBINDIR = $(DBREPDIR)/bin/$(TARGET)

ISRVRLIBS =	\
			-lshm \
			-ldb \
			-lavl \
			-ltst \
			-lsconfig \
			-lslocks \
			-lcommon \
			-lsrvr \
			-lutils  $(DBMALLOCLIB)\

SRVRLIB		=	$(LIBMEM) $(LIBDB) $(LIBUTILS) $(LIBSRVR) $(LIBUTIL) $(LIBCOMMON) $(LIBOSD)\
				$(LIBCRYPTO) $(LIBXML) $(LIBSCONFIG) $(LIBTAVL) $(LIBAVL) $(LIBSLOCKS) $(LIBENUM) \
				$(LIBFCE) $(LIBLOCK) $(LIBTST)

MYLIBS		= -lresolv -lsocket -lnsl -lpthread -lhello -lmcast -lgen -lrt \
				-L$(BASE)/lib/$(TARGET) \
				$(ISRVRLIBS) \
				-lgdbm -ldummy

CFLAGS		+= ${INCLDIR} ${LIBDIR} ${PROFFLAGS}

SRCDIR		= $(CURRDIR)/../src
VPATH		= $(SRCDIR)

CLEANFILES	+= *.o 

LIBHELLO_NAME = $(DBREPDIR)/lib/$(TARGET)/libhello.a
LIBDUMMY_NAME = $(DBREPDIR)/lib/$(TARGET)/libdummy.a

#PROGS		=	hello 

LIBHELLO	=	libhello.a
LIBDUMMY	=	libdummy.a

LOCAL_OBJS	=	main.o
LIBHELLO_OBJS	=	hello.o 
LIBDUMMY_OBJS	=	dummy.o 
HISTDB_OBJS =	histdb.o

exec all:	${LIBHELLO} ${LIBDUMMY} ${PROGS} 

lib:		${LIBHELLO}

#hello:	${LOCAL_OBJS}
#		${CC} ${CFLAGS} -o $@ ${LOCAL_OBJS} $(SRVRLIB) 

hello:	${LOCAL_OBJS} 
		${CC} ${CFLAGS} ${LOCAL_OBJS} ${MYLIBS} -o $@

libhello.a:	${LIBHELLO_OBJS}
			ar rv ${LIBHELLO_NAME} $?
			${RANLIB} ${LIBHELLO_NAME}

libdummy.a:	${LIBDUMMY_OBJS}
			ar rv ${LIBDUMMY_NAME} $?
			${RANLIB} ${LIBDUMMY_NAME}

clean:
		rm -f ${PROGS} ${CLEANFILES}
