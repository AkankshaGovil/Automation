#!/bin/csh -f

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# !!!!!!!!!! You MUST also update env.sh file !!!!!!!!!!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#
# Determine the target platform, and set some
# environment variables depending on that
#

setenv MC `uname -s`	# SunOS | Linux
setenv BASE ${PWD}

setenv CFLAGS " -fno-builtin-log "
if ( "${MC}" == "SunOS" ) then
	setenv PLATFORM solaris
	setenv CC gcc
	setenv CCSHARED 'ld -r'
	setenv CFLAGS "$CFLAGS -DNETOID_LINUX -D_REENTRANT -DSIP_LINUX -DXML_BYTE_ORDER=12 -DNX_BYTE_ORDER=12"
	setenv LDLIBS "-lpthread -lgdbm -lfl -lposix4 -lsocket -lnsl"
	setenv JDKPATH /usr/jdk1.4/bin
	setenv JDKINCPATH /usr/jdk1.4/include

	#SFIO definitions moved to platform specific conditional block
    #use sfio library (non-mt) to work around the limitation with maximum 
    #open file descriptors on solaris
	#define these to nothing, if they trouble, or to their non-mt/mt versions
	# 1) no sfio
	#setenv SFIOLIBS ""
	#setenv SFIOINCLUDE ""
	# 2) non-mt version of sfio
	setenv SFIOLIBS "-lstdio -lsfio"
	setenv SFIOINCLUDE ""
	# 3) mt version
	#setenv SFIOLIBS "-lstdio-mt -lsfio-mt"
	#setenv SFIOINCLUDE "-I/usr/local/include/sfio"

	setenv TARGET `arch`
	if ( "${TARGET}" == "sun4" ) then
		# Solaris Sparc
		setenv CFLAGS "$CFLAGS -DXML_BYTE_ORDER=21 -DNX_BYTE_ORDER=21 -fPIC"
	else
		# Solaris Intel
		setenv CFLAGS "$CFLAGS -DXML_BYTE_ORDER=12 -DNX_BYTE_ORDER=12 -DI86PC"
	endif

	setenv NATIVE_ADDR_BITS `isainfo -b`
else if ( "${MC}" == "Linux" ) then
	setenv PLATFORM linux
	setenv CC gcc
	setenv CCSHARED 'gcc -shared -Wall'
	setenv CFLAGS "$CFLAGS -DNETOID_LINUX -D_REENTRANT -DUSE_SLOCKS -DSIP_LINUX -DXML_BYTE_ORDER=12 -DNX_BYTE_ORDER=12"
	setenv LDLIBS "-lpthread -lfl -lgdbm"
	setenv JDKPATH /usr/local/jdk1.4/bin
	setenv JDKINCPATH /usr/local/jdk1.4/include

	#SFIO definitions moved to platform specific conditional block
    #do not use sfio libs on linux
	#define these to nothing, if they trouble, or to their non-mt/mt versions
	# 1) no sfio
	setenv SFIOLIBS ""
	setenv SFIOINCLUDE ""
	# 2) non-mt version of sfio
	#setenv SFIOLIBS "-lstdio -lsfio"
	#setenv SFIOINCLUDE ""
	# 3) mt version
	#setenv SFIOLIBS "-lstdio-mt -lsfio-mt"
	#setenv SFIOINCLUDE "-I/usr/local/include/sfio"

	setenv TARGET `arch`
	setenv NATIVE_ADDR_BITS `getconf WORD_BIT`
	setenv LINUX_KERNEL_VER `uname -r | cut -d- -f1 | cut -d. -f1,2`
	if ( "${LINUX_KERNEL_VER}" != "2.6" ) then
		echo "*** UNSUPPORTED Linux Kernel \"$LINUX_KERNEL_VER\". Only Linux Kernel 2.6 is supported."
	endif
else
	echo "*** UNSUPPORTED Platform: \"${MC}\" ***\nPlease update $0 to support \"${MC}\""
endif


unsetenv BUILDTYPE
unsetenv BUILDTYPEANS
unsetenv PROFFLAGS
unsetenv release

echo -n "BuildType=Debug?[y|n|ndl|std|dm|dbm|pd|pstd]"
set BUILDTYPEANS = $<

switch(${BUILDTYPEANS})
case "y":
case "":
	setenv BUILDTYPE "debug"
	breaksw

case "n":
	setenv BUILDTYPE "nodebug"
	breaksw

case "ndl":
	setenv BUILDTYPE "nodebuglog"
	breaksw

case "std":
	setenv BUILDTYPE "standard"
	breaksw

case "dm":
	setenv BUILDTYPE "debug_dmalloc"
	breaksw

case "dbm":
	setenv BUILDTYPE "debug_dbmalloc"
	breaksw

