############################################
# 
# module.mk
#
# Ron S. 9 June 1997
#
# Compile subdirectories modules
#
############################################



srcdir = .

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
MAKE = gmake rough=on compiler=on

ifneq ($(OS), HP-UX)
 MAKE = make rough=on compiler=on
endif

ifeq ($(OS), Linux)
 MAKE = make rough=on compiler=on
endif

#ifneq ($(shrink), )
# MAKE_SUPP = $(MAKE) supp=on
#else
# MAKE_SUPP = $(MAKE)
#endif

SHELL = /bin/sh

include $(PROJECT_BASE)/make/common.mk
mall: all
include $(PROJECT_BASE)/make/largelib.mk


LIBSSUBDIRS = $(SUPP_LIBSUBDIRS)

ifneq ($(noapp),  )
	APPSSUBDIRS = 	
else
	APPSSUBDIRS = $(SUPP_APPSUBDIRS)
endif

.SUFFIXES:

ALL = 
ifneq ($(words $(LIBSSUBDIRS)), 0)
	ALL+= libs
endif
ifneq ($(words $(APPSSUBDIRS)), 0)
	ALL+= apps
endif


all: $(ALL)


libs: 
	echo $(x)
	for subdir in $(LIBSSUBDIRS); do \
	  echo making $@ in $$subdir; \
	  ($(MAKE) -C $$subdir) || exit 1; \
	done
	$(MAKE) -C $(SP_LIBSUBDIRS) -f $(SPMAKE)

apps: 
	for subdir in $(APPSSUBDIRS); do \
	  echo making $@ in $$subdir; \
	 (cd $$subdir && $(MAKE) ) || exit 1; \
	done


rm: 
	for subdir in $(LIBSSUBDIRS) $(APPSSUBDIRS); do \
	  echo making rm in $$subdir; \
	  (cd $$subdir && $(MAKE) rm) || exit 1; \
	done
	$(MAKE) -f $(SPMAKE) -C $(SP_LIBSUBDIRS) rm
rmt: 
	for subdir in $(LIBSSUBDIRS) $(APPSSUBDIRS); do \
	  echo making rm in $$subdir; \
	  (cd $$subdir && $(MAKE) rmt) || exit 1; \
	done
	$(MAKE) -f $(SPMAKE) -C $(SP_LIBSUBDIRS) rmt
rmall:
	$(RM) -f $(objdir)/*.o
	$(RM) -f $(objdir)/*.d
	$(RM) -f $(libdir)/*.a
	$(RM) -f $(bindir)/*


check:
installcheck:

$(PROGS):
	cd application && $(MAKE) $@

app:
	cd application && rm -f testApp
	cd application && rm -f dfTest
	cd application && $(MAKE) testApp

mostlyclean: mostlyclean-recursive mostlyclean-local

clean: clean-recursive clean-local

distclean: distclean-recursive distclean-local
	rm config.status

realclean: realclean-recursive realclean-local
	rm config.status

TAGS clean-recursive distclean-recursive \
	    mostlyclean-recursive realclean-recursive:
	for subdir in $(SUBDIRS); do \
	  target=`echo $@|sed 's/-recursive//'`; \
	  echo making $$target in $$subdir; \
	  (cd $$subdir && $(MAKE) $$target) || exit 1; \
	done

mostlyclean-local:

clean-local: mostlyclean-local

distclean-local: clean-local
	rm -f Makefile config.cache config.h config.log stamp-h distname

realclean-local: distclean-local

distname: src/version.c
	echo fileutils-`sed -e '/version_string/!d' \
	    -e 's/[^0-9.]*\([0-9.a-z]*\).*/\1/' -e q src/version.c` > $@-tmp
	mv $@-tmp $@

distdir = `cat distname`
dist: $(DISTFILES) distname
	rm -rf $(distdir)
	mkdir $(distdir)
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) \
	    || { echo copying $$file instead; cp -p $$file $(distdir);}; \
	done
	for subdir in $(SUBDIRS); do \
	  mkdir $(distdir)/$$subdir || exit 1; \
	  (cd $$subdir && $(MAKE) $@) || exit 1; \
	done
	tar --gzip -chvf $(distdir).tar.gz $(distdir)
	rm -rf $(distdir) distname

# For the justification of the following Makefile rules, see node
# `Automatic Remaking' in GNU Autoconf documentation.

#Makefile: config.status Makefile.in
#	CONFIG_FILES=$@ CONFIG_HEADERS= ./config.status

config.h: stamp-h
stamp-h: config.status $(srcdir)/config.h.in
	CONFIG_FILES= CONFIG_HEADERS=config.h ./config.status

# Use echo instead of date to avoid spurious conflicts for
# people who use CVS, since stamp-h.in is distributed.
	echo > $(srcdir)/$@

version:
	@echo Version:$(VERSION) By $(AUTHOR)

# Tell versions [3.59,3.63) of GNU make not to export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
