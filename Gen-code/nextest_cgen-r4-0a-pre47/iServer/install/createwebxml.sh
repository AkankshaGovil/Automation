#!/bin/sh
#
# creates the web.xml file from the user inputs
#



. `dirname $0`/globals.sh
. `dirname $0`/utils.sh


# $1 is the parameter name, $2 is the parameter value and the rest of the arguments
# are the list of files to append
AppendInitParam ()
{
    if [ $# -lt 3 ]
    then
	echo "error in # of arguments for AppendInitParam"
	exit -1
    fi

    PARAM_NAME=$1
    shift
    PARAM_VALUE=$1
    shift
    while [ $# -gt 0 ]
    do
	echo "    <init-param>" >> $1
	echo "      <param-name>$PARAM_NAME</param-name>" >> $1
	echo "      <param-value>$PARAM_VALUE</param-value>" >> $1
	echo "    </init-param>" >> $1
	shift
    done
}

InitServlet1 ()
{
    echo "  <servlet>" > $SERVLET1
    echo "    <servlet-name>" >> $SERVLET1
    echo "      tod" >> $SERVLET1
    echo "    </servlet-name>" >> $SERVLET1
    echo "    <servlet-class>" >> $SERVLET1
    echo "      com.nextone.iportal.TimeOfDayServlet" >> $SERVLET1
    echo "    </servlet-class>" >> $SERVLET1
}

FinishServlet1 ()
{
    echo "    <load-on-startup>1</load-on-startup>" >> $SERVLET1
    echo "  </servlet>" >> $SERVLET1
}

InitServlet2 ()
{
    echo "  <servlet>" > $SERVLET2
    echo "    <servlet-name>" >> $SERVLET2
    echo "      web" >> $SERVLET2
    echo "    </servlet-name>" >> $SERVLET2
    echo "    <servlet-class>" >> $SERVLET2
    echo "      com.nextone.iportal.ibrowse.IBrowseServlet" >> $SERVLET2
    echo "    </servlet-class>" >> $SERVLET2
}

FinishServlet2 ()
{
    echo "    <load-on-startup>2</load-on-startup>" >> $SERVLET2
    echo "  </servlet>" >> $SERVLET2
}

InitServlet3 ()
{
    echo "  <servlet>" > $SERVLET3
    echo "    <servlet-name>" >> $SERVLET3
    echo "      palm" >> $SERVLET3
    echo "    </servlet-name>" >> $SERVLET3
    echo "    <servlet-class>" >> $SERVLET3
    echo "      com.nextone.iportal.ipalm.PalmServlet" >> $SERVLET3
    echo "    </servlet-class>" >> $SERVLET3
}

FinishServlet3 ()
{
    echo "    <load-on-startup>3</load-on-startup>" >> $SERVLET3
    echo "  </servlet>" >> $SERVLET3
}



# main script starts here, $1 is the destination file, $2, $3 and $4 are some temp files
if [ $# -eq 4 ]
then
    FILE=$1
    SERVLET1=$2
    SERVLET2=$3
    SERVLET3=$4
else
    FILE=web.xml
    TIMEOFDAY=`date '+%H%M%S'`
    SERVLET1=/tmp/tod.$TIMEOFDAY
    SERVLET2=/tmp/web.$TIMEOFDAY
    SERVLET3=/tmp/palm.$TIMEOFDAY
fi
if [ $# -eq 1 ]
then
    FILE=$1
fi

echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" > $FILE
echo "<!DOCTYPE web-app PUBLIC \"-//Sun Microsystems, Inc.//DTD Web Application 2.2//EN\" \"http://java.sun.com/j2ee/dtds/web-app_2_2.dtd\">" >> $FILE
echo "<web-app>" >> $FILE

InitServlet1
InitServlet2
InitServlet3

AppendInitParam "phoneDbDirectory" "/usr/local/nextone-iportal" "$SERVLET1" "$SERVLET2" "$SERVLET3"

# ask for user choices

# iserver address
AskUserInput "Please enter the iServer Address" "127.0.0.1"
AppendInitParam "iServerAddress" "$_retval" "$SERVLET1" "$SERVLET2" "$SERVLET3"

# Email server configuration
echo ""
echo "Email Server Configuration:"
# smtp server
AskUserInput "  SMTP server" "smtp.nextone.com"
AppendInitParam "smtpServer" "$_retval" "$SERVLET2" "$SERVLET3"
# pop3 server
AskUserInput "  POP3 server" "pop3.nextone.com"
AppendInitParam "pop3Server" "$_retval" "$SERVLET2" "$SERVLET3"

# email user account configuration
echo ""
echo "Email User Account Configuration:"
# user name
AskUserInput "  User Name" "iportal-ecall%nextone.com"
AppendInitParam "emailUserName" "$_retval" "$SERVLET2" "$SERVLET3"
# password
AskUserInput "  Password" "iportal1"
AppendInitParam "emailPassword" "$_retval" "$SERVLET2" "$SERVLET3"
# email address
AskUserInput "  Default email address" "iportal-ecall@nextone.com"
AppendInitParam "emailDefaultAddress" "$_retval" "$SERVLET2" "$SERVLET3"

# conference server configuration
echo ""
echo "Conference Server Configuration:"
# conference server
AskUserInput "  Conference Server Address" "127.0.0.1"
AppendInitParam "confServer" "$_retval" "$SERVLET2" "$SERVLET3"
# conference gateway
AskUserInput "  Conference Server Gateway Address" "127.0.0.1"
AppendInitParam "confGateway" "$_retval" "$SERVLET2" "$SERVLET3"

FinishServlet1
FinishServlet2
FinishServlet3

# put them into web.xml
cat $SERVLET1 >> $FILE
cat $SERVLET2 >> $FILE
cat $SERVLET3 >> $FILE

echo "  <servlet-mapping>" >> $FILE
echo "    <servlet-name>" >> $FILE
echo "      web" >> $FILE
echo "    </servlet-name>" >> $FILE
echo "    <url-pattern>" >> $FILE
echo "      /web" >> $FILE
echo "    </url-pattern>" >> $FILE
echo "  </servlet-mapping>" >> $FILE
echo "  <servlet-mapping>" >> $FILE
echo "    <servlet-name>" >> $FILE
echo "      palm" >> $FILE
echo "    </servlet-name>" >> $FILE
echo "    <url-pattern>" >> $FILE
echo "      /palm" >> $FILE
echo "    </url-pattern>" >> $FILE
echo "  </servlet-mapping>" >> $FILE
echo "  <taglib>" >> $FILE
echo "    <taglib-uri>iBrowseTags</taglib-uri>" >> $FILE
echo "    <taglib-location>/WEB-INF/taglib.tld</taglib-location>" >> $FILE
echo "  </taglib>" >> $FILE
echo "</web-app>" >> $FILE

# cleanup
rm $SERVLET1
rm $SERVLET2
rm $SERVLET3

