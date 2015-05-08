###################################################
###						###
### H.323 Stack project Makefile		###
###						###
###################################################

PROJECT_NAME = h323

# Subdirectories to run make in for the primary targets.
SRCB_LIBSSUBDIRS = include ads asn_objects cat cm coder config h245 log transport ras utils mib pvt_pst lowunix snmp ../rtpRtcp/rtp

SRCB_APPSSUBDIRS = ../Appl/testapp


# large library
LARGELIB = rvh323
LIBGROUP = ads low utils pvtpst coder conf cm h245 cat ras asn trans mib log

PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))

include $(PROJECT_BASE)/../make/unix/module.mk
