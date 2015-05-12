#!/bin/bash

#####################################################################################
#
# This script is used to consolidate Regression test results and put them in
# a comma separated value spreadsheet table based on FP-35605. 
# 
# To run it, you'll have to either copy generateResult.sh to the directory 
# where you have summarized your nextest results into a results_<test>.txt
# file, where <test> is the name of the test suite (such as obp, or sipt.
#
# The program can be run in one of two ways:
# 1) ./generateResult.sh 
#    (if the result files are in the same directory as generateResult.sh)
#
# 2) ./generateResult.sh <results_directory>
#    (where <results_directory> is the name of the directory containing the results)
#
# In either case a NexTest_Results_<iserver_version>.csv is produced, where 
# <iserver_version> is the verwsion of iserver being tested such as: 4.2c3-6.
#
# Author: Johnny Okafor; Nextone Communications; Feb 2007
#
######################################################################################

# Get today's date
Date=`date +%Y%m%d`

orig_dir=`pwd`

#Determine result directory
if [ ! -n "$1" ] ; then
   resultDir=`pwd`
else
   resultDir=$1
fi

cd $resultDir

# Determine nextest release candidate version
temp_nextest=`cat *.txt | grep -i version | grep -i nextest | awk '{print $3}' | tail -1`
temp_rcNextest=`echo $temp_nextest | tr '-' ' ' | awk '{print $4}' | tr 'pre' ' '`
rc_nextest1=`echo $temp_rcNextest | cut -d' ' -f1`
rc_nextest=`echo rc$rc_nextest1`
nextest_rel1=`echo $temp_nextest | tr '-' ' ' | awk '{print $2}'`
nextest_rel2=`echo $temp_nextest | tr '-' ' ' | awk '{print $3}'`

nextest_version=`echo $nextest_rel1.$nextest_rel2.$rc_nextest`
#echo $nextest_version

# Determine the iserver version
iserverVer=`cat *.txt | grep -i version | grep MSW | awk '{print $3}' | tr -d ':' | tail -1`
rel=`echo $iserverVer | cut -d'.' -f1`
ver=`echo $iserverVer | cut -d'.' -f2`
build=`echo $iserverVer | cut -d'.' -f3`
iserver_version=`echo $rel.$ver$build`
#echo $iserverVer

resFile=`echo NexTest_Results_${iserver_version}.csv`
Result_File=$resFile
#echo Result File is $Result_File

# FP-36372 Begin 
# Define Cumulative result file
# Create the /home/test/"nightly build results" if it does not exist
Nightly_Dir=nightly_results
if [ ! -d "/home/test/$Nightly_Dir" ]
then
   mkdir /home/test/$Nightly_Dir
fi
testDir=`echo "/home/test/$Nightly_Dir"`

release=`echo $rel.$ver`
Cumulative_File=`echo $testDir/Cumulative_Results_${release}.csv`
# Create the Cumulative Result File if it does not exist
if [ ! -f $Cumulative_File ] 
then
   echo Date,\""MSx   " > $Cumulative_File
   echo Version\",\""Nextest " >> $Cumulative_File
   echo Version\",Error,,Fail,,Untested,,Pass,,Total >> $Cumulative_File
   echo ,,,Count,%,Count,%,Count,%,Count,%,Count >> $Cumulative_File
fi
# FP-36372 End 

echo Date,\""MSx $iserver_version " > $Result_File
echo Components\",\""Nextest " >> $Result_File
echo Version\",Error,,Fail,,Untested,,Pass,,Total >> $Result_File
echo ,,,Count,%,Count,%,Count,%,Count,%,Count >> $Result_File

# Determine the test suites that have been run and process them
provisionFile=`grep production_components.provision * | grep -v -i binary | grep -v dat | cut -d':' -f1`
# FP38869 Begin
# Modify the result_files to process results in a directory with results_provision.txt and directories that do not contain the results_provision.txt file
if [ "$provisionFile" == "" ]
then
   result_files=`ls -ltr *.txt | awk '{print $9}'`
else
   result_files=`ls -ltr *.txt | grep -v $provisionFile | awk '{print $9}'`
