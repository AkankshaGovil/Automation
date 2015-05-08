
#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk


ifeq ("X$(PLATFORM)", "Xlinux")
LIBS= -Wl,--start-group 
LIBGISRPC=
else
LIBS=
endif

LIBS += $(LIBMEM) $(LIBDB) $(LIBUTILS) $(LIBSRVR) $(LIBUTIL) $(LIBCOMMON) \
	$(LIBCRYPTO) $(LIBXML) $(LIBSCONFIG) $(LIBTAVL) $(LIBAVL) $(LIBSLOCKS) $(LIBENUM) \
    $(LIBFCE)  $(LIBLOCK) $(LIBTST) $(LIBGISRPC) \
    $(LIBRADC) $(LIBRADIUS) $(LIBEXECD) $(LIBSCM) $(LIBSIPUA) $(LIBSCMRPC) $(LIBOSD)
   
    
ifeq ("X$(PLATFORM)", "Xlinux")
LIBS+= -Wl,--end-group 
LDLIBS	+= -Wl,-Bstatic -lresolv -Wl,-Bdynamic
else
LDLIBS	+= $(SFIOLIBS)
INCDIRS += $(SFIOINCLUDE)
endif


GISOBJDIR = .
BCSOBJDIR = .

GISOBJS = $(GISOBJDIR)/process.o \
	$(GISOBJDIR)/ls.o \
	$(GISOBJDIR)/lsmain.o \
	$(GISOBJDIR)/entry.o \
	$(GISOBJDIR)/phone.o \
	$(GISOBJDIR)/utils.o \
	$(GISOBJDIR)/xml.o \
	$(GISOBJDIR)/handles.o \
	$(GISOBJDIR)/clid.o \
	$(GISOBJDIR)/lsage.o \
	$(GISOBJDIR)/cdr.o	\
	$(GISOBJDIR)/lsstatus.o	\
	$(GISOBJDIR)/gisq.o	 
ifeq ("X$(PLATFORM)", "Xlinux")
else
GISOBJS += \
	$(GISOBJDIR)/ispd_init.o
endif
GISAGEOBJS = $(GISOBJDIR)/age.o $(GISOBJDIR)/phone.o utils.o xml.o

BCSOBJS = $(BCSOBJDIR)/bcs.o $(BCSOBJDIR)/xml.o

all:	$(BINGIS)

##
## Manager level programs.
##
# if you don't want gis to restart, do a "touch norestart" in ls/$TARGET
$(BINGIS):	$(GISOBJS) $(BASE)/lm/ex_data.o $(LIBGK) $(LIBH323) $(LIBSIP) $(SIPLIBS) $(LIBIWF) #$(LIBS) 
#	-if [ -f /usr/local/bin/sudo -a ! -r ./norestart ]; then \
#		sudo /usr/local/nextone/bin/iserver gis stop; \
#	fi
	$(CC) -O3 $(PROFFLAGS) $(LDFLAGS) -o $@ $^ $(SIPLIBS) $(LIBS) $(LDLIBS)
# if it's a customer build, send them a naked gis
	@if [ "$(BUILDTYPE)" = "standard" ]; then \
	cp $@ $(BASE)/bin/$(TARGET)/symbol_table; strip $@; \
	fi

test:
	$(CC) $(CFLAGS) $(LDFLAGS) test.c

testapp: testappc.o $(LIBH323)
	$(CC) $(PROFFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BINGISAGE):	$(GISAGEOBJS) $(LIBS) $(LIBAGEGK) $(LIBH323)
	$(CC) -O3 $(PROFFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BINBCS):	$(BCSOBJS) $(LIBS)
	$(CC) -O3 $(PROFFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.o:	.c
	$(CC) $(CFLAGS) -o $@ $^

clean:	
	$(RM) $(GISOBJS) $(GISAGEOBJS) $(BCSOBJS) gis gisage lex.yy.c config.tab.c bcs

depend:
	$(MAKEDEPEND) $(CFLAGS) *.c

$(SERVERCONF): server.cfg
	cp $^ $@

exec install: all $(SERVERCONF)

# DO NOT DELETE
