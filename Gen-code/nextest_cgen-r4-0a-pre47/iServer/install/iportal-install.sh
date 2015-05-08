#!/bin/sh
#
#

. `dirname $0`/globals.sh
. `dirname $0`/utils.sh
. `dirname $0`/checkpatch.sh

DEFAULTDIR="/usr/local/nextone-iportal"

if [ "$MC" = "SunOS" ]
then
    TOMCATFILE="tomcat.tar"
    TOMCATINSTALLDIR="/usr/local"
    TOMCATDIRNAME="jakarta-tomcat-3.2.1"
    JDKFILE="jdk13.sh"
    JDKINSTALLDIR="/usr/local"
    JDKDIRNAME="j2sdk1_3_1_01"
else
    $ECHO "Installing iPortal is only supported on Solaris platforms"
    exit 0
fi


CheckForSuperuser ()
{
    if [ "$LOGNAME" != "root" ]; then
	echo ""
	echo "You must be super-user to run this script."
	echo "Exiting.... "
	exit 2
    fi
}

# creates the script to start/stop the server, $1 is the script file
CreateStartupScript ()
{
    echo "#!/bin/sh" > $1
    echo "#" >> $1
    echo "# chkconfig: 345 95 95" >> $1
    echo "# description: Nextone Tomcat startup script" >> $1
    echo "#" >> $1
    echo "" >> $1
    echo "TOMCATDIR=$TOMCATDIR" >> $1
    echo "JAVA_HOME=$JAVA_HOME; export JAVA_HOME" >> $1
    echo "" >> $1
    echo "STARTDIR=\`pwd\`" >> $1
    echo "case \"\$1\" in" >> $1
    echo "    start)" >> $1
    echo "        cd \$TOMCATDIR/bin" >> $1
    echo "        nohup ./startup.sh > \$TOMCATDIR/logs/shlog 2>&1" >> $1
    echo "        cd \$STARTDIR" >> $1
    echo "        ;;" >> $1
    echo "    stop)" >> $1
    echo "        cd \$TOMCATDIR/bin" >> $1
    echo "        ./shutdown.sh >> \$TOMCATDIR/logs/shlog 2>&1" >> $1
    echo "        cd \$STARTDIR" >> $1
    echo "        ;;" >> $1
    echo "    status)" >> $1
    echo "        ;;" >> $1
    echo "    restart)" >> $1
    echo "        \$0 stop" >> $1
    echo "        \$0 start" >> $1
    echo "        ;;" >> $1
    echo "    *)" >> $1
    echo "        echo \"Usage: \$0 {start|stop|status|restart}\"" >> $1
    echo "        exit 1" >> $1
    echo "esac" >> $1
    echo "" >> $1
    echo "exit 0" >> $1

    chmod 755 $1
}

