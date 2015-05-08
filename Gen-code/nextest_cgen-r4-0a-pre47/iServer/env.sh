#!/bin/sh

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# !!!!!!!!!! You MUST also update env.csh file !!!!!!!!!!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#
# Determine the target platform, and set some
# environment variables depending on that
#

MC=`uname -s`	# SunOS | Linux
BASE=$PWD
# log.h defines function log which conflicts with inbuilt function log hence this flag.

CFLAGS=" -fno-builtin-log "
if [ "$MC" = "SunOS" ]; then
	PLATFORM=solaris
	CC=gcc
	CCSHARED='ld -r'
	CFLAGS="$CFLAGS -DSUNOS -D_REENTRANT -DSIP_SOLARIS -DANSI_PROTO -DSIP_BY_REFERENCE"
	LDLIBS="-lpthread -lgdbm -lfl -lposix4 -lsocket -lnsl"
	JDKPATH=/usr/jdk1.4/bin
	JDKINCPATH=/usr/jdk1.4/include
	#SFIO definitions moved to platform specific conditional block
	#use sfio library (non-mt) to work around the limitation with maximum 
	#open file descriptors on solaris
	#define these to nothing, if they trouble, or to their non-mt/mt versions
	# 1) no sfio
	#SFIOLIBS=""
	#SFIOINCLUDE=""
	# 2) non-mt version of sfio
	SFIOLIBS="-lstdio -lsfio"
	SFIOINCLUDE=""
	# 3) mt version
	#SFIOLIBS="-lstdio-mt -lsfio-mt"
	#SFIOINCLUDE="-I/usr/local/include/sfio"

	TARGET=`arch`
	if [ "$TARGET" = "sun4" ]; then
		# Solaris Sparc
		CFLAGS="$CFLAGS -DXML_BYTE_ORDER=21 -DNX_BYTE_ORDER=21 -fPIC"
	else
		# Solaris Intel
		CFLAGS="$CFLAGS -DXML_BYTE_ORDER=12 -DNX_BYTE_ORDER=12 -DI86PC"
	fi

	NATIVE_ADDR_BITS=`isainfo -b`
elif [ "$MC" = "Linux" ]; then
	PLATFORM=linux
	CC=gcc
	CCSHARED='gcc -shared -Wall'
	CFLAGS="$CFLAGS -DNETOID_LINUX -D_REENTRANT -DSIP_SOLARIS -DXML_BYTE_ORDER=12 -DNX_BYTE_ORDER=12"
	LDLIBS="-lpthread -lfl -lgdbm"
	JDKPATH=/usr/local/jdk1.4/bin
	JDKINCPATH=/usr/local/jdk1.4/include
	#SFIO definitions moved to platform specific conditional block
	#do not use sfio libs on linux
	#define these to nothing, if they trouble, or to their non-mt/mt versions
	# 1) no sfio
	SFIOLIBS=""
	SFIOINCLUDE=""
	# 2) non-mt version of sfio
	#SFIOLIBS="-lstdio -lsfio"
	#SFIOINCLUDE=""
	# 3) mt version
	#SFIOLIBS="-lstdio-mt -lsfio-mt"
	#SFIOINCLUDE="-I/usr/local/include/sfio"

	TARGET=`arch`
	NATIVE_ADDR_BITS=`getconf WORD_BIT`
	LINUX_KERNEL_VER=`uname -r | cut -d- -f1 | cut -d. -f1,2`
	if [ "$LINUX_KERNEL_VER" != "2.6" ]; then
		echo "*** UNSUPPORTED Linux Kernel \"$LINUX_KERNEL_VER\". Only Linux Kernel 2.6 is supported."
	fi
else
	echo "*** UNSUPPORTED Platform: \"$MC\" ***\nPlease update $0 to support \"$MC\""
fi

unset BUILDTYPE
unset BUILDTYPEANS
unset PROFFLAGS
unset release

echo -n "BuildType=Debug?[y|n|ndl|std|dm|dbm|pd|pstd]"
read BUILDTYPEANS

: ${BUILDTYPEANS:="y"}
: ${BUILDTYPE:="debug"}

if [ "$BUILDTYPEANS" = "n" ]; then
	BUILDTYPE="nodebug"
fi

if [ "$BUILDTYPEANS" = "ndl" ]; then
	BUILDTYPE="nodebuglog"
fi

if [ "$BUILDTYPEANS" = "std" ]; then
	BUILDTYPE="standard"
fi

if [ "$BUILDTYPEANS" = "dm" ]; then
	BUILDTYPE="debug_dmalloc"
fi

if [ "$BUILDTYPEANS" = "dbm" ]; then
	BUILDTYPE="debug_dbmalloc"
fi

if [ "$BUILDTYPEANS" = "pd" ]; then
	BUILDTYPE="profile-debug"
fi

