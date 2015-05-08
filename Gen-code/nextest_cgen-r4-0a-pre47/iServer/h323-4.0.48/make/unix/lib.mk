###########################################################################
###		Library Make include file				###
###   									###
###  lib.mk								###
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
#LIBTARGET := $(libdir)/lib$(TARGET).a		#
#all: $(PRE) $(LIBTARGET)			#
#################################################



LIBS = 
#LDFLAGS = 
LDFLAGS = -g
DEFS   +=  $(PROGDEFS)
CFLAGS +=  $(PROGFLAGS)

.SUFFIXES=$(objdir)/%.o %.c %.o %.C %.cpp %.d

OBJ_OUT = $(foreach file, $(OBJECTS), $(objdir)/$(file))
SRC_OUT = $(OBJECTS:.o=.c)


INC = -I. -I.. -I$(incdir) -I$(srcdir)


ifeq ($(SUPPORT_DEP), on)

# Manage dependencies files

# dependencies
DEP_OUT = $(OBJ_OUT:.o=.d)
-include $(DEP_OUT)

$(objdir)/%.d: %.c $(objdir)/nul
	$(SHELL) -ec "$(CC) -M $(INC) $(DEFS) $(CFLAGS) $(CPPFLAGS) $< | sed 's/$*\.o/$(subst /,\/,$(objdir)/$*.o) $(subst /,\/,$(objdir)/$*.d)/g' > $@"

$(objdir)/%.d: %.C
	$(SHELL) -ec "$(CC) -M $(INC) $(DEFS) $(CFLAGS) $(CPPFLAGS) $< | sed 's/$*\.o/$(subst /,\/,$(objdir)/$*.o) $(subst /,\/,$(objdir)/$*.d)/g' > $@"

$(objdir)/%.d: %.cpp
	$(SHELL) -ec "$(CC) -M $(INC) $(DEFS) $(CFLAGS) $(CPPFLAGS) $< | sed 's/$*\.o/$(subst /,\/,$(objdir)/$*.o) $(subst /,\/,$(objdir)/$*.d)/g' > $@"

endif


$(objdir)/%.o: %.c $(objdir)/nul
	$(CC) $(INC) $(CFLAGS) -c $< $(CPPFLAGS) $(DEFS) -o $@

$(objdir)/%.o: %.C
	$(CPP) $(INC) $(CFLAGS) -c $< $(CPPFLAGS) $(DEFS) -o $@

$(objdir)/%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< $(CPPFLAGS) $(DEFS) $(INC) -o $@



$(LIBTARGET): $(libdir)/nul $(OBJ_OUT) $(DEP_OUT)
	$(RM) -f $@
	$(AR) $(ARFLAGS) $@ $(OBJ_OUT)
	-$(RANLIB) $@

lint:
	$(LINT) $(LINT_FLAGS) $(INC) $(DEFS) $(CFLAGS) $(SRC_OUT)

subdir = src
check:


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

clean:
rm:
	$(RM) -f $(LIBTARGET) *.o core $(OBJ_OUT) $(DEP_OUT)

rmt:
	$(RM) -f $(LIBTARGET) core

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

