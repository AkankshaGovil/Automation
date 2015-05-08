include $(BASE)/global.mk

SOURCES = check_quedb.c 
TESTDBSDIR = testdbs
TESTEXEC = check_quedb
PERFEXEC = quedb_perf
OBJECTS = check_quedb.o
PERFOBJ = quedb_perf.o
LIBS = $(LIBQUEDB) $(LIBCOMMON) -lcheck -lpthread
CFLAGS += -Wall

all:	test

test:	$(TESTEXEC)

perf:	$(PERFEXEC)

$(TESTEXEC): $(OBJECTS) $(LIBQUEDB) $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@if (test -d $(TESTDBSDIR)); then \
		rm -rf  $(TESTDBSDIR); \
	fi; \
	mkdir $(TESTDBSDIR)

$(PERFEXEC): $(PERFOBJ) $(LIBQUEDB) $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJECTS) $(TESTEXEC) $(PERFOBJ) $(PERFEXEC)
	rm -rf $(TESTDBSDIR)/*