case "pd":
	setenv BUILDTYPE "profile-debug"
	breaksw

case "pstd":
	setenv BUILDTYPE "profile-standard"
	breaksw

default:
	echo "Invalid BuildType"
	exit
endsw


unset DBMALLOCLIB
unset DBMALLOCINC
unset DBMALLOCFLAGS


if ( "${BUILDTYPE}" == "debug_dbmalloc" ) then
# Uncomment this to use DBMALLOC 
setenv DBMALLOCLIB "-ldbmalloc"
setenv DBMALLOCINC "-I$BASE/dbmalloc/include"
setenv DBMALLOCFLAGS "-D_DBMALLOC_"
setenv TARGET "${TARGET}-dbm"
endif

if ( "${BUILDTYPE}" == "debug_dmalloc" ) then
#for dmalloc
    if ( ! -d $BASE/fce/$TARGET-dm/ ) then
        mkdir $BASE/fce/$TARGET-dm/
    endif
    if ( ! -d $BASE/radclient/$TARGET-dm/ ) then
        mkdir $BASE/radclient/$TARGET-dm/
    endif
    if ( ! -f $BASE/lib/$TARGET-dm/ ) then
        cp $BASE/fce/$TARGET/libvnsp.a  $BASE/fce/$TARGET-dm/
    endif
    if ( ! -f $BASE/lib/$TARGET-dm/libradius.a ) then
        cp $BASE/radclient/$TARGET/libradius.a  $BASE/radclient/$TARGET-dm/
    endif
setenv DBMALLOCLIB "-ldmallocth"
setenv DBMALLOCFLAGS "-D_DBMALLOC_ -D_DMALLOC_"
setenv TARGET "${TARGET}-dm"
endif

if ( "$?DBMALLOCFLAGS" ) then
setenv CFLAGS "${CFLAGS} ${DBMALLOCFLAGS}"
endif

if ( "`echo $BUILDTYPE | cut -f1 -d'-'`" == "profile" ) then
    setenv LDFLAGS "-L/usr/lib/libp -L/usr/local/lib"
else
    setenv LDFLAGS "-L/usr/lib -L/usr/local/lib"
endif
setenv GCCLIBNAME `gcc --print-libgcc-file-name`
setenv GCCLIBDIR `dirname $GCCLIBNAME`
setenv LDFLAGS "$LDFLAGS -L$GCCLIBDIR"
echo ldflags=$LDFLAGS
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:.
setenv SERPLEXPATH ${BASE}/bin/${TARGET}/
#H.323 ver 3.0.9
#setenv	H323VER h323-3.0.9
#setenv PROJECT_BASE ${BASE}/${H323VER}/
#setenv H323APPCFLAGS "-DH323v30 -DH323GLOBALLOCKS"
#H.323 ver 4.0
#setenv	H323VER h323-4.0
#H.323 ver 4.0.35
#setenv	H323VER h323-4.0.35
#H.323 ver 4.0.48
setenv	H323VER h323-4.0.48
setenv PROJECT_BASE ${BASE}/${H323VER}/h323
setenv H323APPCFLAGS "-DH323v40"

setenv CURRENT_JDK_DIR /usr/jdk1.4
# set PATH
if ( "$JAVA_HOME" == "" ) then
	if ( ! -d "$CURRENT_JDK_DIR" ) then
#using printf , echo -n is not portable
		printf "Enter Java Home ... "
		set JAVA_HOME_VAR = "$<"
		if ( ! -d "$JAVA_HOME_VAR/bin/java" ) then
		    printf "**********java not found in $JAVA_HOME_VAR. Please install jdk \n\n\n"
		else
			setenv JAVA_HOME $JAVA_HOME_VAR
		endif
	else
		setenv JAVA_HOME $CURRENT_JDK_DIR
	endif
endif

setenv JDKPATH $JAVA_HOME/bin
setenv JDKINCPATH $JAVA_HOME/include
setenv PATH "${JDKPATH}:${PATH}:${SERPLEXPATH}"

#set exuberant ctags EX_CTAG 
setenv EX_CTAG /usr/local/bin/ctags

# setup java
setenv JAVACFLAGS "-server -classpath .:${BASE}/java/lib/jce.jar:${BASE}/java/lib/servlet.jar:${BASE}/java/lib/xerces.jar:${BASE}/java/lib/jaxp.jar:${BASE}/java/lib/crimson.jar:${BASE}/java/lib/xalan.jar:${BASE}/java/lib/activation.jar:${BASE}/java/lib/mail.jar -deprecation -d . -sourcepath .."

#setup for crypto compilation
setenv CFLAGS "${CFLAGS} -DTHIRTY_TWO_BIT"