fi
# FP38869 End
num=0
pass_total=0
fail_total=0
error_total=0
untested_total=0
total_test=0
testName[$num]=""
for i in $result_files
do

  fileName[$num]=$i
  #echo file name is ${fileName[$num]}
     test_total[$num]=`cat ${fileName[$num]} | grep total | awk '{print $1}'`
     #echo test total is ${test_total[$num]}

     pass_count[$num]=`cat ${fileName[$num]} | grep % | grep PASS | awk '{print $1}'`
     if [ "${pass_count[$num]}" == "" ]
     then
        pass_count[$num]=0
     fi
     #echo pass count is ${pass_count[$num]}

     pass_percent[$num]=`cat ${fileName[$num]} | grep % | grep PASS | tr -d '()' | awk '{print $2}'`
     if [ "${pass_percent[$num]}" == "" ]
     then
        pass_percent[$num]=0%
     fi

     fail_count[$num]=`cat ${fileName[$num]} | grep % | grep FAIL | awk '{print $1}'`
     if [ "${fail_count[$num]}" == "" ]
     then
        fail_count[$num]=0
     fi

     fail_percent[$num]=`cat ${fileName[$num]} | grep % | grep FAIL | tr -d '()' | awk '{print $2}'`
     if [ "${fail_percent[$num]}" == "" ]
     then
        fail_percent[$num]=0%
     fi

     error_count[$num]=`cat ${fileName[$num]} | grep % | grep ERROR | awk '{print $1}'`
     if [ "${error_count[$num]}" == "" ]
     then
        error_count[$num]=0
     fi

     error_percent[$num]=`cat ${fileName[$num]} | grep % | grep ERROR | tr -d '()' | awk '{print $2}'`
     if [ "${error_percent[$num]}" == "" ]
     then
        error_percent[$num]=0%
     fi

     untested_count[$num]=`cat ${fileName[$num]} | grep % | grep UNTESTED | awk '{print $1}'`
     if [ "${untested_count[$num]}" == "" ]
     then
        untested_count[$num]=0
     fi

     pass_percent[$num]=`cat ${fileName[$num]} | grep % | grep PASS | tr -d '()' | awk '{print $2}'`
     untested_percent[$num]=`cat ${fileName[$num]} | grep % | grep UNTESTED | tr -d '()' | awk '{print $2}'`
     if [ "${untested_percent[$num]}" == "" ]
     then
        untested_percent[$num]=0%
     fi

     # FP-36372 Begin 
     #prod_comp[$num]=`grep "production_components\." ${fileName[$num]} | grep provision | awk '{print $1}'`
     # grep production results_adaptive_fax_routing.txt | grep -v Cleanup | grep -v Setup | grep : | awk '{print $1}' | cut -d'.' -f2 | head -1
     temp=`grep production ${fileName[$num]} | grep -v Cleanup | grep -v Setup | grep : | awk '{print $1}'`
     prod_comp[$num]=`echo $temp | cut -d'.' -f2 | head -1`
     if [ "${prod_comp[$num]}" == "" ]
     then
         # We got here because the test suite is not under ../tdb/production_components.qms directory
         # For now this handles the negative test suites in ../tdb/negative.qms directory 
         # grep Cleanup results_protos.txt | awk '{print $2}' | grep [.] | cut -d'.' -f2 | head -1
         prod_comp[$num]=`grep Cleanup ${fileName[$num]} | awk '{print $2}' | grep [.] | cut -d'.' -f2 | head -1`
     fi
     
     # FP-36372 End 
     testName[$num]=`echo ${prod_comp[$num]} | cut -d'.' -f2`

     echo $Date,${testName[$num]},$nextest_version,${error_count[$num]},${error_percent[$num]},${fail_count[$num]},${fail_percent[$num]},${untested_count[$num]},${untested_percent[$num]},${pass_count[$num]},${pass_percent[$num]},${test_total[$num]} >> $Result_File

     pass_total=`expr $pass_total + ${pass_count[$num]}`
     fail_total=`expr $fail_total + ${fail_count[$num]}`
     error_total=`expr $error_total + ${error_count[$num]}`
     untested_total=`expr $untested_total + ${untested_count[$num]}`

  num=`expr $num + 1`
done

total_test=`expr $total_test + $pass_total + $fail_total + $error_total + $untested_total`

# Calculate the percentages of the various results
if [ $total_test -ne 0 ]
then
   pass_percent=`echo "scale=2; $pass_total * 100 / $total_test" | bc`
   pass_percent=`echo $pass_percent%`

   fail_percent=`echo "scale=2; $fail_total * 100 / $total_test" | bc`
   fail_percent=`echo $fail_percent%`

   error_percent=`echo "scale=2; $error_total * 100 / $total_test" | bc`
   error_percent=`echo $error_percent%`

   untested_percent=`echo "scale=2; $untested_total * 100 / $total_test" | bc`
   untested_percent=`echo $untested_percent%`

   echo $Date,TOTAL,,$error_total,$error_percent,$fail_total,$fail_percent,$untested_total,$untested_percent,$pass_total,$pass_percent,$total_test >> $Result_File

fi

# FP-36372 Begin 
# Append the over all result to the Cumulative results file. This will give a view of quality trend 
echo $Date,$iserverVer,$nextest_version,$error_total,$error_percent,$fail_total,$fail_percent,$untested_total,$untested_percent,$pass_total,$pass_percent,$total_test >> $Cumulative_File
# FP-36372 End 

cd $orig_dir

# FP35962
# Sending an attachment with the mail.

./attachwrapper.py  "Nightly Build Results" $orig_dir/$resultDir/$resFile
./attachwrapper.py  "Nightly Build Results" $Cumulative_File
