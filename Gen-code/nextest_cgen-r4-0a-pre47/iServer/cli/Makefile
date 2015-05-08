
#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk

CFLAGS += -DSAVE_SEC

LIBOBJECTS = cli.o clinetoid.o clivpn.o clidb.o cliusage.o 	\
			cliaux.o parse.o clifaxs.o clicp.o cligkcall.o 	\
			clicache.o clirsd.o cligk.o clitrigger.o clirealm.o	\
			cligisq.o cliigrp.o cliext.o clivnet.o

DEPENDLIBOBJECTS = $(LIBDB) $(LIBUTILS) $(LIBSRVR) $(LIBMEM) $(LIBTAVL) $(LIBAVL) $(LIBSCONFIG) $(LIBCOMMON) $(LIBOSD) $(LIBSLOCKS) $(LIBCRYPTO) $(LIBLOCK) $(LIBTST) $(LIBRSD) $(LIBXML)

LIB=libcli.so

OBJECTS = main.o clilog.o $(LIBOBJECTS) $(BASE)/lm/ex_data.o

ALUI_OBJECTS = ALUI_Commands.o ALUI_AppInit.o 

lib: $(LIBCLI)

exec: $(BINCLI)

all: $(BINCLI) $(LIBCLI)
##
## Manager level programs.
##

$(LIBCLI): $(LIBOBJECTS)
	ar ruv $@ $^

$(BINCLI): $(OBJECTS) $(LIBDB) $(DEPENDLIBOBJECTS) $(HEADERS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(DEPENDLIBOBJECTS) $(LDLIBS)

#lib: $(LIB) $(LIBOBJECTS) $(DEPENDLIBOBJECTS) $(HEADERS) Makefile

$(LIB): $(LIBOBJECTS) 
	$(CCSHARED) $(LDFLAGS) -o $@ $(LIBOBJECTS) $(DEPENDLIBOBJECTS) $(LDLIBS) -lc

cliwrapper: cliwrapper.o clilog.o $(LIBOBJECTS) $(DEPENDLIBOBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

one: parse.o $(LIBDB) $(LIBUTILS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ parse.o $(LIBDB) $(LIBUTILS) -lfl

#parse.o : one.y one.l parse.c
#	bison -d one.y
#	flex one.l
#	$(CC) $(CFLAGS) -o $@ -c parse.c

parse.o	: one.tab.c lex.yy.c

one.tab.c : one.y
	bison -d -o $@ $^

lex.yy.c : one.l
	flex $^

clean:	
	$(RM) -f $(OBJECTS) one.tab.* lex.yy.c one.output cli libcli.so

depend:
	-touch deps
	$(MAKEDEPEND) -f deps $(CFLAGS) *.c

install:	all
#	$(CP) $(LIB) $(LIBCLISO)

##.BEGIN:
##	if [ -f deps ]; then \
##		include deps \
##	else \
##		touch deps \
##		make depend \
##	fi \
# DO NOT DELETE

