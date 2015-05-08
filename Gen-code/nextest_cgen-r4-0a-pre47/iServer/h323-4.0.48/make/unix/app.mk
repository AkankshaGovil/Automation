###########################################################################
###		Application Make include file				###
###   									###
###  app.mk								###
###									###
###########################################################################

###################################################
### Imported variables:				###
###						###
### TARGET =					###
### OBJECTS = 					###
### PROGFLAGS = 				###
### PROGLIBS =					###
### DISTFILES =					###
### LIBPROGS = 					###
### PROGDEFS = 					###
### PRE = 					###
###################################################

#################################################
# before including this file:			#
#						#
#include ../common.mk				#
#APPTARGET := $(bindir)/$(TARGET)		#
#all: $(APPTARGET)				#
#################################################

LIBS = 

#ifneq ($(release), )
#   LDFLAGS = -g
#else
#   LDFLAGS = -g $(PROFILE)
#endif


DEFS   +=  $(PROGDEFS)
CFLAGS +=  $(PROGFLAGS)


ifneq ($(pure), )
  CC      = gcc
  CPP     = gcc
  LD      = $(CPP)
else
  PURIFY  =  
endif



ifeq ($(os), Solaris)
  PROGLIBS += -lsocket -lnsl -ldl
  ifeq ($(nothreads), )
    PROGLIBS += -lposix4 
  endif
endif

ifeq ($(os), Solaris8)
  PROGLIBS += -lsocket -lnsl -ldl
endif

ifeq ($(os), SolarisPC)
  PROGLIBS += -lsocket -lnsl -ldl
  ifeq ($(nothreads), )
    PROGLIBS += -lposix4 
  endif
endif

ifeq ($(os), SolarisPC8)
  PROGLIBS += -lsocket -lnsl -ldl
endif

ifeq ($(os), UnixWare)
  PROGLIBS += -lsocket -lnsl -ldl -lcrypt -lgen
endif


ifeq ($(nothreads), )
  PROGLIBS += -lpthread
endif


# Linking rules.
link_command = $(PURIFY) $(LD) $(LDFLAGS) -o $@

.SUFFIXES=
.SUFFIXES= $(objdir)/%.o .c .o .C .cpp

OBJ_OUT = $(foreach file, $(OBJECTS), $(objdir)/$(file))
SRC_OUT = $(OBJECTS:.o=.c)
LIB_OUT = $(foreach file, $(LIBLIST), $(libdir)/lib$(file).a)
INC = -I$(srcdir) -I. -I.. -I$(incdir)


ifeq ($(SUPPORT_DEP), on)

# Manage dependencies files

# dependencies
TMP = $(filter %.o, $(OBJ_OUT))
DEP_OUT = $(TMP:.o=.d)
-include $(DEP_OUT)

$(objdir)/%.d: %.c $(objdir)/nul
	$(SHELL) -ec "$(CC) -M $(INC) $(DEFS) $(CFLAGS) $(CPPFLAGS) $< | sed 's/$*\.o/$(subst /,\/,$(objdir)/$*.o) $(subst /,\/,$(objdir)/$*.d)/g' > $@"

$(objdir)/%.d: %.C
	$(SHELL) -ec "$(CC) -M $(INC) $(DEFS) $(CFLAGS) $(CPPFLAGS) $< | sed 's/$*\.o/$(subst /,\/,$(objdir)/$*.o) $(subst /,\/,$(objdir)/$*.d)/g' > $@"

$(objdir)/%.d: %.cpp
	$(SHELL) -ec "$(CC) -M $(INC) $(DEFS) $(CFLAGS) $(CPPFLAGS) $< | sed 's/$*\.o/$(subst /,\/,$(objdir)/$*.o) $(subs /,\/,$(objdir)/$*.d)/g' > $@"

endif



$(objdir)/%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< $(CPPFLAGS) $(DEFS) $(INC) -o $@

$(objdir)/%.o: %.C
	$(CPP) $(CFLAGS) -c $< $(CPPFLAGS) $(DEFS) $(INC) -o $@

$(objdir)/%.o: %.c $(objdir)/nul
	$(CC) $(CFLAGS) -c $< $(CPPFLAGS) $(DEFS) $(INC) -o $@

$(APPTARGET): $(OBJ_OUT) $(LIB_OUT) $(DEP_OUT) $(bindir)/nul
	$(link_command) $(OBJ_OUT) -L$(libdir) $(PROGLIBS)



subdir = src


check:

$(objdir)/nul:
	if ( test ! -f $(objdir)/nul) ; then \
		mkdir -p $(objdir); \
		echo "" > $(objdir)/nul; \
	fi


$(bindir)/nul:
	if ( test ! -f $(bindir)/nul) ; then \
		mkdir -p $(bindir); \
		echo "" > $(bindir)/nul; \
	fi


lint:
	$(LINT) $(LINT_FLAGS) $(DEFS) $(INC) $(CFLAGS) $(SRC_OUT)

clean:
rm:
	$(RM) -f $(APPTARGET) *.o core $(OBJ_OUT) $(DEP_OUT)
rmt:
	$(RM) -f $(APPTARGET) core

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

