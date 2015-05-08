##############################################################
#### 		Example of Application Makefile
##############################################################

PROJECT_NAME = example
OBJECTS   = ex1Main.o
TARGET    = ex1
PROGLIBS  = -lex
PROGFLAGS =  -I../ex1
DISTFILES =
PROGDEFS  = 

PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/make/common.mk
APPTARGET = $(bindir)/$(TARGET)
all: $(APPTARGET)
include $(PROJECT_BASE)/make/app.mk
