###########################################################################
###		Large Library Make include file				###
###   									###
###  largelib.mk							###
###									###
###########################################################################

###################################################
### Imported variables:				###
###						###
### LARGELIB =					###
### LIBGROUP = 					###
###################################################

#################################################
# before including this file:			#
#						#
#include $(BASE)/common.mk			#
#LARGELIB = lib-name				#
#LIBGROUP = list of lib names			# 
#all:  $(LARGELIB)				#
#################################################

LIB_OUT = $(foreach file, $(LIBGROUP), $(libdir)/lib$(file).a)

$(LARGELIB):
	cd $(libdir);
	$(RM) -f $(libdir)/lib$(LARGELIB).a *.o
	for file in $(LIB_OUT); do \
	  echo Extracting $$file...; \
	  $(AR) x $$file; \
	  $(RM) -f _____*; \
	done 
	$(AR) $(ARFLAGS) $(libdir)/lib$(LARGELIB).a *.o 
	-$(RANLIB) $(libdir)/lib$(LARGELIB).a
	$(RM) -f *.o
