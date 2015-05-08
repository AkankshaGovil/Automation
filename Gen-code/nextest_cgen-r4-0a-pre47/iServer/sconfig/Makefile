#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk

VPATH = ..
vpath *.c ..

CFLAGS += -I.

OBJS = serverp.o \
	config.tab.o \
	lex.sconfig.o \
	parse.o

SCONFIGOBJS = sconfig.o $(BASE)/lm/$(TARGET)/ex_data.o

exec: sconfig

install: lib sconfig

lib : $(OBJS)
	ar ruv $(LIBSCONFIG) $(OBJS)

parse.o : config.tab.c lex.sconfig.c

config.tab.c: config.y
	bison -v -t -d -p sconfig -o $@ $^

lex.sconfig.c : config.l
	flex -Psconfig $^

sconfig : $(SCONFIGOBJS) $(LIBSCONFIG) $(LIBUTILS) $(LIBSRVR) $(LIBMEM) $(LIBTAVL) $(LIBAVL) $(LIBCOMMON) $(LIBOSD) $(LIBSLOCKS) $(LIBCRYPTO) $(LIBDB) $(LIBTST) $(LIBXML)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	$(CP) $@ $(BINDIR)

clean:	
	$(RM) $(OBJS) lex.sconfig.c config.tab.* config.output $(SCONFIGOBJS) sconfig

depend:
	$(MAKEDEPEND) $(CFLAGS) *.c


# DO NOT DELETE