if [ "$BUILDTYPEANS" = "pstd" ]; then
	BUILDTYPE="profile-standard"
fi

unset DBMALLOCLIB
unset DBMALLOCINC
unset DBMALLOCFLAGS


if [ "$BUILDTYPE" = "debug_dbmalloc" ]; then
# Uncomment this to use DBMALLOC 
DBMALLOCLIB="-ldbmalloc"
DBMALLOCINC="-I$BASE/dbmalloc/include"
DBMALLOCFLAGS="-D_DBMALLOC_"
TARGET=$TARGET-dbm
fi

if [ "$BUILDTYPE" = "debug_dmalloc" ]; then
#for dmalloc
	if [ ! -d $BASE/fce/$TARGET-dm/ ]; then
		mkdir $BASE/fce/$TARGET-dm/
	fi
	if [ ! -d $BASE/radclient/$TARGET-dm/ ]; then
		mkdir $BASE/radclient/$TARGET-dm/
	fi
	if [ ! -f $BASE/lib/$TARGET-dm/ ]; then
		cp $BASE/fce/$TARGET/libvnsp.a  $BASE/fce/$TARGET-dm/
	fi
	if [ ! -f $BASE/lib/$TARGET-dm/libradius.a ]; then
		cp $BASE/radclient/$TARGET/libradius.a  $BASE/radclient/$TARGET-dm/
	fi
DBMALLOCLIB="-ldmallocth"
DBMALLOCFLAGS="-D_DBMALLOC_ -D_DMALLOC_"
TARGET=$TARGET-dm
fi

if [ ! -z "$DBMALLOCFLAGS" ]; then
	CFLAGS="$CFLAGS $DBMALLOCFLAGS"
fi

if [ "`echo $BUILDTYPE | cut -f1 -d'-'`" = "profile" ]; then
	LDFLAGS="-L/usr/lib/libp -L/usr/local/lib"
else
	LDFLAGS="-L/usr/lib -L/usr/local/lib"
fi
GCCLIBNAME=`gcc --print-libgcc-file-name`
GCCLIBDIR=`dirname $GCCLIBNAME`
LDFLAGS="$LDFLAGS -L$GCCLIBDIR"
echo ldflags=$LDFLAGS
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
SERPLEXPATH=$BASE/bin/$TARGET/
#H.323 ver 3.0.9
#H323VER=h323-3.0.9
#PROJECT_BASE=$BASE/$H323VER/
#H323APPCFLAGS="-DH323v30 -DH323GLOBALLOCKS"
#H.323 ver 4.0
#H323VER=h323-4.0
#4.0.35
#H323VER=h323-4.0.35
#4.0.48 
H323VER=h323-4.0.48
PROJECT_BASE=$BASE/$H323VER/h323
H323APPCFLAGS="-DH323v40"

# set PATH
CURRENT_JDK_DIR=/usr/jdk1.4
if [ ! -n "$JAVA_HOME" ]; then
	if [ ! -d $CURRENT_JDK_DIR ]; then
#using printf , echo -n is not portable
    	printf "Enter Java Home ... "
	    read JAVA_HOME_VAR
	    if [ ! -f "$JAVA_HOME_VAR/bin/java" ]; then
		printf "**********java not found in $JAVA_HOME_VAR. Please install jdk \n\n\n"
	    else
		JAVA_HOME=$JAVA_HOME_VAR
    	fi
	else
	JAVA_HOME=$CURRENT_JDK_DIR
	fi
fi

JDKPATH=$JAVA_HOME/bin
JDKINCPATH=$JAVA_HOME/include



PATH=$JDKPATH:$PATH:$SERPLEXPATH

# set Exuberant ctags EX_CTAG
EX_CTAG=/usr/local/bin/ctags

# setup java
JAVACFLAGS="-server -classpath .:$BASE/java/lib/jce.jar:$BASE/java/lib/servlet.jar:$BASE/java/lib/xerces.jar:$BASE/java/lib/jaxp.jar:$BASE/java/lib/crimson.jar:$BASE/java/lib/xalan.jar:$BASE/java/lib/activation.jar:$BASE/java/lib/mail.jar -deprecation -d . -sourcepath .."

#setup for crypto compilation
CFLAGS="$CFLAGS -DTHIRTY_TWO_BIT"

#setup for gatekeeper conpilation
CFLAGS="$CFLAGS -DALLOW_ISERVER_H323"

#add posix thread sematics
CFLAGS="$CFLAGS -D_POSIX_PTHREAD_SEMANTICS"



if [ "$BUILDTYPE" = "debug" ]; then
	CFLAGS="$CFLAGS -ggdb"
	JAVACFLAGS="$JAVACFLAGS -g"
	echo "Using debug mode"


elif [ "$BUILDTYPE" = "debug_dbmalloc" ]; then
	CFLAGS="$CFLAGS -ggdb"
	JAVACFLAGS="$JAVACFLAGS -g"
	echo "Using debug mode with dbmalloc"