MainInstallLoop ()
{
    STARTDIR=`pwd`

    if [ -d "/usr/local/nextone/iportal" -o -d "$DEFAULTDIR" ]
    then
	ClearScreen
	$ECHO " "
	$ECHO " "
	$ECHO "Nextone iPortal installer has detected a previous installation of iPortal."
	$ECHO "Please use the upgrade option if you wish to upgrade to a new release."
	cd $STARTDIR
	Pause
	return 0
    fi

    $ECHO ""
    $ECHO "Once installed, the iPortal server will listen on the default HTTP port (80)"
    $ECHO "Trying to run another web server on that port will create problems"
    $ECHO ""
    AreYouSure "Continue with the installation" "y"
    if [ $? -eq 0 ]
    then
	return 0
    fi

    CheckPort "80" "A server process is found running on port 80"
    if [ $? -ne 0 ]
    then
	AreYouSure "iPortal may already be running, continue to install" "y"
	if [ $? -eq 0 ]
	then
	    return 0
	fi
    fi

    # Blurb about space requirements etc.
#    SIZEOFMEDIA=`du -s | awk ' { print $1 } ' `
#    SPACE=`expr 3 \* $SIZEOFMEDIA / 1024`

    # Check if we meet the space requirements
#    AVAIL=`df -k  /usr  |sed '1d' |awk '{print $4}'`
#    if [ $AVAIL -lt $SPACE ]
#    then
#	$ECHO "You will need atleast $SPACE Kbytes for this install."
#	$ECHO "Only $AVAIL Kbytes available"
#	AreYouSure "Are you sure you want to proceed"
#	if [ $? = 0 ]; then
#	    return 1
#	fi
#    else 
#	$ECHO "$SPACE Kbytes Needed,  $AVAIL Kbytes available ... Proceeding"
#    fi
 
    # install JDK
    while [ 1 ]
    do
	JdkInstallLoop
	status=$?
	if [ $status -eq 0 ]
	then
	    break
	elif [ $status -eq 2 ]
	then
	    return 1
	fi
    done

    # install tomcat
    while [ 1 ]
    do
	TomcatInstallLoop
	status=$?
	if [ $status -eq 0 ]
	then
	    break
	elif [ $status -eq 2 ]
	then
	    return 1
	fi
    done

    # store the current installed data
    VERSION=`cat $STARTDIR/versionfile`
    StoreVersion

    # create the nextoneIportal script
    CreateStartupScript $TOMCATDIR/bin/nextoneIportal

    # ask if we need to start iportal at startup
    AreYouSure "Would you like to start iPortal automatically when the machine boots up" "y"
    if [ $? -eq 1 ]
    then
	ln -s $TOMCATDIR/bin/nextoneIportal /etc/init.d/nextoneIportal
	ln -s /etc/init.d/nextoneIportal /etc/rc3.d/S95nextoneIportal
	ln -s /etc/init.d/nextoneIportal /etc/rc1.d/K95nextoneIportal
    fi

    # create the web.xml file
    cp $STARTDIR/iPortal.war $TOMCATDIR/webapps/
    mkdir $TOMCATDIR/webapps/iPortal
    cd $TOMCATDIR/webapps/iPortal
    $JAVA_HOME/bin/jar xf ../iPortal.war
    cd $STARTDIR
    $ECHO ""
    $ECHO "iPortal Configuration:"
    $ECHO "----------------------"
    ./createwebxml.sh $TOMCATDIR/webapps/iPortal/WEB-INF/web.xml

    # create the soft link
    ln -s $DEFAULTDIR-$VERSION $DEFAULTDIR

    $ECHO ""
    $ECHO "Installation completed successfully"
    $ECHO ""

    Pause
    return 0
}

MainUninstallLoop ()
{
    STARTDIR=`pwd`

    if [ ! -r "$DEFAULTDIR/.data" ]
    then
	$ECHO "Could not find the previous installation directory"
	Pause
	return 1
    fi

    CheckPort "80" "A server process is found running on port 80"
    if [ $? -ne 0 ]
    then
	AreYouSure "iPortal may already be running, continue to uninstall" "n"
	if [ $? -eq 0 ]
	then
	    return 0
	fi
    fi

    OLDVERSION=`cat $DEFAULTDIR/.data | grep VERSION | cut -d= -f2-`
    AreYouSure "Uninstall iPortal version $OLDVERSION" "n"
    if [ $? -eq 0 ]
    then
	$ECHO "Aborting uninstall..."
	Pause
	return 0
    fi

    removeFlag=0

    OLDJAVA_HOME=`cat $DEFAULTDIR/.data | grep JAVA_HOME | cut -d= -f2-`
    if [ -d "$OLDJAVA_HOME" ]
    then
	# delete the old jdk directory
	AreYouSure "Delete the previous JDK installation" "n"
	if [ $? -eq 1 ]
	then
	    rm -rf $OLDJAVA_HOME
	fi
    else
	$ECHO "Unable to find the previous JDK installation at $OLDJAVA_HOME"
	removeFlag=1
    fi

    OLDTOMCATDIR=`cat $DEFAULTDIR/.data | grep TOMCATDIR | cut -d= -f2-`
    if [ -d "$OLDTOMCATDIR" ]
    then
	rm -rf $OLDTOMCATDIR
    else
	$ECHO "Unable to find the previous tomcat installation at $OLDTOMCATDIR"
	removeFlag=1
    fi

    rm -f /etc/init.d/nextoneIportal
    rm -f /etc/rc3.d/S95nextoneIportal
    rm -f /etc/rc1.d/K95nextoneIportal
    rm -rf $DEFAULTDIR-$OLDVERSION
    rm -rf $DEFAULTDIR

    if [ $removeFlag -ne 0 ]
    then
	$ECHO "Some of the installated files have not been deleted, you may have to delete them manually"
    fi

    $ECHO ""
    $ECHO "Uninstallation completed successfully"
    $ECHO ""

    Pause
    return 0
}

