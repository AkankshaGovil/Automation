include $(BASE)/global.mk

VPATH = ..
# If you know what your system's byte order is, define XML_BYTE_ORDER:
# use -DXML_BYTE_ORDER=12 for little-endian byte order;
# use -DXML_BYTE_ORDER=21 for big-endian (network) byte order.
# -DXML_NS adds support for checking of lexical aspects of XML namespaces spec
# -DXML_MIN_SIZE makes a smaller but slower parser

#
# DXML_BYTE_ORDER defined in env.sh, and -g in globals.mk. (Nextone)
#
#CFLAGS+=-O2 -Ixmltok -Ixmlparse -DXML_NS -DXML_BYTE_ORDER=12 -g
CFLAGS+= -Ixmltok -Ixmlparse -DXML_NS
# Use one of the next two lines; unixfilemap is better if it works.
FILEMAP_OBJ=xmlwf/unixfilemap.o
#FILEMAP_OBJ=xmlwf/readfilemap.o
OBJS=xmltok/xmltok.o \
  xmltok/xmlrole.o \
  xmlwf/xmlwf.o \
  xmlwf/xmlfile.o \
  xmlwf/codepage.o \
  xmlparse/xmlparse.o \
  xmlparse/hashtable.o \
  $(FILEMAP_OBJ)
OBJECTS=xmltok/xmltok.o \
  xmltok/xmlrole.o \
  xmlparse/xmlparse.o \
  xmlparse/hashtable.o \
  $(FILEMAP_OBJ)
EXE=

OBJDIRS=xmlwf xmltok xmlparse gennmtab

objdirs: 
	for subdir in $(OBJDIRS); do \
		if (test ! -d $$subdir); then \
			mkdir $$subdir; \
		fi; \
	done

all: xmlwf/xmlwf$(EXE)

libxml.a: $(OBJECTS)
	ar ruv $@ $^

lib install: objdirs libxml.a
	cp libxml.a $(DLIB)

ex: 
	$(CC) $(CFLAGS) sample/elements.c $(OBJECTS) -o sample/ex

xmlwf/xmlwf$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) xmlwf/xmlwf$(EXE)

xmltok/nametab.h: gennmtab/gennmtab$(EXE)
	rm -f $@
	gennmtab/gennmtab$(EXE) >$@

gennmtab/gennmtab$(EXE): gennmtab/gennmtab.c
	$(CC) $(CFLAGS) -o $@ $^ $(SFIOLIBS)

xmltok/xmltok.o: xmltok/nametab.h

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
