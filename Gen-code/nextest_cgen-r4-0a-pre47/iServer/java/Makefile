
# Makefile
#
# Top level makefile to build different java programs
# JAVACFLAGS is defined in global.mk
#

include $(BASE)/global.mk

JDKPATH := $(JDKPATH)
JAR = $(JDKPATH)/jar
JAVAC = $(JDKPATH)/javac
JAVADOC = $(JDKPATH)/javadoc
JAVAH = $(JDKPATH)/javah
JAVACLASSPATH := $(JAVACLASSPATH)
JAVACFLAGS := $(JAVACFLAGS)
JAVALOCALINCLUDE =
CFLAGS += -w

JAVADOCFLAGS = -windowtitle "iView Documentation" -d docs

CCSHARED := $(CCSHARED)

ifeq ($(PLATFORM), linux)
JAVALIBS=$(LIBCLI) $(LIBRSD) $(LIBDB) $(LIBMEM) $(LIBUTILS) $(LIBSRVR) $(LIBTAVL) $(LIBAVL) $(LIBSCONFIG) $(LIBOSD) $(LIBCOMMON) $(LIBOSD) $(LIBXML) \
	$(LIBSLOCKS) $(LIBLOCK) $(LIBTST) -lgdbm -lfl -lcrypto -lc
else
JAVALIBS=  -lcli -lrsd -ldb -lshm -lutils -lsrvr -lavl -ltavl -lsconfig -losd -lcommon -lslocks -llock -lgdbm -lfl -lcrypto -ltst -lxml $(SFIOLIBS) -lgcc  
endif

include iview.mk
#include ifac.mk
include jserver.mk
#include iportal.mk
#include pingtel.mk
#include jainsip.mk

JAVADOC_PACKAGES = com.nextone.JUCon \
                   com.nextone.JUCon.iEdge500 \
                   com.nextone.JUCon.iEdge1000 \
                   com.nextone.JUCon.iEdge1000.Dhcpserver \
                   com.nextone.JUCon.iServer \
                   com.nextone.JUCon.iServer.Maint \
                   com.nextone.JUCon.DhcpServer \
                   com.nextone.common \
                   com.nextone.util \
                   com.nextone.util.JMask \
                   com.nextone.JServer \
                   com.nextone.JFac \
				   com.nextone.wizard \
				   com.nextone.database

FTP_CLASSES = thirdparty/ftp/FtpBean.class \
              thirdparty/ftp/FtpException.class

TAR_CLASSES	=	thirdparty/tar/InvalidHeaderException.class\
				thirdparty/tar/TarArchive.class\
				thirdparty/tar/TarBuffer.class\
				thirdparty/tar/TarEntry.class\
				thirdparty/tar/TarHeader.class\
				thirdparty/tar/TarInputStream.class\
				thirdparty/tar/TarOutputStream.class\
				thirdparty/tar/TarProgressDisplay.class\
				thirdparty/tar/TarTransFileTyper.class\
				thirdparty/tar/tar.class

# implicit rule to compile .java to .class
%.class : %.java
	$(JAVAC) $(JAVACFLAGS) $< 

all:  fac iview

################################################
## iView related rules
################################################
iView iview: iViewResources.jar iView.jar 
	@echo "======================="
	@echo "Finished building iView"
	@echo "======================="

iViewStrong iviewStrong iview-strong iviewstrong: iView
	cp lib/jce.jar .

iViewResources.jar: iview.mk Makefile $(IVIEW_IMAGES) $(IVIEW_SOUNDS)
	@$(JAR) cf0 $@ \
		$(IVIEW_IMAGES) \
		$(IVIEW_SOUNDS)
	@echo "===================================="
	@echo "Finished building resources jar file"
	@echo "===================================="

