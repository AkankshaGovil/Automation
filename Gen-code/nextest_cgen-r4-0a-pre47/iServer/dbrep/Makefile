
include $(BASE)/global.mk
include $(BASE)/dbrep/Make.defines

.PHONY: lib

all:
	@echo "Nothing to make in this directory"
	@echo "Please read the README file"

lib exec:
	( if [ ! -d ../lib ]; then mkdir ../lib; fi)
	( if [ ! -d ../bin ]; then mkdir ../bin; fi)
	( if [ ! -d ../lib/$(TARGET) ]; then mkdir ../lib/$(TARGET); fi)
	( if [ ! -d ../bin/$(TARGET) ]; then mkdir ../bin/$(TARGET); fi)
	( if [ ! -d ../libgen/$(TARGET) ]; then mkdir ../libgen/$(TARGET); fi)
	( if [ ! -d ../libmcast/$(TARGET) ]; then mkdir ../libmcast/$(TARGET); fi)
	( if [ ! -d ../hello/$(TARGET) ]; then mkdir ../hello/$(TARGET); fi)
	( if [ ! -d ../mcast/$(TARGET) ]; then mkdir ../mcast/$(TARGET); fi)
	( cd ../libgen/$(TARGET); if [ ! -f Makefile ]; then $(LN) ../Makefile Makefile; fi; make $@)
	( cd ../libmcast/$(TARGET); if [ ! -f Makefile ]; then $(LN) ../Makefile Makefile; fi; make $@)
	( cd ../hello/$(TARGET); if [ ! -f Makefile ]; then $(LN) ../Makefile Makefile; fi; make $@)
	( cd ../mcast/$(TARGET); if [ ! -f Makefile ]; then $(LN) ../Makefile Makefile; fi; make $@)

install:	exec

tags:	
	( /usr/local/bin/ctags -R --exclude="CVS" ../)

clean:
	$(RM) 	$(BASE)/dbrep/mcast/$(TARGET)/*.o 	\
			$(BASE)/dbrep/hello/$(TARGET)/*.o 	\
			$(BASE)/dbrep/libgen/$(TARGET)/*.o 	\
			$(BASE)/dbrep/libmcast/$(TARGET)/*.o 	\
			$(BASE)/dbrep/lib/$(TARGET)/lib*.a
 
distclean:
	-rm -f	$(BASE)/dbrep/mcast/$(TARGET)/*.o 	\
			$(BASE)/dbrep/hello/$(TARGET)/*.o 	\
			$(BASE)/dbrep/libgen/$(TARGET)/*.o 	\
			$(BASE)/dbrep/libmcast/$(TARGET)/*.o 	\
			$(BASE)/dbrep/lib/$(TARGET)/lib*.a

