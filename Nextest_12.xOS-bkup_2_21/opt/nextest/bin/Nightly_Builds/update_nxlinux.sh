#!/bin/bash

#####################################################################################
#
# This script is used to determine when there is a new (or higher version) of nxlinux
# that is included with the i686pc-msw*.tar.gz(.nc) MSX binary. It performs the 
# following functions: 
#  
#
# - If the downloaded nxlinux-master-update-5.0.1 nxlinux file is the same or older
#   than the currently installed NxLinux, it does nothing.
#
# - If the downloaded nxlinux-master-update-5.0.1 nxlinux file is newer or higher
#   than the currently installed NxLinux, it installs it and reboots the MSX.
#
# - The information used in making the decision is extracted from the /etc/NxLinux-release
#   file and from the nxlinux-master-update-5.0.1 file signature.
#
# To run it you will have to invoke it from the /root/scripts/dailybuild directory of 
# the MSX as follows:
#
# /root/scripts/dailybuild/update_nxlinux.sh
#
#
# Author: Johnny Okafor; Nextone Communications; Jan 2007
#
######################################################################################

#------------------------------------------------------------------------------------#
# FP 51710 | Modify to handle hyphenated nxlinux packages iwith hyphenated versions  #
# Feb 2008 | such as this: nxlinux-master-update-5.0.4-15                   	     #
#  JMO     |           								     #
#------------------------------------------------------------------------------------#
#
# FP 35088 | Created this script for the first time. To be used in aplying the       #
# Jan 2007 | nxlinux-master-update to the MSX nxlinux upgrade                        #
#  JMO     |                                                                         #
#------------------------------------------------------------------------------------#

# Path where the MSX i686pc-msw*tar.gz(.nc) binary package is located
buildPath=/root/IserverBuilds/dailybuilds
# Path where this script is located
scriptsDir=/root/scripts/dailybuild

# Display what version of nxlinux is installed
cat /etc/NxLinux-release

# Determine the installed nxlinux
nxlinux_test=`cat /etc/NxLinux-release | tail -1 | cut -d' ' -f1`
if [ $nxlinux_test == "NxLinux" ]
then
   # Nxlinux had been previously upgraded
   installed_linux=`cat /etc/NxLinux-release | tail -1 | awk '{print $2}'`
else
   # Nxlinux has just been freshly installed
   installed_linux=`cat /etc/NxLinux-release | head -1 | cut -d' ' -f3`
fi
   
nxlinux_master_update=`ls $buildPath | grep nxlinux-master-update | sort | tail -1`
nxlinux_masterStat=`echo $?`
if [ $nxlinux_masterStat -eq 0 ]
then
   # FP 51710 Begin
   # There is an nxlinux-master-update file. Process it
   #downloaded_linux=`echo $nxlinux_master_update | cut -d'-' -f4`
   #`echo nxlinux-master-update-5.0.4-14 | cut -b 23-33`   
   downloaded_linux=`echo $nxlinux_master_update | cut -b 23-33`
   # FP 51710 End

   [ $installed_linux = $downloaded_linux ]
   Stat=`echo $?`
   if [ $Stat -eq 1 ]
   then
      # FP 51710 Begin
      #echo "The installed nxlinux and the downloaded nxlinux are different"
      # Sort the installed & downloaded nxlinuxr. Use the sorted result to install 
      # the downloaded nxlinux (if it is newer or greater than the installed nxlinux) 
      result=`echo $downloaded_linux $installed_linux | tr ' ' '\n' | sort | tail -1`
      # FP 51710 End
      [ $result = $downloaded_linux ]
      resultStat=`echo $?`
      if [ $resultStat -eq 0 ]
      then
         # FP 51710 Begin
         echo "The installed nxlinux and the downloaded nxlinux are different"
         #echo "Install the downloaded nxlinux-master-update file"
         echo "Installing the downloaded nxlinux-master-update file: $nxlinux_master_update"
         # FP 51710 End
         $buildPath/$nxlinux_master_update
         # Reboot the server for the linux update to take effect
         echo "The nxlinux has been updated with $nxlinux_master_update"
         sleep 2
         echo "The system is going down for reboot"
         reboot
      fi
   else
      echo "$nxlinux_master_update is up to date. There is no need to update the nxlinux"
   fi
else
   echo "There is no nxlinux_master_update file in $buildPath directory"
fi
