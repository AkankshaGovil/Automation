#####################################################
#
# common.mk
#
# Generic and common variables for makefiles.
#
# Need to define environment variables: 
#    OS: The operating system.
#    PROJECT_BASE: The project root directory.
#
#####################################################

# 1.2 Automatic Dependencies
# 1.5 Automatic directory creation
# 1.6 Bug fixing.

VERSION = 1.6
AUTHOR = Ron S.

.PHONY: clean rm rmall rmt

###########################
# environment definitions
###########################


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


# Deal with warning switches for the GNU compiler
WARNING = -Wall -pedantic
ifneq ($(rough), )
	WARNING += -isystem /sw/tools/include-v2 -x c++ -fno-builtin -fsigned-char -ansi  -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes
endif


# See if we're compiling a debug mode application or a release mode application
ifeq ($(release), )
	compdir = debug
else
	compdir = release
endif


srcdir   = .
os       = $(OS)
root_dir = ${PROJECT_BASE}/..
bindir   = ${root_dir}/binaries/${os}/${compdir}/bin
libdir   = ${root_dir}/binaries/${os}/${compdir}/lib
objdir   = ${root_dir}/binaries/${os}/${compdir}/obj
incdir   = ${root_dir}/h323/include
mkdir    = ${root_dir}/make/unix


INSTALL = /bin/install -c
INSTALL_PROGRAM = ${INSTALL}
RM      = rm
AR      = ar
ARFLAGS = crv
RANLIB  = ranlib
SHELL   = /bin/sh

PROFILE = 



CPPFLAGS = -I/usr/local/include
DEFS    =
OPTIMI  = -O2

# System include directory option of the compiler
TMP_EMPTY =
ISYSTEM = -isystem $(TMP_EMPTY)

# Compiler supports preprocessing of dependencies list, which we use to create .d files
SUPPORT_DEP = on

# Compiler and linker programs
CC      = gcc
CPP     = gcc
LD      = $(CPP)

LINT    = lint



######################
## Switching between machines.
######################

ifeq ($(os), SunOS)
  CFLAGS += -DPLATFORM_BIG_ENDIAN -DIS_PLATFORM_SUNOS
endif

ifeq ($(os), Solaris)
  CFLAGS += -DPLATFORM_BIG_ENDIAN -DIS_PLATFORM_SOLARIS -DHOST_HAS_DYNAMIC_IP
endif

ifeq ($(os), Solaris8)
  CFLAGS += -DPLATFORM_BIG_ENDIAN -DIS_PLATFORM_SOLARIS -DHOST_HAS_DYNAMIC_IP
endif

ifeq ($(os), SolarisPC)
  CFLAGS += -DIS_PLATFORM_SOLARISPC
endif

ifeq ($(os), SolarisPC8)
  CFLAGS += -DIS_PLATFORM_SOLARISPC
endif




######################
## UnixWare
######################
ifeq ($(os), UnixWare)
  CFLAGS += -DUNIXWARE -DNOTHREADS -DRTP_NOLOCKS
  RANLIB = echo
  nothreads = on
  
  # UnixWare doesn't need ranlib on the resulting object files
endif


######################
## HP
######################
HPUX_CFLAGS   = -r -Aa -D_HPUX_SOURCE -D_POSIX_SOURCE -DXOPEN_CATALOG +e
LINT_FLAGS    = -Aa -s

ifeq ($(os), HP-UX)
	CFLAGS += -DPLATFORM_BIG_ENDIAN -DIS_PLATFORM_HP -DIS_PLATFORM_HPUX
	RM = rm
	CC = cc
	CPP = cc
	LD = $(CPP)
	CFLAGS += $(HPUX_CFLAGS)
	WARNING = -Wall
	ISYSTEM = -I
	OPTIMI = +O2
	SUPPORT_DEP = off
endif


######################
## OFS1 - Alpha
######################
ifeq ($(os), OSF1)
	CFLAGS += -D_ALPHA_ -DHAS_STDARG
	OPTIMI += -fast
	CC = cc
	CPP = cc
	LD = $(CPP)
	ISYSTEM = -I
WARNING =
endif


######################
## Linux - RedHat
######################
ifeq ($(os), Linux)
    CFLAGS += -D_SVID_SOURCE -D__REDHAT__
	DEFS   += -D__LINUX__ -D__USE_BSD -I/usr/include/ncurses -D__REDHAT__ 
    WARNING += -isystem /sw/tools/include-v2 -fno-builtin -fsigned-char -ansi  -Wall -Wwrite-strings -Wstrict-prototypes
	RM = rm

    ifeq ($(release), )
        # This parameter should not be included for release mode due to a bug
        # in GNU library when using it with -O2
        WARNING += -Wpointer-arith
    endif
endif


######################
## VxWorks
######################
VW         = /home/vw/
VWH        = $(VW)h/
H1         = $(VWH)
H2         = $(VWH)net/
H3         = $(VWH)rpc/
H6         = $(VWH)drv/
H7         = $(VWH)
H8         = $(VWH)netinet/
H9         = $(VW)src/config/
GLIB       = $(VW)lib/
G960BASE   = $(VW)gnu960/sun4/
GBIN       = $(G960BASE)bin/

