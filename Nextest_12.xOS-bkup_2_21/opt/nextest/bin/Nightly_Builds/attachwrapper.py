#! /usr/bin/python2.3 

import os
import string
import sys
import emailattach
import pdb
def main() :
    fromaddr = ""
    toaddr = ""
    mailserver = ""
    toaddrs = ""
    count = 0
    cmpValue = 0
    #pdb.set_trace()
    try:
        config_file = open("mailConfig.cfg", 'r')
    except IOError:
        print 'Can\'t open config file for reading, So mail will not be sent.'
        sys.exit(0)
    fileList = config_file.readlines()
    for linestr in fileList:
        lineStr = linestr[:-1]
        if lineStr.startswith('fromaddr') :
           fromaddr = getValue('fromaddr',lineStr)
        if lineStr.startswith('toaddr') :
           toaddr = getValue('toaddr',lineStr)
           toaddrs = toaddr.split(',')
        if lineStr.startswith('mailserver') :
           mailserver = getValue('mailserver',lineStr)
        if lineStr.startswith('userid') :
           userid = getValue('userid',lineStr)
        if lineStr.startswith('passwd') :
           passwd = getValue('passwd',lineStr)
        count = count + 1
    try : 
        mailobj = emailattach.EmailAttach(sys.argv[2],toaddrs,fromaddr,mailserver,userid,passwd)
        mailobj.emailAttachment(sys.argv[1])
    except IOError:
        print '\nPlease provide the Subject and Path of file to sent while calling the script attachwrapper.py \n'
        sys.exit(0)
    print "\nDone"



def getValue(parameter,line):
    """
    This function returns the value of each parameter
    after spliting from '='
    """

    str = line.split('=')
    return str[1].strip()




if __name__ == "__main__" :
     main()

