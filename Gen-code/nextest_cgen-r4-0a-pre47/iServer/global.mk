#################################################################
#################################################################
##
##                      Makefile
##
#################################################################
#################################################################

MAKEDEPEND = makedepend
RM = /bin/rm -f
CP = /bin/cp -p

HPATH = /usr/include

DRINC		= $(BASE)/include
DLIB		= $(BASE)/lib/$(TARGET)
DXML		= $(BASE)/xml
DSLOCK		= $(BASE)/slocks
BINDIR		= $(BASE)/bin/$(TARGET)
CRYPTOTOP	= $(BASE)/crypto
#H323DIR	= $(BASE)/h323-2.6.5.1
#H323DIR	= $(BASE)/h323-3.0
H323DIR		= $(BASE)/$(H323VER)
GKDIR		= $(BASE)/gk
GISDIR		= $(BASE)/ls
SCMRPCDIR	= $(BASE)/ls/scmrpc
SCMDIR		= $(BASE)/scm
ISPDDIR		= $(BASE)/pd
SIPDIR		= $(BASE)/ssip
#SIPSTACKDIR	= $(BASE)/sip-3.1
#SIPSTACKDIR	= $(BASE)/sip-3.1.1
#SIPSTACKDIR	= $(BASE)/sip-3.3
#SIPSTACKDIR	 = $(BASE)/sip-4.1
SIPSTACKDIR	 = $(BASE)/sip-5.2
CLIDIR		= $(BASE)/cli
ENUMDIR		= $(BASE)/enum
IWFDIR		= $(BASE)/iwf
BRIDGEDIR	= $(BASE)/bridge
FCEDIR		= $(BASE)/fce
TAVLDIR		= $(BASE)/tavl
AVLDIR		= $(BASE)/avl-1.4.0
TSTDIR		= $(BASE)/tst1.3
DBREPDIR	= $(BASE)/dbrep
RSYNCDIR	= $(BASE)/rsync
RADCDIR		= $(BASE)/radclient
DBMALLOCDIR   = $(BASE)/dbmalloc/
OSDDIR          = $(BASE)/osd
TAVLDIR		= $(BASE)/tavl
TSMDIR 		= $(BASE)/tsm
SOLARIS_REL	= $(shell uname -r | cut -d. -f2 )
# NATIVE_ADDR_BITS	= $(shell isainfo -b )

#compile time cpp and link flags 
# Next line is for Sip Stack version 5.2 and above.  Use longer list below
# for previous SIP Stack versions.
SIPINCLUDES = -I$(SIPSTACKDIR)/stack_headers
SIPINCLUDES +=  -I$(SIPSTACKDIR)/source/parser/h -I$(SIPSTACKDIR)/source/accessor/h -I$(SIPSTACKDIR)/source/common/h -I$(SIPSTACKDIR)/source/accessor/h -I$(SIPSTACKDIR)/source/common/ccp/h/ -I$(SIPSTACKDIR)/source/common/rpr/h/ -I$(SIPSTACKDIR)/source/accessor/ccp/h -I$(SIPSTACKDIR)/source/accessor/rpr/h

#H323INCLUDES = -I$(H323DIR)/export	# for 2.6.5
#H323INCLUDES = -I$(H323DIR)/include -I$(H323DIR)/mib -I$(H323DIR)/lowunix -I$(H323DIR)/ads -I$(H323DIR)/middle -I$(H323DIR)/cm -I$(H323DIR)/utils -I$(H323DIR)/pdl -I$(H323DIR)/q931 -I$(H323DIR)/per -I$(H323DIR)/config -I$(H323DIR)/pdlproc -I$(H323DIR)/cat -I$(H323DIR)/asn -I$(H323DIR)/newras -I$(H323DIR)/rasdb -I$(H323DIR)/tunneling # for 3.0.9
H323INCLUDES = -I$(H323DIR)/h323/include # for 4.0

#INCDIRS = $(DBMALLOCINC) -I. -I.. $(SFIOINCLUDE) -I$(DRINC) -I$(DRINC)/crypto 
#-L$(DLIB) -I/usr/local/include -L/usr/local/lib 
INCDIRS = $(DBMALLOCINC) -I. -I.. -I$(DRINC) -I$(DRINC)/crypto \
-L$(DLIB) -L/usr/local/lib \
-I$(DXML)/xmltok -I$(DXML)/xmlparse -I$(DXML)/xmlwf -I$(DSLOCK) \
-I$(OSDDIR)/include \
-I$(GKDIR)/include \
-I$(GISDIR)/include	\
-I$(SCMRPCDIR)/include \
-I$(SCMDIR)/include \
-I$(ISPDDIR)/include	\
-I$(SIPDIR)/include \
-I$(ENUMDIR)/include \
-I$(IWFDIR)/include \
$(SIPINCLUDES) \
$(H323INCLUDES) \
-I$(AVLDIR)/ \
-I$(TAVLDIR)/ \
-I$(FCEDIR)/include\
-I$(TSTDIR)/ \
-I$(DBREPDIR)/include \
-I$(BRIDGEDIR)\
-I$(RADCDIR)/include \
-I$(TSMDIR)/include

