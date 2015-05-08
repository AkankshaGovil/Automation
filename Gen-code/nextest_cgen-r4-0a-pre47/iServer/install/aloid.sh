#!/bin/sh

#####################################################
#####################################################
#######  The aloid shell script.
#######
#####################################################
#####################################################

## default
WHOAMI=`/usr/ucb/whoami`
PRODUCT="iserver"
ALOID_VERSION=2.0
ALOID_FILEDIR="$BASE/bin/$TARGET"
ALOID_README="$BASE/bin/README-${ALOID_VERSION}"
#ALOID_FILEDIR="/export/home/`whoami`/netoids/src/bin"
#ALOID_README="/export/home/`whoami`/netoids/src/bin/README-${ALOID_VERSION}"
CP=/bin/cp
ALOID_INDEX=".aloidindex"
MEDIAFILE="${PRODUCT}install.tar"
SYSVINITFILE="/etc/rc.d/init.d/nextoneIserver"
LICENSE_FILE="iserverlc.xml"
TMP_LICENSE_FILE="iserver.lc"
STARTDIR=`pwd`

## Admin stuff
ALOID_ADMINDEX=".admindex"
ADMMEDIAFILE="${PRODUCT}adm-install.tar"
ALOID_ADMFILEDIR="$BASE/utils"
TMPBASEDIR="/tmp"

## Codemap files
CODEMAPTXTFILES="codemap_[0-9][0-9][0-9].txt"
CODEMAPDATFILES="codemap_[0-9][0-9][0-9].dat"

PackAloidFiles ()
{
	$ECHON "Enter directory where files are present [$ALOID_FILEDIR]: "
	read resp

	if [ -z "$resp" ]; then
		resp=$ALOID_FILEDIR
	fi

	ALOID_FILEDIR=$resp

## Reset default again.
ALOID_README="/export/home/$WHOAMI/netoids/src/bin/README-${ALOID_VERSION}"

	$ECHON "Enter location of README file [$ALOID_README]: "
	read resp

	if [ -z "$resp" ]; then
		resp=$ALOID_README
	fi

	ALOID_README=$resp


	## Ask for temporary directory
	$ECHON "Enter temporary directory to use [$TMPBASEDIR]: "
	read resp

	if [ -z "$resp" ]; then
		resp=$TMPBASEDIR
	fi

	TMPBASEDIR=$resp
	TMPBASEDIR=$TMPBASEDIR/$$

	## If not present, then make it...
	if [ ! -d "$TMPBASEDIR" ]; then
		$ECHO "$TMPBASEDIR not present. Creating $TMPBASEDIR."
		mkdir -p $TMPBASEDIR
	fi

	## Create temporary directory
	TMPDIRNAME="$TMPBASEDIR/${PRODUCT}-${ALOID_VERSION}"

	mkdir -p $TMPDIRNAME

	
	## Copy the file(s)

	for i in `cat aloid_filelist`
	do
		$CP -p $ALOID_FILEDIR/$i $TMPDIRNAME
	done
	
	## Copy the codemap files
	$CP -p $ALOID_FILEDIR/$CODEMAPTXTFILES $TMPDIRNAME
	$CP -p $ALOID_FILEDIR/$CODEMAPDATFILES $TMPDIRNAME

	## Copy the README file also

	## Create the index file
	echo "Generating index file"
	## Save off current directory
	CURRENT_DIR=`pwd`
	## Move to the tmp directory
	cd $TMPBASEDIR
	cat > $ALOID_INDEX << E_EOF
$ALOID_VERSION
E_EOF
	cat > $TMP_LICENSE_FILE << E_EOF
iServer 2.0 `date +%m-%d-%Y` 100 100 unbound 100
Features H323 SIP FCE
E_EOF
	$ALOID_FILEDIR/lgen $TMP_LICENSE_FILE
	$CP -p $TMP_LICENSE_FILE $LICENSE_FILE
	$ALOID_FILEDIR/lconv $LICENSE_FILE 
	
	## Verify
	echo "Contents of $TMPDIRNAME"
	/bin/ls -a $TMPDIRNAME


	## Go one level up
	cd $TMPBASEDIR

	## Enter media file
	$ECHON "Enter media or filename [$MEDIAFILE]: "

	##
	$ECHON "Making $MEDIAFILE..."
	tar cvf $MEDIAFILE ${PRODUCT}-${ALOID_VERSION} $ALOID_INDEX $LICENSE_FILE
	
	##
	## Cleanup
	/bin/rm -rf $TMPDIRNAME $ALOID_INDEX
	mv $TMPBASEDIR/$MEDIAFILE $BASE/$TARGET.$MEDIAFILE

	$ECHO "..done"

	echo ""
	echo "The packed file is ${BOLD}$MEDIAFILE${OFFBOLD}"
	echo ""
	##
	##
	echo ""
	echo "Packaging successful !!"
	echo ""
	cd $CURRENT_DIR;
	/bin/rm -rf $TMPBASEDIR
}