GINCLUDES  = -I. -I$(H1) -I$(H2) -I$(H6) -I$(H8) -I$(H9)
GFORMAT    = -Fcoff
GDEFINES   = -DCPU=I960CA -DHOST_SUN -DVX_IGNORE_GNU_LIBS
GCFLAGS    = -ansi $(GDEFINES) $(GINCLUDES) $(GFORMAT)


ifeq ($(os), VxWorks)
  OPTIMI = -O4
  CC     = $(GBIN)gcc960 -ACA
  LD     = $(GBIN)/gld960 -ACA $(GFORMAT) -r
  AR     = $(GBIN)gar960
  ARFLAGS= vrus
  RANLIB = echo
  CFLAGS += $(GCFLAGS) -DRMONVER_1 -DRMONVER_SA
  DEFS   = -D__VXWORKS__ -DNOTHREADS
endif


#################################################################################
#              new GNU compiler (2.7)                                           #
#################################################################################
ifeq ($(os), Tornado)
  GW20_VER   = GW20_VER2
  PHASE	     = phase_9
  VW	     = /el-nino/tornado/gw20/$(PHASE)/
  VWH        = $(VW)h/
  H1         = $(VWH)
  H2         = $(VWH)net/
  H3         = $(VWH)rpc/
  H6         = $(VWH)drv/
  H7         = $(VWH)
  H8         = $(VWH)netinet/
  H9         = $(VW)src/config/
  GLIB       = $(VW)lib/
  G960BASE   = /el-nino/tornado/host/sun4-solaris2/
  GBIN       = $(G960BASE)bin/

  TGT_DIR    = $(WIND_BASE)/$(PHASE)
  CONFIG_ALL = $(TGT_DIR)/config/all
#  GCC_EXEC_PREFIX = $(WIND_BASE)/host/$(WIND_HOST_TYPE)/bin/ 
#  GCC_EXEC_PREFIX = /vobs/BSP_I960_531/host/sun4-solaris2/lib/gcc-lib/

  CC_INCLUDE = -I$(UP)/h $(INCLUDE_CC) $(EXTRA_INCLUDE) \
		  -I. -I$(CONFIG_ALL) -I$(TGT_DIR)/h -I$(TGT_DIR)/src/config \
		  -I$(TGT_DIR)/src/drv
  CC_DEFINES = -DCPU=I960JX  -DVX_IGNORE_GNU_LIBS $(EXTRA_DEFINE)

  INCLUDES   = -I. -I$(H1) -I$(H2) -I$(H6) -I$(H8) -I$(H9)
  DEFINES    = -DCPU_VAR=I960HX  -DVX_IGNORE_GNU_LIBS

  OPTIMI    = -O4
  CC        = $(GBIN)cc960 -B$(GCC_EXEC_PREFIX)
  LD        = $(GBIN)ld960 -AJX -X -N -r 
  AR        = $(GBIN)ar960 
  ARFLAGS   = vrus
  RANLIB    = $(GBIN)ranlib960
  CFLAGS    += -ansi -mca -mstrict-align $(DEFINES) $(INCLUDES) -D$(GW20_VER) \
		-ansi -nostdinc -Wall \
		$(CC_INCLUDE) $(CC_DEFINES) $(ADDED_CFLAGS)  \
		$(CC_SOFT_FLOAT)
  DEFS   = -D__VXWORKS__ -DNOTHREADS -DRTP_NOLOCKS
  rough := on
endif



## Compilation related parameters
## These affect the code being compiled
#######################################


ifneq ($(nolog), )
	# No logs in resulting code
	CFLAGS += -DNOLOGSUPPORT
endif

ifneq ($(release), )
	# Release version - optimized
	CFLAGS  += $(OPTIMI) $(WARNING)
else
	# Debug version
	CFLAGS  += -g $(WARNING) $(PROFILE) -D_DEBUG
endif

ifneq ($(nothreads), )
	# No threads support
 	CFLAGS += -DNOTHREADS
endif

ifneq ($(nolocks), )
	# No locking mechanism in RTP
	CFLAGS += -DRTP_NOLOCKS
endif

ifneq ($(poll), )
	# Use poll() instead of select()
	CFLAGS += -DUSE_POLL
endif

ifneq ($(devpoll), )
	# Use /dev/poll instead of select()
	CFLAGS += -DSELI_USE_DEVPOLL
endif

ifeq ($(os), Linux)
	ifneq ($(devepoll), )
		# Use /dev/epoll instead of select()
		CFLAGS += -DSELI_USE_DEVEPOLL
	endif
endif

ifneq ($(mts), )
  ifneq ($(nothreads), on)
    CFLAGS += -D_MTS_ -D_REENTRANT
  endif
endif

ifneq ($(examine), )
  CFLAGS += -D_EXAMINE_
endif

