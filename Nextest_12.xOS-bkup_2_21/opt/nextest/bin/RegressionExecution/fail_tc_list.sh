cat $1|egrep -i "fail|error|untest"|grep -i production|awk -F: '{print $1}'|sort|uniq > fail_tc_list
sed -e 's/  production_components.//' fail_tc_list
