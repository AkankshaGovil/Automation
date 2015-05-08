
#################################################################
#################################################################
##
##                      Makefile
##
#################################################################
#################################################################

## Global top level Makefile.
## SR, 09/28/98.

## Include global rules.
include $(BASE)/global.mk

#Platform dep
GISDIRS = $(GKDIR) $(SIPDIR) tsm ua bridge iwf server common pd dbrep cli ls
SUBDIRS = $(SCMRPCDIR) $(ENUMDIR) $(GKDIR) $(SIPDIR) $(H323DIR) $(SIPSTACKDIR) tsm ua bridge iwf slocks tavl avl-1.4.0 tst1.3 server common xml crypto $(SCMDIR) sconfig $(FCEDIR) dbrep execd cli java quedb $(RADCDIR)
EXECDIRS = lm pm sconfig pd dbrep execd cli ls 
ADMINDIRS = utils plib
#Platform independent subdirs
USUBDIRS = scripts rsync

ALLDIRS = $(SUBDIRS) $(USUBDIRS) $(EXECDIRS) include utils install

default:
	@echo "Please indicate your target to make"


rtags:
	cvs rtag $(TAG) iServer

commits:
	-cvs commit -l
	for subdir in $(ALLDIRS); do \
		(cd $$subdir; cvs commit;) \
	done

relcheckout:
	cvs checkout -r $(TAG) iServer

updates:
	cvs update -l
	for subdir in $(ALLDIRS); do \
		(cd $$subdir; cvs update -R;) \
	done

# list all the locally modified files, starting from the top level
list:
	@(cvs status | awk -f list.awk | grep -v "Examining"; exit 0)

# the above + the files need to be patched
listall:
	@(cvs status | awk -f listall.awk | grep -v "Examining"; exit 0)

testbed:
	if (test -d /usr/local/nextone/bin); then \
		echo "please cleanup previous install"; \
	else \
		ln -s $(BINDIR) /usr/local/nextone/bin; \
		if (test ! -d /databases); then \
			mkdir /databases; \
			mkdir /locks; \
		fi; \
	fi

iserver aloid plat:
	@if (test ! -d $(BASE)/bin); then \
		mkdir bin; \
	fi; \
	if (test ! -d $(BASE)/lib); then \
		mkdir lib; \
	fi; \
	cd bin; $(BASE)/utils/links.sh $(TARGET) Makefile;	
	@cd lib; $(BASE)/utils/links.sh $(TARGET) Makefile;	
	#rsync does not build properly with profiling turned on.
	@if [ "$(BUILDTYPE)" != "profile-debug"  -a  "$(BUILDTYPE)" != "profile-standard" ]; then \
		for subdir in $(USUBDIRS); do \
			(cd $$subdir; $(MAKE) install; exit $$?; );\
			if [ $$? -ne 0 ]; then \
				echo "======================"; \
				echo "Error making $$subdir"; \
				echo "======================"; \
				exit 1; \
			fi \
		done \
	fi;
	@(cd osd; ./configure --libdir=$(DLIB); $(MAKE) install; exit $$?; );\
	if [ $$? -ne 0 ]; then \
		echo "======================"; \
		echo "Error making osd"; \
		echo "======================"; \
		exit 1; \
	fi
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir; $(BASE)/utils/links.sh $(TARGET) Makefile; \
		if [ "$$subdir" = "java" ]; then \
			$(BASE)/utils/links.sh $(TARGET) jserver.mk; \
			$(BASE)/utils/links.sh $(TARGET) iview.mk; \
			$(BASE)/utils/links.sh $(TARGET) ifac.mk; \
			$(BASE)/utils/links.sh $(TARGET) iportal.mk; \
			$(BASE)/utils/links.sh $(TARGET) pingtel.mk; \
			$(BASE)/utils/links.sh $(TARGET) jainsip.mk; \
		fi; \
		cd $$TARGET; $(MAKE) lib; exit $$?; ); \
		if [ $$? -ne 0 ]; then \
			echo "======================"; \
			echo "Error making lib in $$subdir"; \
			echo "======================"; \
			exit 1; \
		fi \
	done
	-@for subdir in $(EXECDIRS); do \
		(cd $$subdir; $(BASE)/utils/links.sh $(TARGET) Makefile; \
		cd $$TARGET; $(MAKE) exec; exit $$?; ); \
		if [ $$? -ne 0 ]; then \
			echo "======================"; \
			echo "Error making executable in $$subdir"; \
			echo "======================"; \
			exit 1; \
		fi \
	done

tags:
	etags gk/*.[ch] slocks/*.[ch] avl/*.[ch] server/*.[ch] common/*.[ch] xml/*.[ch] nike/*.[ch] crypto/*.[ch] sconfig/*.[ch] cli/*.[ch] ls/*.[ch] faxserver/*.[ch] java/*.[ch] lm/*.[ch] pm/*.[ch] include/*.h fce/*.[ch]

ctags:
	ctags -t $(BASE)/gk/*.[ch] $(BASE)/gk/include/*.h $(BASE)/slocks/*.[ch] $(BASE)/avl/*.[ch] $(BASE)/server/*.[ch] $(BASE)/common/*.[ch] $(BASE)/xml/*.[ch] $(BASE)/nike/*.[ch] $(BASE)/crypto/*.[ch] $(BASE)/sconfig/*.[ch] $(BASE)/cli/*.[ch] $(BASE)/ls/*.[ch]  $(BASE)/java/*.[ch] $(BASE)/lm/*.[ch] $(BASE)/pm/*.[ch] $(BASE)/include/*.h $(BASE)/fce/*.[ch] $(BASE)/tsm/*.[ch] $(BASE)/ssip/*.[ch] $(BASE)/ssip/include/*.h $(BASE)/ua/*.[ch] $(BASE)/bridge/*.[ch]

exctags:
	 $(EX_CTAG) -R --langmap=fortran: --exclude="CVS"

cleangis:
	for subdir in $(GISDIRS); do \
		(cd $$subdir; $(MAKE) clean;) \
	done
	for subdir in $(GISDIRS); do \
		(cd $$subdir; \
		$(MAKE) clean; \
		if (test -d $(TARGET)); then \
			(cd $(TARGET); $(MAKE) clean ); \
		fi ); \
	done
	rm -f bin/$(TARGET)/gis bin/$(TARGET)/gisage bin/$(TARGET)/cli

clean cleanplat:
	for subdir in $(USUBDIRS); do \
		(cd $$subdir; $(MAKE) clean;) \
	done
	for subdir in $(SUBDIRS) $(EXECDIRS); do \
		(cd $$subdir; \
		$(MAKE) clean; \
		if (test -d $(TARGET)); then \
			(cd $(TARGET); $(MAKE) clean ); \
		fi ); \
	done
	for subdir in $(ADMINDIRS); do \
		(cd $$subdir; $(MAKE) clean; ) \
	done
	rm -f bin/$(TARGET)/*
	rm -f lib/$(TARGET)/*

package:  libsip aloid dbtools
	-tar cvf $(TARGET).install.tar setup install/LICENSE install/*.sh install/*filelist install/adminstall.pl install/perlcheck.pl install/nextoneIserver
	cd install; ./package.sh


dbtools:
	@(cd utils; make install)
	@(cd plib; make install)

dbtoolclean:
	@(cd utils; make clean)
	@(cd plib; make clean)


libsip:	.DUMMY
#	cp $(SIPDIR)/lib/libsipcore.a lib/i86pc/
#	cp $(SIPDIR)/lib/libsipapi.a lib/i86pc/
.DUMMY:
