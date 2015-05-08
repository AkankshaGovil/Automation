#!/bin/sh
#
# checkpackage.sh  - library routines to check and install any required
#                    solaris packages for running JRE
#

#
# any other library inclusions
#
DIR=${1:-.}
. $DIR/globals.sh
. $DIR/utils.sh

REQ551="SUNWi1of SUNWxwfnt"
REQ56="SUNWi1of SUNWxwfnt"
REQ57="SUNWi1of SUNWxwfnt"

PKGINFO=/usr/bin/pkginfo

#
# Check to see if the required packages are there
#
CheckRequiredPackages ()
{
    NEED=""
    AVAIL=""
    for package in $REQUIRED_PACKAGES
    do
#	echo package is $package
	status=`$PKGINFO -l $package | grep STATUS | cut -d":" -f2 | tr -s ' ' ' ' | cut -d" " -f3`
#	echo status is $status
	status=${status:-notinstalled}
#	echo status is $status
#	Pause
	if [ $status != "installed" ]; then
	    NEED="$NEED $package"
	else
	    AVAIL="$AVAIL $package"
	fi
    done
    echo
    echo "Required Packages: $REQUIRED_PACKAGES"
    echo "Currently Installed: $AVAIL"
    echo "Needed: $NEED"
    echo

    #
    # Do we need any packages?
    #
    need=`echo $NEED | tr -d ' '`
    if [ -z "$need" ]; then
	echo "All required packages are already installed on this machine"
	ret=0
    else
	echo "For best experience with this software, please install the following packages:"
	echo "    $NEED"
	echo "(They will be available on the Solaris installation CD)"
	ret=1
    fi
    Pause
    return $ret
}

#
# Main routine to come in...
#
echo
echo "Checking for any required Solaris packages before installing the JRE..."
REL=`uname -r`
    case $REL in 
    5.5.1)
    REQUIRED_PACKAGES=$REQ551
    ;;
    5.6)
    REQUIRED_PACKAGES=$REQ56
    ;;
    5.7)
    REQUIRED_PACKAGES=$REQ57
    ;;
esac

#
# See if we need any packages
#
CheckRequiredPackages
exit $?
