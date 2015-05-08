###########################################################################
###		Object Make include file				###
###   									###
###  object.mk								###
###									###
###########################################################################

###################################################
### Imported variables:				###
###						###
### APPTARGET =					###
### LIBTARGET =					###
### OBJECTS = 					###
### INCLUDEPATH = 				###
### PROGFLAGS = 				###
### PROGLIBS =					###
### DISTFILES =					###
### LIBPROGS = 					###
### PROGDEFS = 					###
### PUREPATH = 					###
### PRE = 					###
###################################################

#########################################################################################
#											#
# PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))	#
# include $(PROJECT_BASE)/make/object.mk						#
#											#
#########################################################################################

include $(PROJECT_BASE)/../../make/unix/common.mk



LIBS = 
DEFS   +=  $(PROGDEFS)
INCPATH = $(foreach file, $(INCLUDEPATH), -I$(file))
CFLAGS +=  $(PROGFLAGS) $(INCPATH)

APPT = $(foreach file, $(APPTARGET), $(bindir)/$(file))
LIBT = $(foreach file, $(LIBTARGET), $(libdir)/lib$(file).a)
all: $(LIBT) $(APPT)

#########################################################################
# Purify rules
#########################################################################
ifneq ($(pure), )
  CC      = gcc
  CPP     = gcc
  LD      = $(CPP)
  PURIFY  =  purify -show-directory=yes -user-path=$(PUREPATH)
else
  PURIFY  =  
endif



#########################################################################
# Platform dependent rules
#########################################################################
ifeq ($(os), Solaris)
  PROGLIBS += -lsocket -lnsl -ldl
endif

ifeq ($(os), Solaris8)
  PROGLIBS += -lsocket -lnsl -ldl
endif

ifeq ($(os), SolarisPC)
  PROGLIBS += -lsocket -lnsl -ldl
endif

ifeq ($(os), SolarisPC8)
  PROGLIBS += -lsocket -lnsl -ldl
endif

ifeq ($(os), UnixWare)
  PROGLIBS += -lsocket -lnsl -ldl -lcrypt -lgen
endif



#########################################################################
# Processing rules
#########################################################################

# The linking rule:
link_command = $(PURIFY) $(LD) $(LDFLAGS) -o $@

.SUFFIXES=
.SUFFIXES= $(objdir)/%.o .c .o .C .cpp

.PHONY: clean rm rmall rmt check clean depend lint mostlyclean distclean realclean dist version

OBJ_OUT = $(foreach file, $(OBJECTS), $(objdir)/$(file))
SRC_OUT = $(OBJECTS:.o=.c)
LIB_OUT = $(foreach file, $(LIBLIST), $(libdir)/lib$(file).a)
INC = -I$(srcdir) -I. -I.. -I$(incdir)



#########################################################################
# Dependencies implicit rules
#########################################################################

ifeq ($(nodepend), )

# dependencies
TMP = $(filter %.o, $(OBJ_OUT))
DEP_OUT = $(TMP:.o=.d)
-include $(DEP_OUT)

$(objdir)/%.d: %.c $(objdir)/nul
	$(SHELL) -ec "$(CC) -M $(INC) $(DEFS) $(CFLAGS) $(CPPFLAGS) $< | sed 's/$*\.o/$(subst /,\/,$(objdir)/$*.o) $(subst /,\/,$(objdir)/$*.d)/g' > $@"

$(objdir)/%.d: %.C
	$(SHELL) -ec "$(CC) -M $(INC) $(DEFS) $(CFLAGS) $(CPPFLAGS) $< | sed 's/$*\.o/$(subst /,\/,$(objdir)/$*.o) $(subst /,\/,$(objdir)/$*.d)/g' > $@"

$(objdir)/%.d: %.cpp
	$(SHELL) -ec "$(CC) -M $(INC) $(DEFS) $(CFLAGS) $(CPPFLAGS) $< | sed 's/$*\.o/$(subst /,\/,$(objdir)/$*.o) $(subst	/,\/,$(objdir)/$*.d)/g' > $@"


endif


#########################################################################
# Object implicit rules
#########################################################################

$(objdir)/%.o: %.c $(objdir)/nul
	$(CC) $(INC) $(CFLAGS) -c $< $(CPPFLAGS) $(DEFS) -o $@

$(objdir)/%.o: %.cpp
	$(CPP) $(INC) $(CFLAGS) -c $< $(CPPFLAGS) $(DEFS) -o $@

$(objdir)/%.o: %.C
	$(CPP) $(INC) $(CFLAGS) -c $< $(CPPFLAGS) $(DEFS) -o $@



#########################################################################
# Target processing rules
#########################################################################

$(APPT): $(OBJ_OUT) $(LIB_OUT) $(DEP_OUT) $(bindir)/nul
	$(link_command) $(OBJ_OUT) -L$(libdir) $(PROGLIBS)


$(LIBT): $(libdir)/nul $(OBJ_OUT) $(DEP_OUT)
	$(RM) -f $@
	$(AR) $(ARFLAGS) $@ $(OBJ_OUT)
	-$(RANLIB) $@





subdir = src



#########################################################################
# Automatic generation of object directories.
#########################################################################

$(objdir)/nul:
	if ( test ! -f $(objdir)/nul) ; then \
		mkdir -p $(objdir); \
		echo "" > $(objdir)/nul; \
	fi


$(libdir)/nul:
	if ( test ! -f $(libdir)/nul) ; then \
		mkdir -p $(libdir); \
		echo "" > $(libdir)/nul; \
	fi

$(bindir)/nul:
	if ( test ! -f $(bindir)/nul) ; then \
		mkdir -p $(bindir); \
		echo "" > $(bindir)/nul; \
	fi


#########################################################################
# General target rules.
#########################################################################

check:

lint:
	$(LINT) $(LINT_FLAGS) $(DEFS) $(INC) $(CFLAGS) $(SRC_OUT)

clean:
rm:
	$(RM) -f $(APPT) $(LIBT) *.o core $(OBJ_OUT) $(DEP_OUT)

rmt:
	$(RM) -f $(APPT) $(LIBT) core

mostlyclean: clean

depend:
	makedepend *.c *.C *.cpp $(INC) $(PROGFLAGS)


distclean: clean
	$(RM) -f Makefile

realclean: distclean
	$(RM) -f TAGS

distdir = ../`cat ../distname`/$(subdir)
dist: $(DISTFILES)
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) \
	    || { echo copying $$file instead; cp -p $$file $(distdir);}; \
	done

version:
	@echo Version:$(VERSION) By $(AUTHOR)


# Tell versions [3.59,3.63) of GNU make not to export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
