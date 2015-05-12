#! /usr/bin/env Python

import os
import string
import sys
import mailer
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
        count = count + 1
    
    mailobj = mailer.Mailer(toaddrs,fromaddr,mailserver)
    #print "the value is =>%s" %sys.argv[1]
    #print "the value is => %s" %sys.argv[2]
    #sub = "problem with the MSW installation script"
    #msg = "\n\rNot able to install\n\r Regards\n\r SIT Team\n\r"
    #mailobj.sendMail(sub,msg)
    mailobj.sendMail(sys.argv[1],sys.argv[2])
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

