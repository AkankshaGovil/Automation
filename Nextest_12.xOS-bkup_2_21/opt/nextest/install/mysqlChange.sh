
echo "Configuring Mysql for openser db authentication"
sudo /etc/init.d/mysql start

pid=`ps -ef | grep mysqld | grep -v grep | awk '{print $2}'| wc -l`

if [ $pid == "0" ] 
then
	echo "Unable to start Mysql, Please check if mysql is installed on the machine" 
	exit
fi


echo "Deleting the existing openser database"
echo "MySQL password for root:"
read pwd
mysql -e "DROP DATABASE openser" -u root -p$pwd

if [ $? != 0 ]
then
    echo " Error deleting the Openser database, Please verify the password and run this script again"
fi


echo "Creating Openser Database"
cd /opt/nextest/bin/
./openserdbctl create
if [ $? != 0 ]
then
	echo " "
    echo " Error creating Openser database, Please verify and run this script again"

fi

cd /opt/nextest/install