MainUpgradeLoop ()
{
    STARTDIR=`pwd`

    if [ ! -d "$DEFAULTDIR" -a ! -d "/usr/local/nextone/iportal" ]
    then
	$ECHO "Cannot find a previous installation, please use the install option"
	Pause
	return 1
    fi

    # check for anything running on port 80
    CheckPort "80" "A server process is found running on port 80"
    if [ $? -ne 0 ]
    then
	AreYouSure "iPortal may already be running, continue with upgrade" "n"
	if [ $? -eq 0 ]
	then
	    return 0
	fi
    fi

    if [ -r "$DEFAULTDIR/.data" ]
    then
	OLDVERSION=`cat $DEFAULTDIR/.data | grep VERSION | cut -d= -f2-`
	OLDJAVA_HOME=`cat $DEFAULTDIR/.data | grep JAVA_HOME | cut -d= -f2-`
	OLDTOMCATDIR=`cat $DEFAULTDIR/.data | grep TOMCATDIR | cut -d= -f2-`
	haveData=1
    else
	OLDVERSION="0.0"
	AskUserInput "Please enter your previous JDK installation directory" "None"
	if [ "$_retval" != "None" -a ! -x "$_retval/bin/javac" ]
	then
	    $ECHO "Cannot find the previous JDK installation at $_retval"
	    Pause
	    return 1
	else
	    OLDJAVA_HOME=$_retval
	fi
	AskUserInput "Please enter your previous tomcat installation directory" "None"
	if [ "$_retval" != "None" -a ! -f "$_retval/webapps/iPortal.war" ]
	then
	    $ECHO "Cannot find the previous tomcat installation at $_retval"
	    Pause
	    return 1
	else
	    OLDTOMCATDIR=$_retval
	fi
	haveData=0
    fi


    VERSION=`cat $STARTDIR/versionfile`

    # cannot upgrade within the same version
    if [ "$VERSION" = "$OLDVERSION" ]
    then
	$ECHO "Cannot upgrade to the same version ($OLDVERSION->$VERSION), use the uninstall/install option instead"
	Pause
	return 1
    fi

    # make sure about the upgrade
    AreYouSure "Upgrade from $OLDVERSION to $VERSION" "y"
    if [ $? -eq 0 ]
    then
	$ECHO "Aborting upgrade..."
	Pause
	return 0
    fi

    # if we did not have a previous JDK, install one
    if [ "$OLDJAVA_HOME" = "None" ]
    then
	while [ 1 ]
	do
	    JdkInstallLoop
	    if [ $? -eq 0 ]
	    then
		break
	    fi
	done
    else
	JAVA_HOME=$OLDJAVA_HOME
    fi

    # if we did not have a previous tomcat, install one
    if [ "$OLDTOMCATDIR" = "None" ]
    then
	while [ 1 ]
	do
	    TomcatInstallLoop
	    if [ $? -eq 0 ]
	    then
		break
	    fi
	done
    else
	TOMCATDIR=$OLDTOMCATDIR
    fi

    # store the current installed data
    StoreVersion

    # create the nextoneIportal script
    CreateStartupScript $TOMCATDIR/bin/nextoneIportal

    # ask if we need to start iportal at startup
    rm -f /etc/init.d/nextoneIportal
    rm -f /etc/rc3.d/S95nextoneIportal
    rm -f /etc/rc1.d/K95nextoneIportal
    AreYouSure "Would you like to start iPortal automatically when the machine boots up" "y"
    if [ $? -eq 1 ]
    then
	ln -s $TOMCATDIR/bin/nextoneIportal /etc/init.d/nextoneIportal
	ln -s /etc/init.d/nextoneIportal /etc/rc3.d/S95nextoneIportal
	ln -s /etc/init.d/nextoneIportal /etc/rc1.d/K95nextoneIportal
    fi

    # keep the same web.xml?
    createXml=1
    if [ -r "$TOMCATDIR/webapps/iPortal/WEB-INF/web.xml" ]
    then
	AreYouSure "Would you like to keep the previous web.xml file" "y"
	if [ $? -eq 1 ]
	then
	    cp $TOMCATDIR/webapps/iPortal/WEB-INF/web.xml $TOMCATDIR/webapps/
	    createXml=0
	fi
    fi

    # copy the new war file and the web.xml
    rm -rf $TOMCATDIR/webapps/iPortal/
    cp $STARTDIR/iPortal.war $TOMCATDIR/webapps/
    mkdir $TOMCATDIR/webapps/iPortal
    cd $TOMCATDIR/webapps/iPortal
    $JAVA_HOME/bin/jar xf ../iPortal.war
    cd $STARTDIR
    if [ $createXml -eq 0 ]
    then
	mv $TOMCATDIR/webapps/web.xml $TOMCATDIR/webapps/iPortal/WEB-INF/
    else
	$ECHO ""
	$ECHO "iPortal Configuration:"
	$ECHO "----------------------"
	./createwebxml.sh $TOMCATDIR/webapps/iPortal/WEB-INF/web.xml
    fi

    # create the soft link
    rm -f $DEFAULTDIR
    ln -s $DEFAULTDIR-$VERSION $DEFAULTDIR

    # move the phone number databases into the new directory
    if [ "$OLDVERSION" = "0.0" ]
    then
	cd /usr/local/nextone/iportal
    else
	cd $DEFAULTDIR-$OLDVERSION
    fi
    for dir in `ls`
    do
	if [ -d $dir ]
	then
	    mv $dir $DEFAULTDIR-$VERSION
	fi
    done

    $ECHO ""
    $ECHO "Upgrade completed successfully"
    $ECHO ""

    Pause
    return 0
}

