#################################################################
#################################################################
##
##                      Makefile
##
#################################################################
#################################################################
##
## Makefile for making Perl Libraries.
##

## 
## Targets supported are 
##
## config - creates the Makefile from Makefile.PL
##          it is better to generate this in separate step from install
##          so the module is not recompiled if it is already compiled
##
## install - installs the modules in $BASE/lib
##
## clean - runs make clean in all perl mod subdirs
##

##
## Add all the perl module names which follow the standard processing
##

ifeq ("${MKMK_VER}","new")
MKMK_PRE = "PREFIX=$(BASE)"
else
MKMK_POS = "PREFIX=$(BASE)"
endif

PERL_MOD_DIRS = Statistics-Descriptive-2.4 \
	Mail-Sendmail-0.78 \
	Date-EzDate-0.93 \
	libnet-1.0704 \
	MIME-Base64-2.12 \
	File-Rsync-0.20 \
	Net-Telnet-3.02 \
	File-Tail-0.98 \
	Time-HiRes-1.35 \
	Digest-Perl-MD5-1.5 \
	Config-Simple-2.82 \
	DBI-1.18 \
	Msql-Mysql-modules-1.2219 \
	TermReadKey-2.21
#	encryption \ Removed as no longer used by msw.

OTH_MOD_DIRS = expat \
	XML-Parser-2.30 \
	XML-Simple-1.08_01

PLIBDIRS = $(PERL_MOD_DIRS) $(OTH_MOD_DIRS)

##
## install
##
install lib exec: compile createdist

##
## narsinstall
##
narsinstall: checkmysql compile createdist
	@(cd $(BASE)/lib; tar uf libperl.tar libmysqlclient.so.10)


##
## compile
##
compile: othmod_compile perlmod_compile

othmod_compile: .DUMMY
	@(cd expat; if [ ! -r Makefile ]; then ./configure; fi)
	@(cd expat; make prefix=$(BASE) bindir=$(BASE)/bin/$(TARGET) libdir=$(BASE)/lib/$(TARGET) install)
	@(cd XML-Parser-2.30; if [ ! -r Makefile ]; then perl Makefile.PL EXPATINCPATH=$(BASE)/include EXPATLIBPATH=$(BASE)/lib/$(TARGET) $(MKMK_PRE); fi)
	@(cd XML-Parser-2.30; make $(MKMK_POS) install)
	@(cd XML-Simple-1.08_01; if [ ! -r Makefile ]; then perl -I$(BASE)/lib/perl5/site_perl Makefile.PL $(MKMK_PRE); fi)
	@(cd XML-Simple-1.08_01; make $(MKMK_POS) install)

perlmod_compile: .DUMMY
	@for subdir in $(PERL_MOD_DIRS); do \
		(cd $$subdir;  if [ ! -r Makefile ]; then perl Makefile.PL $(MKMK_PRE); fi; exit $$?; ); \
		if [ $$? -ne 0 ]; then \
			echo "======================"; \
			echo "Error making Makefile in $$subdir"; \
			echo "======================"; \
			exit 1; \
		fi; \
		(cd $$subdir;  make $(MKMK_POS) install; exit $$?; ); \
		if [ $$? -ne 0 ]; then \
			echo "======================"; \
			echo "Error Compiling and Installing $$subdir"; \
			echo "======================"; \
			exit 1; \
		fi; \
	done


##
## createdist
##
createdist: .DUMMY
	@(cd $(BASE)/lib/perl5/site_perl; cp -fpR $(PERLVERSION) $(PERLADJVERSION); cp -fpR $(PERLVERSION) $(PERLOLDVERSION);)
	@(cd $(BASE)/lib; tar cvf libperl.tar perl5)
	@(cd $(BASE)/lib/$(TARGET); tar cvf libexpatpackage.tar libexpat.* ; mv -f libexpatpackage.tar .. )


##
## createdist
##
checkmysql: .DUMMY
	@(if (test ! -r /usr/local/mysql/lib/mysql/libmysqlclient.so.10); then \
		echo "============================================================================="; \
		echo "Need local mysql installation at /usr/local/mysql to create narsagent package"; \
		echo "============================================================================="; \
		exit 1; \
	fi; \
	if (test ! -d $(BASE)/lib); then \
		mkdir $(BASE)/lib; \
	fi; \
	cd $(BASE)/lib; $(BASE)/utils/links.sh $(TARGET) Makefile);
	cd $(BASE)/lib; cp /usr/local/mysql/lib/mysql/libmysqlclient.so.10 .;


##
## clean
##
clean: .DUMMY
	@for subdir in $(PLIBDIRS); do \
		(cd $$subdir;  if [ -r Makefile ]; then make clean; fi; exit $$?; ); \
		if [ $$? -ne 0 ]; then \
			echo "======================"; \
			echo "Error cleaning $$subdir"; \
			echo "======================"; \
			exit 1; \
		fi \
	done



.DUMMY:

