#!/bin/sh

##
## Package script for packing and shipping NexTone executable.
##
##
## Sridhar Ramachandran, 11/08/98.
##


## Include help file
##. ./help.sh

## Globals
. ./globals.sh

## Utilities
. ./utils.sh

## Include aloid file
. ./aloid.sh

##
## Self identification.
MY_VERSION_NAME="NexTone Packing Program, "
MY_VERSION_ID="v0.1"
MY_COPYRIGHT="Copyright 2000 NexTone Communications, Inc."

NETOID_FILELIST="./netoid_filelist"
ALOID_FILELIST="./aloid_filelist"

PROG_VERSION_NAME=
PROG_VERSION_ID=

GENERIC_PACK_HELP="See the file help.sh for packaging help."

## Clear the screen.
ClearScreen

##
## Provide self identification
##
echo ""
echo "$MY_VERSION_NAME $MY_VERSION_ID"
echo "$MY_COPYRIGHT"
echo ""

##
## Offer generic help and instructions.
##
echo "$GENERIC_PACK_HELP"
echo ""

##
## Before entering mainloop, check for user input.
##
echo "Hit <CR> to continue"
read resp
echo " "


##
## Main loop
##
while [ 1 ]
do
$ECHON "What are you packing? [A]loid-iServer-Core/[N]-iServer-Admin/[B]oth "
read resp

	case "$resp" in
	[aA]*)
		echo "You have picked iServer Core Package. "
		echo ""
		PackageAloid
		break
	;;
	[nN]*)
		echo "You have picked iServer Administration Package. "
		echo ""
		PackageAdmin
		break
	;;
	[bB]*)
		echo "You have picked both iServer Core and Administration Packages. "
		echo ""
		PackageAloid
		echo ""
		PackageAdmin
		break
	;;
	*)
		echo "Answer correctly !!"
	;;
	esac

done


echo "Exiting... " 
exit 0