CFLAGS += $(INCDIRS) -DALLOW_ISERVER_SIP -DSIP_BY_REFERENCE -DANSI_PROTO -D_REENTRANT -DNO_STDLIB -DNO_STDARG -DSIP_THREAD_SAFE -D_POSIX_PTHREAD_SEMANTICS -DUNIX $(H323APPCFLAGS) -DSIP_SESSIONTIMER -DSIP_DCS -DSIP_PRIVACY -DSIP_IMPP $(PROFFLAGS)

# fce stuff for ipv6 dep
CFLAGS += -DSOLARIS_REL=$(SOLARIS_REL) -DNATIVE_ADDR_BITS=$(NATIVE_ADDR_BITS)

# flags for warnings
CFLAGS += -Wall -Wno-unused -Wno-parentheses -Wno-missing-braces 

#CFLAGS += -Wformat=2 -Wimplicit -Wmissing-braces -Wparentheses -Wsequence-point -Wreturn-type -Wswitch -Wunused-function -Wuninitialized -Wdiv-by-zero -W -Wfloat-equal -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wsign-compare -Wconversion -Waggregate-return -Werror

#CFLAGS += -Wswitch -Wuninitialized -Werror

LDFLAGS +=-L$(DLIB) -L/usr/local/BerkeleyDB.4.1/lib

#libraries
LIBDB		= $(DLIB)/libdb.a
LIBMEM		= $(DLIB)/libshm.a
LIBLOCK		= $(DLIB)/liblock.a
LIBSRVR		= $(DLIB)/libsrvr.a
LIBUTILS	= $(DLIB)/libutils.a
LIBOSD          = $(DLIB)/libosd.a
LIBCOMMON	= $(DLIB)/libcommon.a
LIBCOMMONSO	= $(DLIB)/libcommon.so
LIBNIKE		= $(DLIB)/libnike.a
LIBCRYPTO	= $(DLIB)/libcrypto.a
LIBXML		= $(DLIB)/libxml.a
LIBSCONFIG	= $(DLIB)/libsconfig.a
LIBAVL		= $(DLIB)/libavl.a
LIBTAVL		= $(DLIB)/libtavl.a
LIBCLISO	= $(DLIB)/libcli.so
LIBSLOCKS	= $(DLIB)/libslocks.a
LIBH323		= $(DLIB)/librvh323.a
LIBRTP		= $(DLIB)/librtp.a
LIBGK		= $(DLIB)/libgk.a
LIBAGEGK	= $(DLIB)/libagegk.a
LIBSIPCORE	= $(DLIB)/libsipcore.a
LIBSIPAPI	= $(DLIB)/libsipapi.a
LIBSIP		= $(DLIB)/libsip.a
LIBSIP2		= $(DLIB)/libsip2.a
LIBSIPTSM	= $(DLIB)/libsiptsm.a
LIBSIPUA	= $(DLIB)/libsipua.a
LIBSIPUA2	= $(DLIB)/libsipua2.a
LIBCLI		= $(DLIB)/libcli.a
LIBENUM		= $(DLIB)/libENUM.a
LIBIWF		= $(DLIB)/libiwf.a
LIBBRIDGE	= $(DLIB)/libbridge.a
LIBFCE		= $(DLIB)/libfce.a
#LIBFCECONFIG	= $(DLIB)/libfceconfig.a
#LIBARAVOX	= $(DLIB)/libvnsp.a
LIBTST		= $(DLIB)/libtst.a
LIBRSD		= $(DLIB)/librsd.a
LIBGISRPC	= $(DLIB)/libgisrpc.a
LIBSCMRPC	= $(DLIB)/libscmrpc.a
LIBSCM		= $(DLIB)/libscm.a
LIBSNMP 	= $(DLIB)/libsnmp.a
LIBMIB 		= $(DLIB)/libmib.a
LIBRADIUS	= $(DLIB)/libradius.a
LIBRADC		= $(DLIB)/libradc.a
LIBEXECD	= $(DLIB)/libexecd.a
LIBQUEDB        = $(DLIB)/libquedb.a


SERVERLIBS	= $(LIBDB) $(LIBMEM) $(LIBSRVR) $(LIBUTILS)
SIPLIBS		= $(LIBSIPTSM) $(LIBSIPUA) $(LIBBRIDGE) $(LIBSIP) $(LIBSIPAPI) $(LIBSIPCORE)

#LDLIBS		+=  -lresolv -lrt $(SFIOLIBS) $(DBMALLOCLIB)
ifeq ("X$(PLATFORM)", "Xlinux")
LDLIBS		+=   -lresolv -lrt -ldl $(DMALLOCLIB)
else
LDLIBS		+=   -lgen -lresolv -lrt $(DBMALLOCLIB) -ldl -lkvm
endif

#Defines which database library to use
DDB = QUEDB

ifeq ($(DDB), QUEDB)
LDLIBS += $(LIBQUEDB)
else
LDLIBS += /usr/local/BerkeleyDB.4.1/lib/libdb.a
endif

#configuration files
GKCONF		= $(BINDIR)/h323cfg-gktemplate.val
GKAGECONF	= $(BINDIR)/h323cfg-age.val
SERVERCONF	= $(BINDIR)/server.cfg

#binaries
BINGIS			= $(BINDIR)/gis
BINGISAGE		= $(BINDIR)/gisage
BINBCS			= $(BINDIR)/bcs
BINCLI			= $(BINDIR)/cli
BINISPD			= $(BINDIR)/ispd
BINRSD			= $(BINDIR)/rsd
BINHISTDB		= $(BINDIR)/histdb


VPATH = ..
