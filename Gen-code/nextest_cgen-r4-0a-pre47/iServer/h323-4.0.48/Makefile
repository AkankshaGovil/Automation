##############################################################
####        Makefile
##############################################################
include $(PROJECT_BASE)/../make/unix/common.mk

DLIB	 = $(BASE)/lib/$(TARGET)
LIBH323	 = $(DLIB)/librvh323.a

os       = $(OS)
root_dir = ${PROJECT_BASE}/..
bindir   = ${root_dir}/binaries/${os}/${compdir}/bin
libdir   = ${root_dir}/binaries/${os}/${compdir}/lib
objdir   = ${root_dir}/binaries/${os}/${compdir}/obj

lib install:
ifeq ($(os), Linux)
	(cd ../h323; make devepoll=on mts=on noapp=on PROJECT_NAME=$(H323VER)/h323)
else
	(cd ../h323; make devpoll=on mts=on noapp=on PROJECT_NAME=$(H323VER)/h323)
endif
	-(cd ../h323; make rvh323)
	-(cp ${libdir}/librvh323.a $(LIBH323))
	
clean:
	rm -f ${libdir}/*
	rm -f ${objdir}/*
