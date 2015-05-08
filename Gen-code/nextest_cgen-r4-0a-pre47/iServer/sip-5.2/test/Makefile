########################################################################
# 		This is the primary Makefile for the SipStack   	
########################################################################
#
#		Creates libsip and test programs
#		
#		Usage: make 
#	
#	Please see MakeVars for various compiler settings
########################################################################
include ../MakeVars

TESTINCLUDES=-I../source/parser/h -I../source/accessor/h -I../source/accessor/dcs/h -I../source/common/h -I../source/common/dcs/h  -I../source/common/ccp/h/  -I../source/common/rpr/h/ -I../source/accessor/ccp/h -I../source/accessor/rpr/h  -I../source/common/tel/h/ -I../source/accessor/tel/h/ -I../source/parser/tel/h/ -I../source/common/h 

TESTLDFLAGS=$(LDFLAGS:-L%=-L%)

TESTOBJFILES=parser/siptest.o parser/siptxntimer.o parser/txnclone.o parser/txnfree.o parser/txndecode.o \
			 parser/txndecodeintrnl.o parser/txninit.o

all: test 

test:bin/siptest bin/sendmessage  

performance: bin/encodetime bin/parsetime

bin/sendmessage: parser/sendmessage.c
	$(CC) $(CFLAGS) $(TESTLDFLAGS) $^ -o $@

parser/siptest.o:	parser/siptest.c
	$(CC) -DSIP_TXN_LAYER $(CFLAGS) $(TESTINCLUDES) -c $^ -o $@

parser/siptxntimer.o:	parser/siptxntimer.c
	$(CC) -DSIP_TXN_LAYER $(CFLAGS) $(TESTINCLUDES) -c $^ -o $@

parser/txnclone.o:	../source/common/src/txnclone.c
	$(CC) -DSIP_TXN_LAYER $(CFLAGS) $(TESTINCLUDES) -c $^ -o $@

parser/txnfree.o:	../source/common/src/txnfree.c
	$(CC) -DSIP_TXN_LAYER $(CFLAGS) $(TESTINCLUDES) -c $^ -o $@

parser/txndecode.o:	../source/parser/src/txndecode.c
	$(CC) -DSIP_TXN_LAYER $(CFLAGS) $(TESTINCLUDES) -c $^ -o $@

parser/txndecodeintrnl.o:	../source/parser/src/txndecodeintrnl.c
	$(CC) -DSIP_TXN_LAYER $(CFLAGS) $(TESTINCLUDES) -c $^ -o $@

parser/txninit.o:	../source/common/src/txninit.c
	$(CC) -DSIP_TXN_LAYER $(CFLAGS) $(TESTINCLUDES) -c $^ -o $@

bin/siptest:  ../lib/libsipapi.a $(TESTOBJFILES)
	$(CC) -DSIP_TXN_LAYER $(TESTOBJFILES) $(TESTLDFLAGS) -lsipapi -lsipcore -lpthread -lsipserialize -o $@

#bin/siptest:  ../lib/libsipapi.a parser/siptest.c parser/timermanager.h \
#		parser/networkmanager.h parser/networkmanager.c \
#	parser/siptxntimer.c \
#	parser/timermanager.c
#	$(PURIFYCC) parser/timermanager.c parser/networkmanager.c \
#	parser/siptxntimer.c \
#	 parser/siptest.c \
#$(CFLAGS) $(TESTINCLUDES) $(TESTLDFLAGS) -lsipapi -lsipcore -lpthread -lsipserialize -o $@

bin/encodetime: ../lib/libsipcpp.a parser/encodetime.cpp parser/timermanager.h parser/timermanager.cpp parser/networkmanager.h parser/networkmanager.cpp
	$(PURIFYCC) parser/encodetime.cpp parser/timermanager.cpp \
	parser/siptxntimer.cpp \
	parser/networkmanager.cpp $(CFLAGS) $(TESTINCLUDES) $(TESTLDFLAGS) -lsipcpp -lpthread -lsipserialize -o $@

bin/parsetime: ../lib/libsipcpp.a parser/parsetime.cpp parser/timermanager.h parser/timermanager.cpp parser/networkmanager.h parser/networkmanager.cpp
	$(PURIFYCC) parser/parsetime.cpp parser/timermanager.cpp \
	parser/siptxntimer.cpp \
	parser/networkmanager.cpp $(CFLAGS) $(TESTINCLUDES) $(TESTLDFLAGS) -lsipcpp -lpthread -lsipserialize -o $@

