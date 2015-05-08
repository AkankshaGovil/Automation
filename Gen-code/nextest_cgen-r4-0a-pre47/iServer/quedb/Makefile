include $(BASE)/global.mk

OBJS =  dbcommit.o dbget.o dbmanager.o metadatamanager.o dbput.o utils.o datamanager.o 

TEST = tests

all:	lib browse

lib: $(LIBQUEDB)

$(LIBQUEDB): $(OBJS) $(LIBCOMMON)
	$(AR) ruv $@ $^

test:
	@(cd $(TEST); make)

browse:	db_browse

db_browse:

	$(CC) ../dbbrowse.c $(CFLAGS) -lquedb -lpthread -lcommon -o $@
	@(cp $@ $(BASE)/bin/$(TARGET)/)

clean:	
	rm -f $(OBJS) $(LIBQUEDB) db_browse

