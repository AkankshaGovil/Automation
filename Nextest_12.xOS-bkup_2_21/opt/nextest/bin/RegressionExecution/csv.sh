
> results.csv

i=`ls results`

for m in $i
do
    echo $m

    #Extracting suite name
    index1=`expr index $m '_'`
    index2=`expr index $m '.'`

    length=`expr $index2 - $index1`

    str1=${m:$index1:$length-1}
    #str1=`expr substr $m $index1 $length`

    Total=`tail -6 results/$m | grep total |  awk '{ print $FNR }'`
    Pass=`tail -3 results/$m | grep PASS |  awk '{ print $FNR }'`

    str=$str1,$Total,$Pass

    echo $str >> results.csv
done


# Creating a log file having total tests and pass tests count for each suite

