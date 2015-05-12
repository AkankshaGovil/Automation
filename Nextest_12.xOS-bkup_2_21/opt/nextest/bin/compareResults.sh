#!/bin/sh
# Ticket 29862
# Script for comparision of results against the certified results

# 34334 Done changes to take the path for the results file from cmdline
if [ $# -ne 3 ] ; then
    echo "Incorrect number of arguments provided!"
    echo "Usage: compareResults.sh PathofNewRunResults PathofCertifiedRunResults PathforStoringComparisonResults"
    exit
fi

echo "******************************************************************************************************************"
echo "Please note that the files under $1 that are of the form - results_suitename.txt will be compared"
echo " against the ones with the same name under $2. All other files will be ignored!"
echo "*******************************************************************************************************************"

# Path where the results file for the new run are located
pathForNewRun=$1

# Path where the results file for the certified run are located
pathForCertifiedRes=$2

# Path where the results file after comparision and log file will be located
pathForComparisionRes=$3

# Temporary files
currentResults="/tmp/currentRes.txt"
certifiedRes="/tmp/cert.txt"

#34334 - Modified to compare the results for the filenames prefixed with results_
newRunRes=`ls $pathForNewRun | grep 'results_' `
certRunRes=`ls $pathForCertifiedRes | grep 'results_'`

if [ ! -d $pathForComparisionRes ]; then
        #34334 Included -p option to create the subdirectories as well
        #Otherwise it gives an error no such file or directory  
	mkdir -p $pathForComparisionRes
fi

# Empty the log file 
> $pathForComparisionRes/log
echo "----------------------------------------------------" >> $pathForComparisionRes/log
echo "         Log file for Comparision results           " >> $pathForComparisionRes/log
echo "----------------------------------------------------" >> $pathForComparisionRes/log
echo >> $pathForComparisionRes/log
echo >> $pathForComparisionRes/log

for FILE in $newRunRes
    do
       date1=`date +%F`
       date2=`date +%R`
           #34334 - Modified to take more than one underscore
           suiteName=`ls $pathForNewRun/$FILE | cut -d "_" -f2- | cut -d "." -f1`
	   echo "COMPARISION RESULTS FOR SUITE ::::$suiteName:::::" >> $pathForComparisionRes/log
       echo "-------------------------------------------------" >> $pathForComparisionRes/log
	   
       if [ ! -f $pathForCertifiedRes/$FILE ]; then
		  echo "$date1 $date2: Error : results not compared " >> $pathForComparisionRes/log
		  echo "$date1 $date2: Error : $pathForCertifiedRes/$FILE doesnt exist" >> $pathForComparisionRes/log
		  echo >> $pathForComparisionRes/log
	      continue
	   fi
       grep "production_components" $pathForNewRun/$FILE | grep ":" | sort -u > $currentResults
	   
       grep "production_components" $pathForCertifiedRes/$FILE | grep ":" | sort -u > $certifiedRes
       
	   diff $currentResults $certifiedRes > /tmp/differ
	   
	   if [ ! -s /tmp/differ ]; then
		  echo "$date1 $date2: Results are SAME" >> $pathForComparisionRes/log
		  echo >> $pathForComparisionRes/log
		  continue
	   else 
	      cp /tmp/differ $pathForComparisionRes/difference_$FILE
	      echo "$date1 $date2: Results are DIFFERENT" >> $pathForComparisionRes/log
	      echo "$date1 $date2: Details in the file $pathForComparisionRes/difference_$FILE " >> $pathForComparisionRes/log
	      echo >> $pathForComparisionRes/log
	  fi
	done
		
echo
echo
echo "Comparision is complete, see $pathForComparisionRes/log for details"
echo
echo
