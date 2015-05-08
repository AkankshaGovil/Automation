##############################################################
#### 		ASN EncoderDecoder Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS = emanag.o q931.o persimpl.o perseq.o perchoic.o perseqof.o peroctst.o 	peroid.o perchrst.o per.o perBitString.o coder.o
TARGET = coder
PROGFLAGS = -I../lowunix -I../log -I../ads -I../pvt_pst -I../utils -I../asn_objects -I../defs
DISTFILES =
PROGS =
PROGLIBS =
PROGDEFS = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
