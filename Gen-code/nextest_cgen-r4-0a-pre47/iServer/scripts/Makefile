
######################
##### Makfile ########
######################
include $(BASE)/global.mk

#
# Define VPATH as ., as install target clashes with upper level
# directory "install ;)
#
VPATH = .

all:	install

install:	
	cp iserver $(BINDIR)
	cp ipcrmall $(BINDIR)
	cp logtrim $(BINDIR)
	cp nsfconfig $(BINDIR)
	cp fwconfig $(BINDIR)
	cp itune $(BINDIR)
	cp cdrdict.xml $(BINDIR)
	cp ifmgmt $(BINDIR)

clean:
