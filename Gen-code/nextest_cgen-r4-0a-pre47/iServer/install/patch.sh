#!/bin/sh
#
# patch.sh  - used while installing iView on solaris
#
#
DIR=${1:-.}
. $DIR/checkpatch.sh

CheckAndInstallPatches 1.3
status=$?
if [ $status -eq 2 ]; then
    echo "One or more of the patches installed require rebooting"
    echo
    echo "Patch installation may not be complete, please re-run the patch installer (`pwd`/patch.sh)"
    echo "script after the reboot to complete the patch installation"
    exit 0
elif [ $status -ne 0 ]; then
    AreYouSure "Did not complete installing patches, continue?" "n"
    if [ $? -eq 0 ]; then
	exit 1
    fi
fi

# check if any packages are needed
./checkpackage.sh

# proceed to the GUI install
if [ -x install.bin ]
then
  ./install.bin
fi
exit 0
