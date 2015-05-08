#!/bin/sh

rsync_stat=0;

# Add the rsync to /etc/services

grep "^rsync[ 	]" /etc/services >/dev/null 2>&1

if [ $? != 0 ]; then
	mv /etc/services /etc/services.bak.$$
	cat /etc/services.bak.$$ - >/etc/services <<-EOF
	###  The following line has been added by the nextone's package  
	rsync 		873/tcp
	EOF
	echo "Adding an entry to /etc/services. Copied old file to /etc/services.bak.$$"
else
	echo "Warning: found an entry for rsync in /etc/services. Leaving file unmodified ..."
	rsync_stat=1;
fi

# Add the rsync to /etc/inetd.conf

grep "^rsync[	 ]" /etc/inetd.conf >/dev/null 2>&1

if [ $? != 0 ]; then
	mv /etc/inetd.conf /etc/inetd.conf.bak.$$
	cat /etc/inetd.conf.bak.$$ - >/etc/inetd.conf <<-EOF
	###  The following line has been added by the nextone's package  
	rsync	stream	tcp	nowait	root	/usr/local/bin/rsync rsync --daemon
	EOF
	echo "Adding an entry to /etc/inetd.conf. Copied old file to /etc/inetd.conf.bak.$$"
else
	echo "Warning: found an entry for rsync in /etc/inetd.conf. Leaving file unmodified ..."
	rsync_stat=1;
fi

if [ -f /etc/rsyncd.conf ]; then
	mv /etc/rsyncd.conf /etc/rsyncd.conf.bak.$$
	echo "Warning: found file /etc/rsyncd.conf. Copied it to /etc/rsyncd.conf.bak.$$"
fi

cp ./rsyncd.conf /etc/rsyncd.conf

if [ rsync_stat -ne 0 ]
then
	echo "You may want to check and delete the backup files created."
fi
