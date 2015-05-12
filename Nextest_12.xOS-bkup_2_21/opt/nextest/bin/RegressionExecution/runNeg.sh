
MajorVersion='8.0'

mkdir results

resultDir='results'

#Reading the source file for advanced suites to be tested
suitefile=suites_negative$MajorVersion.txt

# Create nextest log directory
sudo mkdir -p /var/temp

echo $suitefile
lineCount=`wc -l $suitefile | awk '{print $1}'`
echo $lineCount
count=1

while true; do

        if [ $count -gt $lineCount ]; then
                echo "Exiting the loop"
                break
        fi

        #reading next line
        line=`head -$count $suitefile | tail -1`

        #Skipping the suite if it is commented, it wont be executed
        a=${line:0:1}
        if [ $a = "#" ]; then
                echo $a
                count=`expr $count + 1`
                continue
        fi


        echo "Executing------------ engtest.$line ------------"

        echo "Starting time" `date`
        stime=`date`
        START=$(date +%s)

        > /tmp/nextest.log

        echo "corecount_primary"
        ssh root@mymsw 'ls -l /var/core | wc -l'

        echo "corecount_secondary"
        ssh root@bkupmsw 'ls -l /var/core | wc -l'

        /opt/nextest/bin/qmtest run -c nextest.scm_configuration=ON -c nextest.trace_level=DEBUG -c nextest.disable_debugflags=OFF -c nextest.radius_verify=OFF -o results_$line.qmr negative.$line

        Result_File=results_negative_$line.txt

        sudo cp /tmp/nextest.log /var/temp/$line.log
        qmtest summarize -f full results_$line.qmr > $Result_File
        mv results_negative_$line.txt $resultDir
        count=`expr $count + 1`

        echo "End time" `date`
        etime=`date`
        END=$(date +%s)

        DIFF=$(( $END - $START ))

        echo $line,$stime,$etime,$START,$END,$DIFF >> duration.csv

done

