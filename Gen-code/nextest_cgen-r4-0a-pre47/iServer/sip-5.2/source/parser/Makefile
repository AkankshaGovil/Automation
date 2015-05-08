#######################################################################
# 		This is the Parser  Makefile for the SipStack   	
########################################################################
#
#		Creates libsipcore
#		
########################################################################
include ../../MakeVars

COMMONRPRSRC = rprfree.c \
	rprinit.c
COMMONDCSSRC = dcsinit.c \
dcsfree.c
COMMONSRC = portlayer.c \
	sipinit.c \
	sipfree.c \
	siplist.c \
	siptrace.c \
	sipstatistics.c \
 	siphash.c \
	imppinit.c \
	imppfree.c \
	sipbcptinit.c \
	sipbcptfree.c
PARSERSRC = sipdecode.c\
			siptimer.c\
			sipformmessage.c\
			sipparserclone.c\
			sipsendmessage.c \
			sdpdecode.c
LEXSRC = Attrib.l \
	Fromto.l \
	To.l \
	Pgp.l \
	Tokencomment.l \
	Via.l \
	Contact.l \
	Reqline.l \
	Tokens.l \
	Datetime.l \
	Key.l \
	Sdp.l \
	Tokensltoken.l \
	Statusline.l \
	Utf8.l \
	Mime.l \
	RprTokens.l \
	Dcs.l  \
	Im.l  \
	Media.l \

BISONSRC =	Attrib.y \
	Fromto.y \
	To.y \
	Pgp.y \
	Tokencomment.y \
	Via.y \
	Contact.y \
	Reqline.y \
	Tokens.y \
	Datetime.y \
	Sdp.y \
	Tokensltoken.y \
	Media.y \
	Statusline.y \
	Utf8.y \
	Mime.y \
	RprTokens.y \
	Dcs.y \
	Im.y \
        Key.y \

DCSPARSESRC = dcsdecode.c \
	dcsformmessage.c
	
INCLUDES =  -Ih -I../common/h -Ibison/cfiles  \
		-I../common/rpr/h/  \
		-I../accessor/rpr/h/  \
		-I../common/dcs/h/  -Idcs/h    \
			-I../parser/h 


COMMONOBJS = $(COMMONSRC:%.c=../../obj/common/%.o) 
COMMONRPROBJS = $(COMMONRPRSRC:%.c=../../obj/common/rpr/%.o) 
COMMONDCSOBJS = $(COMMONDCSSRC:%.c=../../obj/common/dcs/%.o) 
PARSEROBJS = $(PARSERSRC:%.c=../../obj/parser/%.o) 
LEXCFILES = $(LEXSRC:%.l=lex/cfiles/lex.%.c) 
BISONCFILES = $(BISONSRC:%.y=bison/cfiles/%.tab.c) 
LEXOBJS = $(LEXSRC:%.l=../../obj/parser/lex/lex.%.o) 
BISONOBJS = $(BISONSRC:%.y=../../obj/parser/bison/%.tab.o) 
DCSPARSEOBJS = $(DCSPARSESRC:%.c=../../obj/parser/dcs/%.o)

OBJECTS = $(COMMONOBJS)  $(BISONOBJS) $(LEXOBJS) $(COMMONRPROBJS)  \
			$(COMMONDCSOBJS) $(DCSPARSEOBJS)  \
		 $(PARSEROBJS)

.PRECIOUS:	$(BISONCFILES) $(LEXCFILES)
all: libsipcore 

libsipcore: ../../lib/libsipcore.a
../../lib/libsipcore.a: $(OBJECTS)
	$(AR)  ../../lib/libsipcore.a $(OBJECTS)
include .depend
../../obj/common/%.o: ../common/src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
../../obj/common/rpr/%.o: ../common/rpr/src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
../../obj/common/dcs/%.o: ../common/dcs/src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
../../obj/parser/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
../../obj/parser/lex/%.o: lex/cfiles/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
../../obj/parser/bison/%.tab.o: bison/cfiles/%.tab.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
../../obj/parser/dcs/%.o: dcs/src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
bison/cfiles/%.tab.c: bison/%.y
	$(YACC) --debug -d -p`echo $? | awk -F/ '{print $$2}' | awk -F. '{print "glbSipParser"$$1}'` $? -o$@
	cat $@ | sed "s/malloc(/BISONMALLOC(/g" | sed "s/\<free[ ]*(/BISONFREE(/g" | sed "s/<stdio\.h>/\"portlayer\.h\"/g" | sed "s/fprintf/HSS_FPRINTF/g" | sed "s/strdup/BISONSTRDUP/g" | sed "s/\<realloc(/BISONREALLOC(/g" > $@.tmp
	mv -f $@.tmp $@
	cat $@ | sed "s/YYSTACK_ALLOC malloc/YYSTACK_ALLOC BISONMALLOC/g" > $@.tmp
	mv -f $@.tmp $@

../../obj/parser/bison/%.tab.o: source/parser/bison/cfiles/%.tab.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
lex/cfiles/lex.%.c: lex/%.l
	$(LEX) -t -P`echo $? | awk -F/ '{print $$2}' | awk -F. '{print "glbSipParser"$$1}'` $? | sed "s/malloc(/FLEXMALLOC(/g" | sed "s/free( ptr );/FLEXFREE(ptr);/g" | sed "s/<stdio\.h>/\"portlayer\.h\"/g" | sed "s/fprintf/HSS_FPRINTF/g" | sed "s/ferror/HSS_FERROR/g" | sed "s/fread/HSS_FREAD/g" | sed "s/getc/HSS_GETC/g" | sed "s/strdup/FLEXSTRDUP/g" | sed "s/\<realloc(/FLEXREALLOC(/g" > $@
../../obj/parser/lex/lex.%.o: lex/cfiles/lex.%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
