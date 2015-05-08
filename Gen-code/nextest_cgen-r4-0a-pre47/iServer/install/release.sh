#!/bin/sh

##################################################
## Utility for releasing software.
## This utility descends into respective
## source directories and creates index files
## that are later used for creating a tape
## archive (tar) file that can be compressed.
##
## Currently lacks the feature where it can checkout
## appropriately tagged source files from a CVS 
## repository. Will be added soon. - SR, 03/15/96
##
## Author: 	Sridhar Ramachandran
## 
## Date:	March, 1999.
##################################################

## Include utilities library here.
. ./utils.sh


## Master variables.
LS=/bin/ls
DIRLISTFILE=$PROJECT/utils/build/dirlist
INDEXFILE=./.index
GZIP=/usr/local/bin/gzip
ProgVersion=v0.2
ProgDescription="NexTone Release Archiver"

## Relevant files
TARFILE=test-00.tar
MASTERFILE=$PROJECT/.masterfile

## Do the following directories.
DIRLIST=`cat $DIRLISTFILE`

## Files that we are interested in.
FILE_INTEREST="*.c *.h *akefile* *.sh *.cmd *.asm *.s *.txt *.inc *.awk" 

## Ensure clean slate before we start.
ClearScreen

## Identify ourselves.
echo "$ProgDescription $ProgVersion"
echo ""

## Some error checking needed here.
if [ -z "$PROJECT" ]; then
	echo ""
	echo "PROJECT environment variable not set"
	echo "Exiting.."
	exit 2
fi

## Go to root of source tree.
cd $PROJECT

## Remove old Masterfile.
rm -f $MASTERFILE

##
## Process all the relevant directories.
for i in $DIRLIST
do
	echo ""
	echo "Looking into $i"
	cd $i
	echo "Generating index files...."
	$LS $FILE_INTEREST  > $INDEXFILE 2> /dev/null
	echo "...done"
	FILELIST=`cat .index`
	for j in $FILELIST
	do
		echo "$i/$j" >> $MASTERFILE
	done	
	## Go back to parent directory.
	cd $PROJECT
done

echo ""
echo "Generation of index files done"
echo ""
AreYouSure "Are you sure you want to create a tape archive (tar) file" "y"
if [ $? = 0 ]; then
	echo "Exiting..."
	exit 0
fi

$ECHON "Give name of tar file: "
read filen

if [ -z "$filen" ]; then
	echo "Using default filename: $TARFILE"
else
	TARFILE=$filen
fi

## tar it up.
tar cvf $TARFILE `cat $MASTERFILE`
echo "Done taring....file is $TARFILE"

## Do you want to compress?
AreYouSure "Are you sure you want to compress" "y"
if [ $? = 1 ]; then
	echo "This may take several minutes...."
	echo "Compressing..."
	$GZIP $TARFILE
	echo "...done"
else
	echo "No compression done"
fi


exit 0


##
## TODO:
## 1. Add option to make or use same "index" files.
## 2. Make index auto generation complete.
##