# checks if there is a server running on port $1, if it is, displays $2 message
CheckPort ()
{
    $ECHO "Checking for any server process waiting on port 80..."
    tmp=`netstat -a -P tcp -f inet | tr -s ' ' ' ' | grep "\*\.$1 " `
    if [ "$tmp" != "" ]
    then
	if [ -z "$2" ]
	then
	    $ECHO $2
	fi
	return 1
    fi

    return 0
}

# creates the current version directory and stores the version and dir details
StoreVersion ()
{
    mkdir $DEFAULTDIR-$VERSION
    echo "VERSION=$VERSION" > $DEFAULTDIR-$VERSION/.data
    echo "JAVA_HOME=$JAVA_HOME" >> $DEFAULTDIR-$VERSION/.data
    echo "TOMCATDIR=$TOMCATDIR" >> $DEFAULTDIR-$VERSION/.data
    return 0
}

JdkInstallLoop ()
{
    CheckAndInstallPatches 1.3
    status=$?
    if [ $status -ne 0 -a $status -ne 2 ]
    then
	AreYouSure "Did not complete installing patches, continue" "y"
	if [ $? -eq 0 ]
	then
	    return 2
	fi
    fi

    # ask if JDK is already installed
    AreYouSure "Install JDK from scratch" "y"
    if [ $? -eq 0 ]
    then
	AreYouSure "Use an existing JDK installation" "y"
	if [ $? -eq 1 ]
	then
	    ORIGJDKINSTALLDIR=$JDKINSTALLDIR/$JDKDIRNAME
	    while [ 1 ]
	    do
		GetJdkInstallDir
		if [ ! -d "$JDKINSTALLDIR" ]
		then
		    $ECHO "Directory \"$JDKINSTALLDIR\" does not exist"
		elif [ ! -x "$JDKINSTALLDIR/bin/javac" ]
		then
		    $ECHO "Executable file \"JDKINSTALLDIR/bin/javac\" does not exist"
		else
		    break
		fi
	    done

	    JdkInstallFinish
	    return $?
	else
	    return 2
	fi
    fi

    # install the JDK
    AskUserInput "Enter the JDK shell archive file" "$JDKFILE"
    if [ "$_retval" = "$JDKFILE" ]
    then
	_retval=`pwd`/$_retval
    fi
    if [ ! -f "$_retval" ]
    then
	$ECHO "Cannot find \"$_retval\": No such file"
	Pause
	return 1
    fi
    FULLJDKFILE=$_retval

    # accept install dir
    while [ 1 ]
    do
	AskUserInput "Enter destination directory" "$JDKINSTALLDIR"
	if [ -d "$_retval/$JDKDIRNAME" ]
	then
	    AreYouSure "An installation of JDK already exists in that directory, overwrite" "y"
	    if [ $? -eq 1 ]
	    then
		rm -rf $_retval/$JDKDIRNAME
		break
	    fi
	else
	    if [ ! -d "$_retval" ]
	    then
		mkdir $_retval
		if [ $? -ne 0 ]
		then
		    $ECHO "Error creating destination directory"
		else
		    break
		fi
	    fi
	    if [ ! -w "$_retval" ]
	    then
		$ECHO "Insufficient permissions to install under $_retval"
	    else
		break
	    fi
	fi
    done
    JDKINSTALLDIR=$_retval/$JDKDIRNAME

    CURPWD=`pwd`
    cd `dirname $JDKINSTALLDIR`
    $FULLJDKFILE
    if [ $? -ne 0 ]
    then
	cd $CURPWD
	Pause
	return 1
    fi

    cd $CURPWD
    JdkInstallFinish
    return $?
}

GetJdkInstallDir ()
{
	$ECHO ""
	AskUserInput "Enter existing JDK directory" "$ORIGJDKINSTALLDIR"
	JDKINSTALLDIR=$_retval
}

JdkInstallFinish ()
{
    JVERSION=`$JDKINSTALLDIR/bin/java -version 2>&1 | grep -i "java version" | cut -d\" -f2 | cut -d\_ -f1`
    if [ "$JVERSION" != "1.3.0" -a "JVERSION" != "1.3.1" ]
    then
	$ECHO "Need java version 1.3.0/1.3.1 to run iPortal, (found version $JVERSION)"
	return 1
    fi

    $ECHO "JDK found successfully at $JDKINSTALLDIR"
    JAVA_HOME=$JDKINSTALLDIR
    return 0
}

TomcatInstallLoop ()
{
    AskUserInput "Enter the tomcat tar file" "$TOMCATFILE"
    if [ "$_retval" = "$TOMCATFILE" ]
    then
	_retval=`pwd`/$_retval
    fi
    if [ ! -f "$_retval" ]
    then
	$ECHO "Cannot find \"$_retval\": No such file"
	Pause
	return 1
    fi
    FULLTOMCATFILE=$_retval

    # accept install dir
    while [ 1 ]
    do
	AskUserInput "Enter destination directory" "$TOMCATINSTALLDIR"
	if [ -d "$_retval/$TOMCATDIRNAME" ]
	then
	    AreYouSure "An installation of tomcat already exists in that directory, overwrite" "y"
	    if [ $? -eq 1 ]
	    then
		rm -rf $_retval/$TOMCATDIRNAME
		break
	    fi
	else
	    if [ ! -d "$_retval" ]
	    then
		mkdir $_retval
		if [ $? -ne 0 ]
		then
		    $ECHO "Error creating destination directory"
		else
		    break
		fi
	    fi
	    if [ ! -w "$_retval" ]
	    then
		$ECHO "Insufficient permissions to install under $_retval"
	    else
		break
	    fi
	fi
    done
    TOMCATINSTALLDIR=$_retval/$TOMCATDIRNAME

    CURPWD=`pwd`
    cd `dirname $TOMCATINSTALLDIR`
    tar xf $FULLTOMCATFILE
    if [ $? -ne 0 ]
    then
	cd $CURPWD
	Pause
	return 1
    fi

    cd $CURPWD
    TOMCATDIR=$TOMCATINSTALLDIR
    return 0
}


## main menu
MainMenu ()
{
    ClearScreen
    cat <<EOF



		${BOLD}NexTone iPortal Setup Utility. v1.0${OFFBOLD}



		1.	Install iPortal.

		2.	Uninstall iPortal.

		3.	Upgrade iPortal.


		q.   Exit this utility.

EOF

}


##
## main program starts here
##
if [ $# -eq 1 ]
then
    ECHO "Usage: ./$0"
    exit 0
fi

CheckForSuperuser

## main loop
while [ 1 ]
do
    MainMenu
    SelectChoice
    read ans

    case $ans in
    1)
	MainInstallLoop
	;;
    2)
	MainUninstallLoop
	;;
    3)
	MainUpgradeLoop
	;;
    q|Q)
	ClearScreen
	exit 0
	;;
    esac
done

exit 0
