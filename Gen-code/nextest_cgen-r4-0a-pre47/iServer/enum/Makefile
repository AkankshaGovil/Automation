##***************************************************************************
##   Top-level build makefile for E.164 RESOLVER library
##
## Copyright (c) 2001 NetNumber.com, Inc. All Rights Reserved.
##
## This application is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation; either
## version 2.1 of the License, or (at your option) any later version.
##
## This application is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public
## License along with this application; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##
##
##***************************************************************************

include $(BASE)/global.mk

###################
## Variables
###################
AR = ar
CC = gcc
CFLAGS = -Wall -g

##
## Directories
##  Set RESOLVER_DIR to your appropriate base directory.
##
RESOLVER_DIR         = /nndev/resolver/C
RESOLVER_DIR         = ..

RESOLVER_SRC_DIR     = $(RESOLVER_DIR)/src
RESOLVER_INC_DIR     = $(RESOLVER_DIR)/include
#RESOLVER_LIB_DIR     = $(RESOLVER_DIR)/lib
RESOLVER_LIB_DIR     = $(DLIB)
RESOLVER_BIN_DIR     = $(RESOLVER_DIR)/bin

RESOLVER_LIB   = libENUM.a
VPATH				= $(RESOLVER_DIR)/src

ifeq ("X$(PLATFORM)", "Xlinux")
LDLIBS = -lresolv -lENUM -lbind
else
LDLIBS= -lresolv -lENUM
endif

###################
## Top-Level Targets
###################

lib: all

install: all

all: formatter naptr_resolver enum_resolver naptr_client enum_client
test_all: formatter naptr_resolver enum_resolver naptr_client enum_client

clean: 
	rm -f $(RESOLVER_BIN_DIR)/naptr_client \
              $(RESOLVER_BIN_DIR)/enum_client \
              $(RESOLVER_LIB_DIR)/*.[ao]

formatter:
	$(CC) $(CFLAGS) $(PROFFLAGS) -I$(RESOLVER_INC_DIR) -c -o $(RESOLVER_LIB_DIR)/$@.o $(RESOLVER_SRC_DIR)/$@.c
	$(AR) -r $(RESOLVER_LIB_DIR)/$(RESOLVER_LIB) $(RESOLVER_LIB_DIR)/$@.o
	rm -f $(RESOLVER_LIB_DIR)/$@.o

naptr_resolver:
	$(CC) $(CFLAGS) $(PROFFLAGS) -I$(RESOLVER_INC_DIR) -c -o $(RESOLVER_LIB_DIR)/$@.o $(RESOLVER_SRC_DIR)/$@.c
	$(AR) -r $(RESOLVER_LIB_DIR)/$(RESOLVER_LIB) $(RESOLVER_LIB_DIR)/$@.o
	rm -f $(RESOLVER_LIB_DIR)/$@.o

enum_resolver:
	$(CC) $(CFLAGS) $(PROFFLAGS) -I$(RESOLVER_INC_DIR) -c -o $(RESOLVER_LIB_DIR)/$@.o $(RESOLVER_SRC_DIR)/$@.c
	$(AR) -r $(RESOLVER_LIB_DIR)/$(RESOLVER_LIB) $(RESOLVER_LIB_DIR)/$@.o
	rm -f $(RESOLVER_LIB_DIR)/$@.o

naptr_client:
	$(CC) $(CFLAGS) $(PROFFLAGS) -L$(RESOLVER_LIB_DIR) -I$(RESOLVER_INC_DIR) -o $(RESOLVER_BIN_DIR)/$@ $(RESOLVER_SRC_DIR)/$@.c $(LDLIBS) $(SFIOLIBS) 

enum_client:
	$(CC) $(CFLAGS) $(PROFFLAGS) -L$(RESOLVER_LIB_DIR) -I$(RESOLVER_INC_DIR) -o $(RESOLVER_BIN_DIR)/$@ $(RESOLVER_SRC_DIR)/$@.c $(LDLIBS) $(SFIOLIBS)
