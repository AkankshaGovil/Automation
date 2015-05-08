##
## Makefile
##

include $(BASE)/global.mk


CFLAGS := -I../include $(CFLAGS) $(PROFFLAGS) 

OBJS =	radclient.o \
	radauth.o \
	radacct.o \
	lex.radacct.o

ifeq ($(DDB), QUEDB)
CFLAGS := -I$(BASE)/quedb $(CFLAGS)
OBJS += radacct_quedb.o
else
CFLAGS := -I/usr/local/BerkeleyDB.4.1/include $(CFLAGS)
OBJS += radacct_db.o
endif


ALLOBJS = $(OBJS)

lib install: $(LIBRADC)
	@$(CP) libradius.a $(DLIB)

$(LIBRADC): $(OBJS)
	$(AR) ruv $@ $^

lex.radacct.c: marshal.l
	flex -Cr -Pradacct $^

clean:	
	rm -f $(ALLOBJS) $(LIBRADC)

