#!/bin/sh
PID=`ps -eaf| grep defunct | grep -v grep`
if [ "$PID" == "" ]; then
echo "exiting"
exit
fi

echo $PID | awk '{print $2}' | xargs kill -9
echo "Killed $PID" >>/tmp/killed.log


