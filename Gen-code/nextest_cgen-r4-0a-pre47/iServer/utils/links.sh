#!/bin/sh

if [ ! -d $1 ]; then
	mkdir $1
fi

test -f $2 || test -d $2 || exit 0

cd $1	

(test -f $2 || test -d $2) && exit 0

ln -s ../$2 $2
