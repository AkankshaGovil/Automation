#
# This file is generated by autoconf from "Make.defines.in".
#
# This is the "Make.defines" file that almost every "Makefile" in the
# source directories below this directory include.
# The "../" in the pathnames actually refer to this directory, since
# "make" is executed in all the subdirectories of this directory.
#
# System = i386-pc-solaris2.8

include $(BASE)/global.mk
include $(DBREPDIR)/Make.defines

INCLUDEDIR= -I../include -I./include
CFLAGS += ${INCLUDEDIR}
SRCDIR = $(CURRDIR)/../src
VPATH = $(SRCDIR)

# Following is the directory built from all the object files in the 
# libgen/ directory
LIBGEN_NAME = $(DBREPDIR)/lib/$(TARGET)/libgen.a

# Following are all the object files to create in the libgen/ directory.
LIBGEN_OBJS = 	wrapstdio.o			\
				wrapunix.o			\
				wrapsock.o			\
				wrappthread.o		\
				udp_client.o		\
				udp_serv.o			\
				error.o				\
				sockatmark.o		\
				isfdtype.o			\
				sock_ntop.o			\
				sockfd_to_family.o	\
				unix_serv.o			\
				tcp_serv.o			\
				readline.o			\
				writen.o			\
				sighdl.o			\
				signal.o			\
				getipaddr.o			\
				msleep.o			\
				daemon_init.o

# Following Dirs are added for cleaning
CLEANF = ${CLEANFILES} $(SRCDIR)/../$(TARGET)/*.o $(SRCDIR)/../$(TARGET)/core  

exec all:	${LIBGEN_OBJS}
	ar rv ${LIBGEN_NAME} $?
	${RANLIB} ${LIBGEN_NAME}

lib:

clean:
	rm -f ${PROGS} ${CLEANF}
