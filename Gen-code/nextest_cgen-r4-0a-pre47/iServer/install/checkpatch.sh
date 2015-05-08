#!/bin/sh
#
# checkpatch.sh  - library routines to check and install any required JRE
#                  patches on solaris
#

#
# any other library inclusions
#
DIR=${1:-.}
. $DIR/globals.sh
. $DIR/utils.sh

PLAT=`uname -m`
if [ "$PLAT" = "i86pc" ]; then
    PATCH_FILE56=patches_intel_5.6.tar
    PATCH_FILE57=patches_intel_5.7.tar
    PATCH_FILE58=patches_intel_5.8.tar
else
    PATCH_FILE56=patches_sparc_5.6.tar
    PATCH_FILE57=patches_sparc_5.7.tar
    PATCH_FILE58=patches_sparc_5.8.tar
fi

SHOWREV=showrev
UNCOMPRESS=uncompress
PATCHADD=/usr/sbin/patchadd

#
# Check to see what patches we have and what else we need
#
CheckRequiredPatches ()
{
	NEED=""
	AVAIL=""
	LESSER=""
	for patch in $REQUIRED_PATCHES
	do
		patch_num=`echo $patch | cut -d- -f1`
		patch_rev=`echo $patch | cut -d- -f2`
		avail=`$SHOWREV -p | grep $patch_num | grep Patch | cut -d: -f2 | grep $patch_num | cut -d" " -f2`
		#
		# Is any revision of the patch already installed?
		#
		if [ ! -z "$avail" ]; then
			lesser=""
			for all in $avail
			do
				pr=`echo $all | cut -d- -f2`
				if [ $pr -ge $patch_rev ]; then
					AVAIL="$AVAIL $all"
					break
				else
					lesser="$lesser $all"
				fi
			done
			#
			# Do we have the right revision
			#
			check=`echo $AVAIL | grep $patch_num`
			if [ -z "$check" ]; then
				NEED="$NEED $patch"
				LESSER="$LESSER $lesser"
			fi
		else
			NEED="$NEED $patch"
		fi
	done
	echo
	echo "Required Patches:"
	echo "$REQUIRED_PATCHES"
	echo
	echo "Currently Installed:"
	echo "    Meets Required Revision: $AVAIL"
	echo "    Does not need Required Revision: $LESSER"
	echo "Needed: $NEED"
	echo

	#
	# Do we need any patches at all?
	#
	need=`echo $NEED | tr -d ' '`
	if [ -z "$need" ]; then
		echo "All required patches are already installed on this machine"
		Pause
		return 0
	fi
	Pause
	return 1
}