elif [ "$BUILDTYPE" = "debug_dmalloc" ]; then
	CFLAGS="$CFLAGS -ggdb"
	JAVACFLAGS="$JAVACFLAGS -g"
	echo "Using debug mode with dmalloc"


elif [ "$BUILDTYPE" = "nodebuglog" ]; then
	CFLAGS="$CFLAGS -ggdb -DNODEBUGLOG -O3"
	JAVACFLAGS="$JAVACFLAGS -g"
	echo "Using non-debug-log mode"


elif [ "$BUILDTYPE" = "standard" ]; then
	CFLAGS="$CFLAGS -ggdb -O3 -DNOTMRLOG -DNOTHRDLOG -DNOFDLOG -DNORESLOG"
	export release=yes
	JAVACFLAGS="$JAVACFLAGS -g"
	echo "Using standard mode"


elif [ "$BUILDTYPE" = "profile-debug" ]; then
	CFLAGS="$CFLAGS -ggdb"
	PROFFLAGS=" -pg"
	LDLIBS="$LDLIBS -ldl"
	JAVACFLAGS="$JAVACFLAGS -g"
	unset SFIOLIBS
	unset SFIOINCLUDE
	echo "Using profile-debug mode"


elif [ "$BUILDTYPE" = "profile-standard" ]; then
	CFLAGS="$CFLAGS -ggdb -O3 -DNOTMRLOG -DNOTHRDLOG -DNOFDLOG -DNORESLOG"
	export release=yes
	JAVACFLAGS="$JAVACFLAGS -g"
	PROFFLAGS=" -pg"
	LDLIBS="$LDLIBS -ldl"
	unset SFIOLIBS
	unset SFIOINCLUDE
	echo "Using profile-standard mode"


else
	CFLAGS="$CFLAGS -s -O3"
	LDFLAGS="$LDFLAGS -s"
	JAVACFLAGS="$JAVACFLAGS -g:none"
	echo "Using non-debug mode"
fi

if [ "$CVSROOT" = "" ]; then
        CVSROOT=/netoids/repos
        echo "CVSROOT not set. Initializing it to \"${CVSROOT}\""
else
        echo "CVSROOT is        : \"${CVSROOT}\""
fi

# test that the required variables have been set
if [ "$NATIVE_ADDR_BITS" = "" ]; then
	NATIVE_ADDR_BITS="32"
	echo "NATIVE_ADDR_BITS not set. Initializing it to \"${NATIVE_ADDR_BITS}\""
else
        echo "NATIVE_ADDR_BITS is : "\"${NATIVE_ADDR_BITS}\"
fi

# setup correct permissions for files in utils directory
if [ ! -x $BASE/utils/links.sh ]; then
	chmod +x $BASE/utils/links.sh
fi
if [ ! -x $BASE/osd/configure ]; then
	chmod +x $BASE/osd/configure
fi



PERLSUPPORTEDVERSION="5.6.0"
PERLVERSION=`perl -e 'printf "%vd\n", $^V;' `
if [ "$PERLVERSION" = "5.8.3" ]; then
	PERLADJVERSION="5.6.1"
	PERLOLDVERSION="5.6.0"


elif [ "$PERLVERSION" = "5.8.0" ]; then
	PERLADJVERSION="5.6.1"
	PERLOLDVERSION="5.6.0"


elif [ "$PERLVERSION" = "5.6.0" ]; then
	PERLADJVERSION="5.6.1"
	PERLOLDVERSION="5.005"


elif [ "$PERLVERSION" = "5.6.1" ]; then
	PERLADJVERSION="5.6.0"
	PERLOLDVERSION="5.005"


else
	echo "Unknown or unsupported version of Perl."
	echo "Please upgrade to Perl version ${PERLSUPPORTEDVERSION}"
fi


MKMK_VER=`perl -e 'use ExtUtils::MakeMaker; if ($ExtUtils::MakeMaker::VERSION < 5.49) { print "old"; } else { print "new"; }' `


#change window title
echo "]2;`pwd`"

export BASE
export PROJECT_BASE
export CC
export CFLAGS
export PROFFLAGS
export LDLIBS
export TARGET
export JAVA_HOME
export JDKPATH
export JDKINCPATH
export PLATFORM
export CCSHARED
export LDFLAGS
export LD_LIBRARY_PATH
export SERPLEXPATH
export PATH
export JAVACFLAGS
export BUILDTYPE
export CVSROOT
export SFIOLIBS SFIOINCLUDE
export EX_CTAG
export PERLVERSION
export PERLADJVERSION
export PERLOLDVERSION
export MKMK_VER
export H323VER
export H323APPCFLAGS
export DBMALLOCINC DBMALLOCLIB DBMALLOCFLAGS
export NATIVE_ADDR_BITS
