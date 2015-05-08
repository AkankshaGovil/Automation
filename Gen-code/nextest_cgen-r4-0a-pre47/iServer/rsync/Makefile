CMDARG = $@
RSYNCDIR = $(BASE)/rsync
RSYNCPKGDIR = $(BASE)/utils/rsyncpkg
RSYNCBINDIR = $(RSYNCDIR)/bin/$(TARGET)
RSYNCETCDIR = $(RSYNCDIR)/etc
ifeq ("X$(PLATFORM)", "Xlinux")
MORE = /bin/more
GREP = /bin/grep
RM 	 = /bin/rm -f
SED  = /bin/sed
SH	 = /bin/sh
CP	 = /bin/cp -p
MKDIR = /bin/mkdir -p
FIND = /bin/find
CPIO = /bin/cpio
CVS = /usr/bin/cvs
else
MORE = /usr/bin/more
GREP = /usr/bin/grep
RM 	 = /usr/bin/rm -f
SED  = /usr/bin/sed
SH	 = /usr/bin/sh
CP	 = /usr/bin/cp -p
MKDIR = /usr/bin/mkdir -p
FIND = /usr/bin/find
CPIO = /usr/bin/cpio
CVS = /usr/local/bin/cvs
endif

configure : 
	( cd $(RSYNCDIR)/rsync; ./configure; )

install : all
	( if [ ! -d $(RSYNCBINDIR) ]; then $(MKDIR) $(RSYNCBINDIR); fi; )
	( cd $(RSYNCDIR)/rsync; make -s -n install > $(RSYNCBINDIR)/rsync-bin.sh )
	(chmod +x $(RSYNCBINDIR)/rsync-bin.sh )
	( cd $(RSYNCDIR)/rsync; $(MORE) $(RSYNCBINDIR)/rsync-bin.sh | $(GREP) install-sh | \
		$(SED) "s, [^ ]*$$, $(RSYNCBINDIR),g" | $(SH); )
	( cd $(RSYNCDIR)/rsync; $(MORE) $(RSYNCBINDIR)/rsync-bin.sh | \
		$(GREP) -v make > $(RSYNCBINDIR)/rsync-bin.sh.tmp; \
		mv -f $(RSYNCBINDIR)/rsync-bin.sh.tmp $(RSYNCBINDIR)/rsync-bin.sh)
	(chmod +x $(RSYNCBINDIR)/rsync-bin.sh )
	( cd $(RSYNCDIR)/rsync; $(CP) install-sh $(RSYNCBINDIR) )
	( if [ ! -d $(RSYNCPKGDIR) ]; then $(MKDIR) $(RSYNCPKGDIR); fi; )
	( OLDDIR=`pwd`; \
		{ cd $(RSYNCBINDIR); $(FIND) . -print | $(GREP) -v 'CVS' | $(CPIO) -o 2>/dev/null; } | \
		{ cd $(RSYNCPKGDIR); $(MKDIR) bin; cd bin; echo "Copying to $(RSYNCPKGDIR)/bin"; \
			$(CPIO) -iuv 2>/dev/null; cd $(OLDDIR); } )
	( OLDDIR=`pwd`; \
		{ cd $(RSYNCETCDIR); $(FIND) . -print | $(GREP) -v 'CVS' | $(CPIO) -o 2>/dev/null; } | \
		{ cd $(RSYNCPKGDIR); $(MKDIR) etc; cd etc; echo "Copying to $(RSYNCPKGDIR)/etc"; \
			$(CPIO) -iuv 2>/dev/null; cd $(OLDDIR); } )
ifneq ("X$(PLATFORM)", "Xlinux")
	( $(CP) $(RSYNCDIR)/rsync.act $(RSYNCPKGDIR)/../rsync.act )
endif


clean :
	( if [ -d $(RSYNCBINDIR) ]; then cd $(RSYNCBINDIR); $(RM) * >/dev/null 2>&1; fi;)
	( if [ -d $(RSYNCPKGDIR) ]; then cd $(RSYNCPKGDIR); $(RM) -r * >/dev/null 2>&1; fi;)
	( $(RM) $(RSYNCPKGDIR)/../rsync.act >/dev/null 2>&1;)
	( cd $(RSYNCDIR)/rsync; $(MAKE) $@; )

%::
	( cd $(RSYNCDIR)/rsync; if [ ! -f config.status ]; then ./configure; fi;)
	( cd $(RSYNCDIR)/rsync; $(MAKE) $@; )