##
## 
##
PackAdminFiles ()
{
	$ECHON "Enter directory where files are present [$ALOID_ADMFILEDIR]: "
	read resp

	if [ -z "$resp" ]; then
		resp=$ALOID_ADMFILEDIR
	fi

	ALOID_ADMFILEDIR=$resp

	## Create temporary directory
	TMPADMDIRNAME="/tmp/${PRODUCT}-${ALOID_ADMVERSION}"

	mkdir -p $TMPADMDIRNAME

	
	## Copy the file(s)

	CURDIR=`pwd`

	cd $ALOID_ADMFILEDIR

	## Doing this so we can copy absolute filenames..
	##
	for i in `cat $CURDIR/admfiles`
	do
		$CP -p $i $TMPADMDIRNAME
	done

	## back to where we were.
	cd $CURDIR

	## Create the index file
	echo "Generating index file"
	## Save off current directory
	CURRENT_DIR=`pwd`
	## Move to the tmp directory
	cd /tmp
	cat > $ALOID_ADMINDEX << E_EOF
$ALOID_ADMVERSION
E_EOF
	
	## Verify
	echo "Contents of $TMPADMDIRNAME"
	/bin/ls -a $TMPADMDIRNAME


	## Go one level up
	cd /tmp

	## Enter media file
	$ECHON "Enter media or filename [$ADMMEDIAFILE]: "

	##
	$ECHON "Making $ADMMEDIAFILE..."
	tar cvf $ADMMEDIAFILE ${PRODUCT}-${ALOID_ADMVERSION} $ALOID_ADMINDEX 
	
	##
	## Cleanup
	/bin/rm -rf $TMPADMDIRNAME $ALOID_ADMINDEX
	mv /tmp/$ADMMEDIAFILE $BASE/$TARGET.$ADMMEDIAFILE

	$ECHO "..done"

	echo ""
	echo "The packed file is ${BOLD}$ADMMEDIAFILE${OFFBOLD}"
	echo ""
	##
	##
	echo ""
	echo "Admin Packaging successful !!"
	echo ""
	cd $CURRENT_DIR;
}


##
##
##
GatherAdminReleasorInfo ()
{

	$ECHON "Enter ${BOLD}Admin Version Number${OFFBOLD} [$ALOID_ADMVERSION] "
	read resp

	if [ -z "$resp" ]; then
		resp=$ALOID_ADMVERSION
	fi

	ALOID_ADMVERSION=$resp
}


##
##
##
GatherReleasorInfo ()
{

	$ECHON "Enter ${BOLD}Version Number${OFFBOLD} [$ALOID_VERSION] "
	read resp

	if [ -z "$resp" ]; then
		resp=$ALOID_VERSION
	fi

	ALOID_VERSION=$resp
}

##
##
PackageAloid ()
{
	echo "Packaging Aloid-iServer-Main..."

	GatherReleasorInfo

	PackAloidFiles
}

##
## Package iServer Administration Package
##
PackageAdmin ()
{
	echo "Packaging iServer Admin..."

	GatherAdminReleasorInfo

	PackAdminFiles
	PackageRsync
}

#
#  Package rsync
#
PackageRsync ()
{
	echo "Packaging Rsync Package..."

	RSYNCPKGDIR=rsyncpkg;
	RSYNCTMPF=/tmp/rsync$$.tar;

	CURRDIR=pwd;
	cd $ALOID_ADMFILEDIR;

	RSYNCFILES=`tar -cf $RSYNCTMPF rsyncpkg; tar -tf $RSYNCTMPF | grep -v 'CVS' `
	tar -uvf $BASE/$TARGET.$ADMMEDIAFILE $RSYNCFILES 2>/dev/null;

	/bin/rm -f $RSYNCTMPF
	echo "..done"
}