#
# Install the required patches
#
InstallPatches ()
{
	TOINSTALL=`echo $NEED | tr -s ' ' ' '`
	AreYouSure "Do you want to install the required patches?" "y"
	if [ $? -ne 0 ]; then
		#
		# untar the patch file
		#
		echo "Preparing to install patches..."
		curd=`pwd`

		if [ -r $PATCH_FILE ]; then
		    PATCH_FILE=`pwd`/$PATCH_FILE
		else
		    # look in the right dir
		    if [ -r `dirname $curd`/$PATCH_FILE ]; then
			# one level up
			PATCH_FILE=`dirname $curd`/$PATCH_FILE
		    elif [ -r $DIR/$PATCH_FILE ]; then
			# under install dir
			PATCH_FILE=$DIR/$PATCH_FILE
		    else
			echo "Cannot find patch file ($PATCH_FILE)"
			Pause
			return 1
		    fi
		fi

		cd /tmp
		tar xf $PATCH_FILE
		cd $PATCH_DIR

		for patch in $TOINSTALL
		do
			echo
			echo "Installing $patch..."
			if [ ! -r $patch.tar.Z ]; then
				echo "Cannot find patch file ($patch.tar.Z)"
				INCOMPLETE_PATCH="$INCOMPLETE_PATCH $patch"
				continue
			else
				$UNCOMPRESS $patch.tar.Z
				tar xf $patch.tar
				$PATCHADD $patch
				if [ $? -ne 0 ]; then
				    INCOMPLETE_PATCH="$INCOMPLETE_PATCH $patch"
				else
				    NeedReboot $patch
				fi
			fi
#			if [ $REBOOT_NEEDED -eq 1 ]; then
#			    break
#			fi
		done

		#
		# Cleanup
		#
		cd /tmp
		rm -rf $PATCH_DIR
		cd $curd
		echo
		echo "Patch install completed!"
		if [ ! -z "$INCOMPLETE_PATCH" ]; then
		    echo "(Installation of the following patches were not successfull:"
		    echo "    $INCOMPLETE_PATCH"
		    echo " After satisfying the patch requirements, please run"
		    echo " `dirname $PATCH_FILE`/patch.sh to attempt installing them again)"
		fi
		echo
		Pause
	else
		return 1
	fi

	# display reboot needed message
	if [ $REBOOT_NEEDED -eq 1 ]; then
	    echo "One or more of the patches installed requires the system to be rebooted"
	    echo "After the installation is complete, please manually reboot the system"
	    echo "After rebooting, please run `dirname $PATCH_FILE`/patch.sh"
	    echo "to make sure the patches were installed correctly."
	    echo
	    Pause
	    return 2
	fi

	return 0
}

#
# routine to check if installing a certain patch needs rebooting
#
NeedReboot ()
{
    if [ $REBOOT_NEEDED -eq 1 ]; then
	return 0
    fi

    for x in $REBOOT_LIST
    do
	if [ "$x" = "$1" ]; then
	    REBOOT_NEEDED=1
	    return 0
	fi
    done

    return 1
}


#
# Main routine to come in...
#
CheckAndInstallPatches ()
{
	# delete the "runiView" and "iView" links here...
	rm -f $HOME/runiView > /dev/null 2>&1
	rm -f $HOME/iView > /dev/null 2>&1

	echo
	echo "Checking if you need any patches before installing..."

	JREVERSION=${1:-1.3}  # desired JRE version can be passed here

	if [ "$JREVERSION" = "1.3" ]; then
	    if [ "$PLAT" = "i86pc" ]; then
		REBOOT_LIST="108529-18 108828-35 111307-04 112439-01"
	    else
		REBOOT_LIST="108528-18 108827-35"
	    fi
        elif [ "$JREVERSION" = "1.4" ]; then
            echo "Patches for JRE 1.4 needs to be installed separately"
            return 0
	else
	    # We can only install patches for 1.2.2 or 1.3
	    echo "Do not know anything about patches for JRE $JREVERSION"
	    Pause
	    return 1
	fi

	REL=`uname -r`
	case $REL in 
#		5.6)
#		REQUIRED_PATCHES=$REQ56
#		PATCH_FILE=$PATCH_FILE56
#		PATCH_DIR=5.6
#		;;
		5.7)
		REQUIRED_PATCHES=`tar tf ../$PATCH_FILE57 | cut -d"/" -f2 | cut -d"." -f1`
		PATCH_FILE=$PATCH_FILE57
		PATCH_DIR=5.7
		;;
		5.8)
		REQUIRED_PATCHES=`tar tf ../$PATCH_FILE58 | cut -d"/" -f2 | cut -d"." -f1`
		PATCH_FILE=$PATCH_FILE58
		PATCH_DIR=5.8
		;;
		*)
		echo "Cannot install patches for JRE on Solaris version $REL"
		echo "Only have patches for 5.7 and 5.8"
		Pause
		return 1
		;;
	esac

	#
	# See if we need any patches
	#
	CheckRequiredPatches
	if [ $? -eq 0 ]; then
		return 0
	fi

	#
	# We need patches, install them
	#
	REBOOT_NEEDED=0
	InstallPatches
	return $?  # 0 if everything is okay, 1 if something is wrong and 2 if reboot is needed
}