iView.jar: JUConManifest juconclasses Makefile
	$(JAR) cmf JUConManifest $@ \
		com/nextone/JUCon/iEdge500/*.class \
		com/nextone/JUCon/DhcpServer/*.class \
		com/nextone/JUCon/iEdge1000/*.class \
		com/nextone/JUCon/iEdge1000/Dhcpserver/*.class \
		com/nextone/JUCon/PingTel/*.class \
		com/nextone/JUCon/VegaStream/*.class
	$(JAR) uf $@ \
		com/nextone/JUCon/iServer/*.class \
		com/nextone/JUCon/iServer/Maint/*.class \
		com/nextone/JUCon/iServer/cfg/*.class
	$(JAR) uf $@ \
		com/nextone/JUCon/*.class
	$(JAR) uf $@ \
		com/nextone/common/*.class
	$(JAR) uf $@ \
		com/nextone/wizard/*.class
	$(JAR) uf $@ \
		com/nextone/database/*.class

	$(JAR) uf $@ \
		com/nextone/util/*.class \
		com/nextone/util/JMask/*.class \
		thirdparty/ftp/*.class \
		thirdparty/tar/*.class 
	@rm -f jce.jar  # delete this in case we built strong earlier
	@cp lib/jhall.jar .
	@cp lib/jaxp.jar .
	@cp lib/crimson.jar .
	@cp lib/xalan.jar .
	@cp lib/activation.jar .
	@cp lib/log4j.jar .
	@cp lib/regexp-1.2.jar .

	@echo "====================================="
	@echo "Finished building main iview jar file"
	@echo "====================================="

juconclasses:	$(FTP_CLASSES) \
		$(TAR_CLASSES)\
		$(IVIEW_UTIL_CLASSES) \
		$(IVIEW_COMMON_CLASSES) \
		$(IVIEW_CLASSES) \
		$(IVIEW_DHCP_SERVER_CLASSES) \
		$(IVIEW_IEDGE500_CLASSES) \
		$(IVIEW_IEDGE1000_CLASSES) \
		$(IVIEW_PINGTEL_CLASSES) \
		$(IVIEW_VEGASTREAM_CLASSES) \
		$(IVIEW_DHCP_SERVER_1000_CLASSES) \
		$(IVIEW_ISERVER_CLASSES) \
		$(IVIEW_WIZARD_CLASSES) \
		$(IVIEW_DATABASE_CLASSES)

cleaniview cleanjcon: .neverbuilt
	rm -f com/nextone/JUCon/*.class
	rm -f com/nextone/JUCon/DhcpServer/*.class
	rm -f com/nextone/JUCon/iEdge500/*.class
	rm -f com/nextone/JUCon/iEdge1000/*.class
	rm -f com/nextone/JUCon/iEdge1000/Dhcpserver/*.class
	rm -f com/nextone/JUCon/iServer/*.class
	rm -f com/nextone/JUCon/iServer/Maint/*.class
	rm -f com/nextone/JUCon/iServer/cfg/*.class
	rm -f com/nextone/JUCon/PingTel/*.class
	rm -f com/nextone/JUCon/VegaStream/*.class
	rm -f com/nextone/wizard/*.class
	rm -f com/nextone/database/*.class
	rm -f com/nextone/common/*.class

################################################
## iFac related rules
################################################
fac: ientry ifac

ientry: iEntry.jar

jFac jfac ifac iFac: iFac.jar

iFac.jar: JFacManifest ifacclasses ifac.mk Makefile jdbc-mysql.jar
	$(JAR) cmf JFacManifest $@ \
		com/nextone/util/*.class \
		com/nextone/util/JMask/*.class \
		com/nextone/common/*.class \
		com/nextone/JFac/*.class
	cd thirdparty; $(JAR) uf ../$@ \
		org/gjt/mm/mysql/*.class \
		org/gjt/mm/mysql/jdbc2/*.class
	$(JAR) uf0 $@ \
		$(IFAC_IMAGES) \
		$(IFAC_SOUNDS)

iEntry.jar: IEntryManifest ientryclasses Makefile jdbc-mysql.jar
	$(JAR) cmf IEntryManifest $@ \
		com/nextone/util/*.class \
		com/nextone/util/JMask/*.class \
		com/nextone/common/*.class \
		com/nextone/JFac/*.class
	cd thirdparty; $(JAR) uf ../$@ \
		org/gjt/mm/mysql/*.class \
		org/gjt/mm/mysql/jdbc2/*.class
	$(JAR) uf0 $@ \
		$(IENTRY_IMAGES) \
		$(IENTRY_SOUNDS)

ifacclasses: $(IFAC_UTIL_CLASSES) $(IFAC_COMMON_CLASSES) $(IFAC_CLASSES)

ientryclasses: $(IENTRY_UTIL_CLASSES) $(IENTRY_COMMON_CLASSES) $(IENTRY_CLASSES)

jdbc-mysql.jar: Makefile
	cd thirdparty; make -f Makefile.mm all

cleanjfac cleanifac cleanientry: .neverbuilt
	rm -f com/nextone/JFac/*.class

cleanmysql: .neverbuilt
	cd thirdparty; make -f Makefile.mm clean

################################################
## jServer related rules
################################################
jServer jserver: install


## check if we have the 1.4 or greater version of JDK
checkversion:
	@(JVERSION=`$(JDKPATH)/java -version 2>&1 | grep "java version" | cut -d\" -f2 | cut -d\_ -f1`; \
	if [ `echo $$JVERSION | cut -d. -f2` -lt 4 ]; then \
		echo "--------------------------------------------"; \
		echo "Need JDK version 1.4 or higher for compiling"; \
		echo "(found version $$JVERSION in $(JDKPATH))"; \
		echo "--------------------------------------------"; \
		exit -1; \
	else \
		echo "Found JDK version $$JVERSION"; \
	fi )

install-indep: jServer.jar
#	$(JAVAH) -o AutoDownload.h -jni com.nextone.JServer.AutoDownload
	$(JAVAH) -o BridgeServer.h -jni com.nextone.JServer.BridgeServer
	$(JAVAH) -o ExtractVersion.h -jni com.nextone.JServer.JServerMain
	$(JAVAH) -o ProcessManagerClient.h -jni com.nextone.JServer.ProcessManagerClient
	$(JAVAH) -o JServer.h -jni com.nextone.JServer.JServer
	$(CP) jServer.jar $(BINDIR)
	$(CP) $(BASE)/java/lib/jaxp.jar $(BINDIR)
	$(CP) $(BASE)/java/lib/crimson.jar $(BINDIR)
	$(CP) $(BASE)/java/lib/xalan.jar $(BINDIR)
	$(CP) $(BASE)/java/lib/activation.jar $(BINDIR)


lib install: checkversion install-indep bridgeserver

bridgeserver: ExtractVersion.c BridgeServerImpl.c iserverconfig.c utils.c capabilities.c
	@echo "Building BridgeServer shared library"
	$(CC) -Wall -c -L. $(CFLAGS) -I. -I$(BASE)/include -I$(JDKINCPATH) -I$(JDKINCPATH)/$(PLATFORM) -I$(TAVLDIR) -I$(AVLDIR) -I. ../ExtractVersion.c
	$(CC) -Wall -c -L. $(CFLAGS) -I. -I$(BASE)/include -I$(BASE)/cli -I$(BASE)/slocks -I$(JDKINCPATH) -I$(JDKINCPATH)/$(PLATFORM) $(JAVALOCALINCLUDE) -I$(TAVLDIR) -I$(AVLDIR) -I. ../utils.c
	$(CC) -Wall -c -L. -I. -I$(BASE)/include -I$(BASE)/cli -I$(BASE)/slocks -I$(JDKINCPATH) -I$(JDKINCPATH)/$(PLATFORM) $(JAVALOCALINCLUDE) -I$(TAVLDIR) -I$(AVLDIR) -I$(BASE)/ls/include -I$(BASE)/fce/include -I. $(CFLAGS) ../iserverconfig.c
	$(CC) -Wall -c -L. -I. -I$(BASE)/include -I$(BASE)/cli -I$(BASE)/slocks -I$(JDKINCPATH) -I$(JDKINCPATH)/$(PLATFORM) $(JAVALOCALINCLUDE) -I$(TAVLDIR) -I$(AVLDIR) -I. $(CFLAGS) ../BridgeServerImpl.c
	$(CC) -Wall -c -L. -I. -I$(BASE)/include -I$(BASE)/cli -I$(BASE)/slocks -I$(JDKINCPATH) -I$(JDKINCPATH)/$(PLATFORM) $(JAVALOCALINCLUDE) -I$(TAVLDIR) -I$(AVLDIR) -I. $(CFLAGS) ../capabilities.c
	$(CCSHARED) -L$(DLIB) -L. $(LDFLAGS) -o libBridgeServer.so utils.o ExtractVersion.o BridgeServerImpl.o iserverconfig.o  capabilities.o $(JAVALIBS)
	$(CP) libBridgeServer.so $(BINDIR)

jServer.jar: JServerManifest jserverclasses Makefile jserver.mk
	@echo "Building JAR file"
	$(JAR) cmf ../JServerManifest $@ \
		com/nextone/util/*.class \
		com/nextone/common/*.class \
		com/nextone/JServer/*.class

jserverclasses: $(JSERVER_UTIL_CLASSES) $(JSERVER_COMMON_CLASSES) \
                $(JSERVER_CLASSES) $(JSERVER_DB_CLASSES)

cleanjserver: clean clean-indep
	(cd $(BASE)/cli/$(TARGET); make clean)

clean: .neverbuilt clean-indep
	rm -f $(TARGET)/libBridgeServer.so $(TARGET)/libcli.so

clean-indep: .neverbuilt cleanutil cleancommon
	rm -f $(TARGET)/jServer.jar
	@rm -f ./jServer.jar
	rm -f $(TARGET)/com/nextone/JServer/*.class
	@rm -f ./com/nextone/JServer/*.class
	rm -f $(TARGET)/com/nextone/common/*.class
	@rm -f ./com/nextone/common/*.class
	rm -f $(TARGET)/BridgeServer.h $(TARGET)/ListServer.h $(TARGET)/ExtractVersion.h

################################################
## iportal rules
################################################
DIRLIST=WEB-INF/classes \
	WEB-INF/classes/com \
	WEB-INF/classes/com/nextone \
	WEB-INF/classes/com/nextone/iportal \
	WEB-INF/classes/com/nextone/iportal/debug \
	WEB-INF/classes/com/nextone/iportal/ibrowse \
	WEB-INF/classes/com/nextone/iportal/ipalm \
	WEB-INF/classes/com/nextone/iportal/admin \
	WEB-INF/classes/com/nextone/util \
	WEB-INF/classes/com/nextone/common\
	WEB-INF/lib\
	components

runpalmtest: .neverbuilt
	$(JDKPATH)/java -cp $(JAVACLASSPATH) com.nextone.iportal.IPalmTest

iportal iPortal: iPortal.war

iPortal.war: $(IPORTAL_CLASSES) $(IPORTAL_IBROWSE_CLASSES) $(IPORTAL_IPALM_CLASSES) $(IPORTAL_DEBUG_CLASSES) $(IPORTAL_UTIL_CLASSES) $(IPORTAL_COMMON_CLASSES) $(IPORTAL_ADMIN_CLASSES) WEB-INF/web.xml WEB-INF/taglib.tld iportal.mk Makefile $(IBROWSE_JSP_FILES) $(IBROWSE_HTML_FILES)
	@echo "Building WAR file"
	@if [ ! -d WEB-INF -a ! -h WEB-INF ]; then \
		ln -s ../WEB-INF .; \
	fi
	@for dir in $(DIRLIST); do \
		if [ ! -d $$dir ]; then \
			mkdir $$dir; \
		fi \
	done
	@cp -fp log4j.properties WEB-INF/classes/
	@cp -fp com/nextone/iportal/*.class WEB-INF/classes/com/nextone/iportal/
	@cp -fp com/nextone/iportal/debug/*.class WEB-INF/classes/com/nextone/iportal/debug/
	@cp -fp com/nextone/iportal/ibrowse/*.class WEB-INF/classes/com/nextone/iportal/ibrowse/
	@cp -fp com/nextone/iportal/ipalm/*.class WEB-INF/classes/com/nextone/iportal/ipalm/
	@cp -fp com/nextone/iportal/admin/*.class WEB-INF/classes/com/nextone/iportal/admin/
	@cp -fp com/nextone/util/*.class WEB-INF/classes/com/nextone/util/
	@cp -fp com/nextone/common/*.class WEB-INF/classes/com/nextone/common/
	@cp -fp ibrowse/components/*.dll components/ 
	@cp	-fp	lib/activation.jar	WEB-INF/lib/
	@cp	-fp	lib/crimson.jar	WEB-INF/lib/
	@cp	-fp	lib/log4j.jar	WEB-INF/lib/
	@cp	-fp	lib/jaxp.jar	WEB-INF/lib/
	@cp	-fp	lib/xalan.jar	WEB-INF/lib/
	@cp	-fp	lib/mail.jar	WEB-INF/lib/
	@cp	-fp	lib/mysql-bin.jar	WEB-INF/lib/

	@if [ -h WEB-INF ]; then \
		($(JAR) cmf ../iPortalManifest $@ WEB-INF/classes; \
		$(JAR) uf $@ WEB-INF/taglib.tld; \
		$(JAR) uf $@ WEB-INF/web.xml; \
		$(JAR) uf $@ WEB-INF/lib; \
		$(JAR) uf $@ WEB-INF/components; \
		cd ../ibrowse; $(JAR) uf ../$(TARGET)/$@ *.html; \
		$(JAR) uf ../$(TARGET)/$@ *.js*; \
		$(JAR) uf ../$(TARGET)/$@ *.css; \
		$(JAR) uf ../$(TARGET)/$@ images/*.gif); \
	else \
		($(JAR) cmf iPortalManifest $@ WEB-INF/classes; \
		$(JAR) uf $@ WEB-INF/taglib.tld; \
		$(JAR) uf $@ WEB-INF/web.xml; \
		$(JAR) uf $@ WEB-INF/lib; \
		$(JAR) uf $@ components; \
		cd ibrowse; $(JAR) uf ../$@ *.html; \
		$(JAR) uf ../$@ *.js*; \
		$(JAR) uf ../$@ *.css; \
		$(JAR) uf ../$@ images/*.gif); \
	fi

iportal-package: iportal versionfile iportal-install.tar

versionfile:
	@if [ -h WEB-INF ]; then \
		grep "public static final String VERSION" ../com/nextone/iportal/Constants.java | cut -d\" -f2 | cut -d" " -f3 | cut -dv -f2 > versionfile; \
	else \
		grep "public static final String VERSION" com/nextone/iportal/Constants.java | cut -d\" -f2 | cut -d" " -f3 | cut -dv -f2 > versionfile; \
	fi

iportal-install.tar: 
	@echo ""
	@echo "To create the iPortal install package, please tar the following files together:"
	@echo "1. iPortal.war"
	@echo "2. versionfile"
	@echo "3. jdk13.sh"
	@echo "4. tomcat.tar"
	@echo "5. patches_<platform>_<solaris version>.tar"
	@echo "6. iportal-install.sh"
	@echo "7. createwebxml.sh"
	@echo "8. globals.sh"
	@echo "9. utils.sh"
	@echo "10. checkpatch.sh"

cleaniportal: .neverbuilt
	rm -f com/nextone/iportal/*.class
	rm -f com/nextone/iportal/debug/*.class
	rm -f com/nextone/iportal/ibrowse/*.class
	rm -f com/nextone/iportal/ipalm/*.class
	rm -rf WEB-INF/classes
	rm -rf WEB-INF/lib
	rm -rf browse/components

################################################
## PingTel Client related rules
################################################

PingTel pingtel: PingTel.jar

## check if we have the 1.1.8 version of JDK
checkPingTelVersion:
  #JDKPATH = c:/jdk1.1.8/bin
	@(JVERSION=`$(JDKPATH)/java -version 2>&1 | grep "java version" | cut -d\" -f2`; \
	if [ "$$JVERSION" != "1.1.8" ]; then \
		echo "----------------------------------"; \
		echo "Need JDK version 1.1.8 for compiling"; \
		echo "(found version $$JVERSION)"; \
		echo "----------------------------------"; \
		exit -1; \
	else \
		echo "Found JDK version $$JVERSION"; \
	fi )

PingTel.jar: $(PINGTEL_CLASSES) $(PINGTEL_PROPS_FILES) Makefile
	$(JAR) cf $@ $(PINGTEL_CLASSES) \
               $(PINGTEL_IMAGES) \
               $(PINGTEL_PROPS_FILES)
cleanPingTel: 
	cd $(TARGET); rm -f com/nextone/clients/PingTel/*.class 


################################################
## sip related rules
################################################
sip: jainsip nextonejainsipimpl

jainsip: jainsip.jar

jainsip.jar: Makefile $(JAIN_API)
	@$(JAR) cf $@ \
		jain/protocol/ip/sip/*.class \
		jain/protocol/ip/sip/address/*.class \
		jain/protocol/ip/sip/header/*.class \
		jain/protocol/ip/sip/message/*.class
	@echo "=============================="
	@echo "Finished building JAIN SIP api"
	@echo "=============================="

dynamicsoftjainsipimpl: Makefile $(DS_IMPL)
	@$(JAR) cf $@.jar \
		com/dynamicsoft/ri/jain/protocol/ip/sip/*.class \
		com/dynamicsoft/ri/jain/protocol/ip/sip/address/*.class \
		com/dynamicsoft/ri/jain/protocol/ip/sip/header/*.class \
		com/dynamicsoft/ri/jain/protocol/ip/sip/message/*.class
	@echo "==================================================================="
	@echo "Finished building Dynamicsoft JAIN SIP api reference implementation"
	@echo "==================================================================="

nextonejainsipimpl: jainsipimpl.jar

jainsipimpl.jar: Makefile $(NEXTONE_IMPL)
	@$(JAR) cf $@ \
		com/nextone/jain/protocol/ip/sip/*.class \
		com/nextone/jain/protocol/ip/sip/address/*.class \
		com/nextone/jain/protocol/ip/sip/header/*.class \
		com/nextone/jain/protocol/ip/sip/message/*.class
	@$(JAR) uf $@ \
		com/nextone/util/*.class
	@echo "====================================================="
	@echo "Finished building Nextone JAIN SIP api implementation"
	@echo "====================================================="

cleansip: .neverbuilt
	rm -f jain/protocol/ip/sip/*.class
	rm -f jain/protocol/ip/sip/address/*.class
	rm -f jain/protocol/ip/sip/header/*.class
	rm -f jain/protocol/ip/sip/message/*.class
	rm -f com/dynamicsoft/ri/jain/protocol/ip/sip/*.class
	rm -f com/dynamicsoft/ri/jain/protocol/ip/sip/address/*.class
	rm -f com/dynamicsoft/ri/jain/protocol/ip/sip/header/*.class
	rm -f com/dynamicsoft/ri/jain/protocol/ip/sip/message/*.class
	rm -f com/nextone/jain/protocol/ip/sip/*.class
	rm -f com/nextone/jain/protocol/ip/sip/address/*.class
	rm -f com/nextone/jain/protocol/ip/sip/header/*.class
	rm -f com/nextone/jain/protocol/ip/sip/message/*.class
	rm -f jainsip.jar jainsipimpl.jar dynamicsoftjainsipimpl.jar
	rm -f com/nextone/util/*.class

################################################
## some shared rules
################################################
cleanftp: .neverbuilt
	rm -f thirdparty/ftp/*.class

cleantar: .neverbuilt
	rm -f thirdparty/tar/*.class

cleanutil: .neverbuilt
	rm -f $(TARGET)/com/nextone/util/*.class
	@rm -f ./com/nextone/util/*.class
	rm -f $(TARGET)/com/nextone/util/JMask/*.class
	@rm -f ./com/nextone/util/JMask/*.class

cleancommon: .neverbuilt
	rm -f $(TARGET)/com/nextone/common/*.class
	@rm -f ./com/nextone/common/*.class

cleanall: cleaniview cleanjfac cleanmysql clean clean-indep cleanutil cleancommon cleanftp cleantools cleaniportal cleantar cleansip
	rm -f iView.jar iViewResources.jar iFac.jar iEntry.jar jServer.jar TrafficGen.jar TrafficSink.jar PasswordFileViewer.jar jhall.jar iPortal.war activation.jar crimson.jar jaxp.jar xalan.jar log4j.jar regexp-1.2.jar

javadocs: .neverbuilt
	$(JAVADOC) $(JAVADOCFLAGS) $(JAVADOC_PACKAGES) 

################################################
## nextone tools
################################################
tools: TrafficGen.jar TrafficSink.jar PasswordFileViewer.jar

PasswordFileViewer.jar:	PasswordFileViewerManifest Makefile \
		com/nextone/tools/PasswordFileViewer.class \
		com/nextone/tools/Utils.class \
		com/nextone/util/Line.class
	(cp -f PasswordFileViewerManifest $(TARGET) > /dev/null 2>&1; \
	cd $(TARGET); $(JAR) cmf PasswordFileViewerManifest $@ \
		com/nextone/tools/PasswordFileViewer*.class \
		com/nextone/tools/PasswordTable*.class \
		com/nextone/tools/Utils*.class \
		com/nextone/JUCon/Constants*.class \
		com/nextone/common/CommonConstants*.class \
		com/nextone/util/WeakEnc*.class \
		com/nextone/JUCon/Unkn*.class \
		com/nextone/JUCon/AbstractUserP*.class \
		com/nextone/JUCon/Password*.class \
		com/nextone/images/header.gif \
		com/nextone/JUCon/Help.gif \
		com/nextone/util/Pop*.class \
		com/nextone/util/File*.class \
		com/nextone/JUCon/User*.class)

TrafficGen.jar:	TrafficGenManifest Makefile \
		com/nextone/tools/TrafficGen.class \
		com/nextone/tools/TrafficSource.class \
		com/nextone/util/JMask/IPAddressField.class \
		com/nextone/util/JMask/NumberField.class \
		com/nextone/util/Line.class
	(cp -f TrafficGenManifest $(TARGET) > /dev/null 2>&1; \
	cd $(TARGET); $(JAR) cmf TrafficGenManifest $@ \
		com/nextone/tools/TrafficGen*.class \
		com/nextone/tools/TrafficSource.class \
		com/nextone/util/JMask/IPAddressField*.class \
		com/nextone/util/JMask/NumberField*.class \
		com/nextone/util/Line*.class)

TrafficSink.jar:  TrafficSinkManifest Makefile \
		com/nextone/tools/TrafficSink.class \
		com/nextone/tools/TrafficSource.class \
		com/nextone/util/JMask/NumberField.class
	(cp TrafficSinkManifest $(TARGET) > /dev/null 2>&1; \
	cd $(TARGET); $(JAR) cmf TrafficSinkManifest $@ \
		com/nextone/tools/TrafficSink*.class \
		com/nextone/tools/TrafficSource.class \
		com/nextone/util/JMask/NumberField*.class)

cleantools: .neverbuilt
	cd $(TARGET); rm -f com/nextone/tools/*.class; \
		rm -f com/nextone/utils/JMask/*.class




.neverbuilt:

