###################################################
###						###
### Example of Module Makefile			###
###						###
###################################################

PROJECT_NAME = example

# Subdirectories to run make in for the primary targets.
SRCB_LIBSSUBDIRS = ex1
SRCB_APPSSUBDIRS = ex

# Supplumentary directories
SUPP_LIBSUBDIRS = 
SUPP_APPSUBDIRS = 


# large library
LARGELIB = exlib
LIBGROUP = ex

PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/make/module.mk
