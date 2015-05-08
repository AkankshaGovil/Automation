
all: cm other asnfiles addons

rm: clean
clean:
	rm -f *.h


# CM related header files.
# All of these files are needed for the include of cm.h
cm: cm.h cmsize.h cmhook.h cmras.h cmctrl.h faststart.h transpcap.h redencod.h userinput.h coder.h pvaltree.h psyntree.h ci.h cibuf.h rvcommon.h rvdefunix.h msg.h rv_defs.h seli.h mti.h

cm.h : ../cm/cm.h
	cp ../cm/cm.h . > /dev/null

cmsize.h : ../cm/cmsize.h
	cp ../cm/cmsize.h . > /dev/null

cmhook.h : ../cm/cmhook.h
	cp ../cm/cmhook.h . > /dev/null

cmcfgrt.h : ../cm/cmcfgrt.h
	cp ../cm/cmcfgrt.h . > /dev/null

cmras.h : ../ras/cmras.h
	cp ../ras/cmras.h . > /dev/null

cmctrl.h : ../h245/cmctrl.h
	cp ../h245/cmctrl.h . > /dev/null

faststart.h : ../h245/faststart.h
	cp ../h245/faststart.h . > /dev/null

transpcap.h : ../h245/transpcap.h
	cp ../h245/transpcap.h . > /dev/null

redencod.h : ../h245/redencod.h
	cp ../h245/redencod.h . > /dev/null

userinput.h : ../h245/userinput.h
	cp ../h245/userinput.h . > /dev/null

coder.h : ../coder/coder.h
	cp ../coder/coder.h . > /dev/null

pvaltree.h : ../pvt_pst/pvaltree.h
	cp ../pvt_pst/pvaltree.h . > /dev/null

psyntree.h : ../pvt_pst/psyntree.h
	cp ../pvt_pst/psyntree.h . > /dev/null

ci.h : ../config/ci.h
	cp ../config/ci.h . > /dev/null

cibuf.h : ../config/cibuf.h
	cp ../config/cibuf.h . > /dev/null

rvcommon.h : ../defs/rvcommon.h
	cp ../defs/rvcommon.h . > /dev/null

rvdefunix.h : ../lowunix/rvdefunix.h
	cp ../lowunix/rvdefunix.h . > /dev/null

msg.h : ../lowunix/msg.h
	cp ../lowunix/msg.h . > /dev/null

rv_defs.h : ../defs/rv_defs.h
	cp ../defs/rv_defs.h . > /dev/null

seli.h : ../lowunix/seli.h
	cp ../lowunix/seli.h . > /dev/null

mti.h : ../lowunix/mti.h
	cp ../lowunix/mti.h . > /dev/null


# OTHER header files
# May be left out, or used by the application
other: stkutils.h Threads_API.h

stkutils.h : ../cm/stkutils.h
	cp ../cm/stkutils.h . > /dev/null

Threads_API.h : ../lowunix/Threads_API.h
	cp ../lowunix/Threads_API.h . > /dev/null

# ASNFILES header files
# ASN.1 generated header files, for use with PVT functions
asnfiles: q931asn1.h h245.h

q931asn1.h : ../asn_objects/q931asn1.h
	cp ../asn_objects/q931asn1.h . > /dev/null

h245.h : ../asn_objects/h245.h
	cp ../asn_objects/h245.h . > /dev/null


# ADDONS header files
# This part is processed only if the addons can be found in the stack's directory
addons : rtp mib supserve sse h235


# RTP Addon header files
DIREXIST = $(wildcard ../../rtpRtcp)
ifneq ($(DIREXIST), )

rtp : payload.h rtcp.h rtp.h

payload.h : ../../rtpRtcp/rtp/payload.h
	cp ../../rtpRtcp/rtp/payload.h . > NUL

rtcp.h : ../../rtpRtcp/rtp/rtcp.h
	cp ../../rtpRtcp/rtp/rtcp.h . > NUL

rtp.h : ../../rtpRtcp/rtp/rtp.h
	cp ../../rtpRtcp/rtp/rtp.h . > NUL

else

rtp :

endif



# MIB Addon header files
DIREXIST = $(wildcard ../snmp)
ifneq ($(DIREXIST), )

mib : rvh341agent.h h341common.h snmpI.h applSnmpI.h

rvh341agent.h : ../snmp/rvh341agent.h
	cp ../snmp/rvh341agent.h . > /dev/null

h341common.h : ../snmp/h341common.h
	cp ../snmp/h341common.h . > /dev/null

snmpI.h : ../snmp/snmpI.h
	cp ../snmp/snmpI.h . > /dev/null

applSnmpI.h : ../applSnmp/applSnmpI.h
	cp ../applSnmp/applSnmpI.h . > /dev/null

else

mib :

endif


# SUPS Addon header files
DIREXIST = $(wildcard ../supserve)
ifneq ($(DIREXIST), )

supserve : sssize.h h450.h h450asn.h

sssize.h : ../supserve/sssize.h
	cp ../supserve/sssize.h . > /dev/null

h450.h : ../supserve/h450.h
	cp ../supserve/h450.h . > /dev/null

h450asn.h : ../supserve/h450asn.h
	cp ../supserve/h450asn.h . > /dev/null

else

supserve :

endif


# SSE Addon header files
DIREXIST = $(wildcard ../sse)
ifneq ($(DIREXIST), )

sse : sse.h

sse.h : ../sse/sse.h
	cp ../sse/sse.h . > /dev/null

else

sse :

endif


# H235 Addon header files
DIREXIST = $(wildcard ../h235)
ifneq ($(DIREXIST), )

h235 : h235_api.h

h235_api.h : ../h235/h235_api.h
	cp ../h235/h235_api.h . > /dev/null

else

h235 :

endif