#setup for gatekeeper conpilation
setenv CFLAGS "${CFLAGS} -DALLOW_ISERVER_H323"

#add posix thread sematics
setenv CFLAGS "${CFLAGS} -D_POSIX_PTHREAD_SEMANTICS"


switch(${BUILDTYPE})
case "debug":
	setenv CFLAGS "${CFLAGS} -ggdb"
	setenv JAVACFLAGS "${JAVACFLAGS} -g"
	echo "Using debug mode"
	breaksw

case "debug_dmalloc":
	setenv CFLAGS "${CFLAGS} -ggdb"
	setenv JAVACFLAGS "${JAVACFLAGS} -g"
	echo "Using debug mode with dmalloc"
	breaksw

case "debug_dbmalloc":
	setenv CFLAGS "${CFLAGS} -ggdb"
	setenv JAVACFLAGS "${JAVACFLAGS} -g"
	echo "Using debug mode with dbmalloc"
	breaksw

case "nodebuglog":
	setenv CFLAGS "${CFLAGS} -ggdb -DNODEBUGLOG -O3"
	setenv JAVACFLAGS "${JAVACFLAGS} -g"
	echo "Using non-debug-log mode"
	breaksw

case "standard":
	setenv CFLAGS "${CFLAGS} -ggdb -O3 -DNOTMRLOG -DNOTHRDLOG -DNOFDLOG -DNORESLOG"
	setenv release yes
	setenv JAVACFLAGS "${JAVACFLAGS} -g"
	echo "Using standard mode"
	breaksw

case "profile-debug":
	setenv CFLAGS "${CFLAGS} -ggdb"
	setenv PROFFLAGS "-pg"
	setenv LDLIBS "${LDLIBS} -ldl"
	setenv JAVACFLAGS "${JAVACFLAGS} -g"
	unsetenv SFIOLIBS
	unsetenv SFIOINCLUDE
	echo "Using profile debug mode"
	breaksw

case "profile-standard":
	setenv CFLAGS "${CFLAGS} -ggdb -O3 -DNOTMRLOG -DNOTHRDLOG -DNOFDLOG -DNORESLOG"
	setenv release yes
	setenv JAVACFLAGS "${JAVACFLAGS} -g:none"
	setenv PROFFLAGS "-pg"
	setenv LDLIBS "${LDLIBS} -ldl"
	unsetenv SFIOLIBS
	unsetenv SFIOINCLUDE
	echo "Using profile-standard mode"
	breaksw

default:
	setenv CFLAGS "${CFLAGS} -s -O3"
	setenv LDFLAGS "${LDFLAGS} -s"
	setenv JAVACFLAGS "${JAVACFLAGS} -g:none"
	echo "Using non-debug mode"
endsw

if ( "${CVSROOT}" == "" ) then
        setenv CVSROOT /netoids/repos
        echo "CVSROOT not set. Initializing it to \"${CVSROOT}\""
else
        echo "CVSROOT is        : "\"${CVSROOT}\"
endif

# test that the required variables have been set
if ( "${NATIVE_ADDR_BITS}" == "" ) then
	setenv NATIVE_ADDR_BITS "32"
	echo "NATIVE_ADDR_BITS not set. Initializing it to \"${NATIVE_ADDR_BITS}\""
else
        echo "NATIVE_ADDR_BITS is : "\"${NATIVE_ADDR_BITS}\"
endif

# setup correct permissions for files in utils directory
if ( ! -x $BASE/utils/links.sh ) then
	chmod +x $BASE/utils/links.sh
endif

setenv PERLSUPPORTEDVERSION "5.6.0"
setenv PERLVERSION `perl -e 'printf "%vd\n", $^V;' `

switch(${PERLVERSION})
case "5.8.3":
        setenv PERLADJVERSION "5.6.1"
        setenv PERLOLDVERSION "5.6.0"
        breaksw

case "5.8.0":
        setenv PERLADJVERSION "5.6.1"
        setenv PERLOLDVERSION "5.6.0"
        breaksw

case "5.6.0":
        setenv PERLADJVERSION "5.6.1"
        setenv PERLOLDVERSION "5.005"
        breaksw

case "5.6.1":
        setenv PERLADJVERSION "5.6.0"
        setenv PERLOLDVERSION "5.005"
        breaksw

default:
        echo "Unknown or unsupported version of Perl."
        echo "Please upgrade to Perl version ${PERLSUPPORTEDVERSION}"
        breaksw
endsw

setenv MKMK_VER `perl -e 'use ExtUtils::MakeMaker; if ($ExtUtils::MakeMaker::VERSION < 5.49) { print "old"; } else { print "new"; }' `


# change window title
echo "]2;`pwd`"

