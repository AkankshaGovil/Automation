##############################################################
####        testapp  Makefile
##############################################################

PROJECT_NAME = Appl

PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk


ifeq ($(OS), )
  OS := $(shell uname)
  VER := $(shell uname -r)

  ifeq ($(OS), SunOS)
    ifeq ($(word 1, $(subst ., , $(VER))), 5)
        OS := Solaris
        ifeq ($(VER), 5.8)
            OS := Solaris8
        endif
        ifeq ($(word 1, $(subst ., , $(shell uname -m))), i86pc)
            OS := SolarisPC
            ifeq ($(VER), 5.8)
                OS := SolarisPC8
            endif
        endif
    endif
  endif
endif

XLIBS     = -lXaw -Xlinker -Bstatic -lXmu -Xlinker -Bdynamic -lXt -lXext -lX11 -lm

ifeq ($(OS), IRIX)
    XLIBS = -lXaw -lXmu  -lXt -lXext -lX11 -lm
endif

ifeq ($(OS), OSF1)
    XLIBS = -lXaw -lXmu  -lXt -lXext -lX11 -lm
endif

ifeq ($(OS), SunOS)
    XLIBS     =  -Xlinker -Bstatic -lXaw -lXmu -lXt -lXext -lX11 -Xlinker -Bdynamic -lm
endif

ifeq ($(OS), AIX)
    XLIBS     =  -lXaw -lXmu -lXt -lXext -lX11 -lm
endif

ifeq ($(OS), Linux)
    XLIBS     =  -lXaw3d -lXmu -lXt -lXext -lX11 -lm
endif

ifeq ($(OS), UnixWare)
    XLIBS =  -lXaw  -lXmu  -lXt -lXext -lX11  -lSM -lICE -lm
endif

ifeq ($(OS), HP-UX)
    XLIBS = -lX11 -L/usr/local/bin
endif


XINCLUDES = \
	$(ISYSTEM)/usr/openwin/include \
	$(ISYSTEM)/usr/include/X11R5 \
	$(ISYSTEM)/usr/local/X11R5/include \
	$(ISYSTEM)/usr/X11/include \
	$(ISYSTEM)/usr/X/include




OBJECTS   = \
    TAP_call.o \
    TAP_channel.o \
    TAP_general.o \
    TAP_h450.o \
    TAP_init.o \
    TAP_main.o \
    TAP_ras.o \
    TAP_rtp.o \
    TAP_scripts.o \
    TAP_security.o \
    TAP_snmp.o \
    TAP_tools.o \
    TAP_utils.o \
    TAP_wrapper.o


# Check if we have a single H.323 library file or a bunch of them
MAINLIBEXIST = $(wildcard $(libdir)/librvh323.a)
ifeq ($(MAINLIBEXIST), $(libdir)/librvh323.a)
	H323LIBS = -lrvh323
else
	H323LIBS = \
		-lcat -lcm -lcoder -lconf -lh245 -llog -llow -lmib -lpvtpst -lras \
		-ltrans -lutils -lasn -lads -lcat
endif


TARGET    = testapp
PROGLIBS  = \
    -L/usr/X11R6/lib  -L/usr/X11R6 -L/usr/X/lib -L/usr/X11/lib -L/usr/local/X11R5/lib \
    $(XLIBS) $(H323LIBS) \
    -ltcl8.3 -ltk8.3 -lm
LIBLIST   =
PROGFLAGS = $(XINCLUDES) -I../include
PROGDEFS  =

ifneq ($(rtp), )
  CFLAGS += -DRV_USE_RTP -DUSE_RTP
  PROGLIBS += -lrtp $(H323LIBS)
  LIBLIST += rtp
  PROGFLAGS += -I../../rtp/rtp
endif

ifneq ($(h450), )
  CFLAGS += -DUSE_H450
  PROGLIBS += -lsups -lsse $(H323LIBS)
  LIBLIST += sups sse
endif

ifneq ($(h235), )
  CFLAGS += -DUSE_SECURITY
  PROGLIBS += -lh235
  LIBLIST += h235
  ifeq ($(shell echo sec*.c),sec_hash.c)
    OBJECTS += sec_hash.o
    CFLAGS += -DUSE_SEC_HASH
  endif
endif

APPTARGET = $(bindir)/$(TARGET)
all: $(APPTARGET)
include $(PROJECT_BASE)/../make/unix/app.mk
