#!/usr/bin/python
"""
Control resources for test automation.

This class is used to establish contact with the RADIUS server.
"""
import pexpect
import qm.common
import qm.fields
from   qm.test.resource import *
import logging
from pyrad import radserver
import socket
import posix
import ipsetup
import session
import os 

class RadiusServer(Resource):
    """
    RADIUS server resource.

    This resource contains the minimum attributes necessary to
    represent a RADIUS server.

    """
    arguments = [
        qm.fields.TextField(
            name='property',
            title='Property Name',
            not_empty_text = True,
            description="""
            The name of the context property that will be used to refer to
            this endpoint in your tests.

            For example, if you enter 'myendpoint', you can refer to the
            session as "context['myendpoint']" in your test code.  The
            value of this property is a handle to an Endpoint object.
            """
            ),
        qm.fields.TextField(
            name="ipaddr",
            title="RADIUS server IP address/FQDN",
            not_empty_text=True,
            description="""
            """
            ),
        qm.fields.TextField(
            name="msw_ip",
            title="Add MSW IP/FQDN as RADIUS server user",
            not_empty_text=True,
            description="""
            """
            ),
        qm.fields.TextField(
            name="shared_secret",
            title="Shared secret to access the RADIUS server",
            not_empty_text=True,
            description="""
            """
            )
        ]

    def SetUp(self, context, result):
        """
        This method is over written in the derived class
        """
        self.virtualIp = VirtualIpAddress(self.ipaddr,context)
        ipaddress = getIp(self.ipaddr,result)
        User_msw = getIp(self.msw_ip,result)
        
	#PR 133009 Adding path variable to write the radius packets
	path = context['nextest.result_path']
        self.RadiusSvr = radserver.Radserver(ipaddress,path) 
        context[self.property] = self.RadiusSvr
        self.RadiusSvr.AddUser(User_msw, self.shared_secret)
       
        #PR 153229- Adding support for bkupmsw 
        if (context['nextest.scm_configuration'] == 'ON'):
            User_bkup=getIp("bkupmsw",result)
            self.RadiusSvr.AddUser(User_bkup, self.shared_secret)

        self.RadiusSvr.Start()

    def CleanUp(self, result):
        """
        This method is over written in the derived class
        """
        self.RadiusSvr.Stop()
        self.virtualIp = None

################## private method ###############################

def getIp(addr,result):
    if addr.isalpha():
        try:
            ip = socket.gethostbyname(addr)
	except (socket.gaierror, exc):        	
            result.Annotate({'Resource name' : self.property,
                        'IP address' : self.ipaddr,
                        'Exception' : str(exc)})
            result.Fail('Could not resolve IP address')
            self.log.error('Could not resolve IP address')
    else:
        try:
            socket.gethostbyaddr(addr)
            ip = addr
        except socket.gaierror, exc:
            result.Annotate({'Resource name' : self.property,
                        'IP address' : self.ipaddr,
                        'Exception' : str(exc)})
            result.Fail('Could not resolve IP address')
            self.log.error('Could not resolve IP address')
    return(ip)


class VirtualIpAddress:
    def __init__(self, ipAddr,context):

        if not ipAddr.isalpha():
            raise EnvironmentError, "control-rad.py: Expected host name, got address"

        self.shell = session.LocalShell()
        self.ostype = posix.uname()[0]
        ipm = ipsetup.IPManager()

        # 22726 - Radius Access Response needs to be received by the MSW on the same interface on
        # on which the request was sent. Radius Request messages will be sent on the management
        # interface.
        self.network = context['userConfig.radius_eth_iface']
        self.ipAddr = socket.gethostbyname(ipAddr)

        if self.network == '-1':
            raise OSError, 'IP address %s not found in ipsetup.py' % self.ipAddr

        if self.ostype == 'Linux':
                self.shell.assertCommand('sudo ip addr add %s/16 dev eth%s > /dev/null 2>&1' \
                                % (self.ipAddr, self.network))
        elif self.ostype == 'SunOS':
                self.shell.assertCommand( 'sudo ifconfig e1000g%s addif %s/16 up > /dev/null 2>&1' \
                                % (self.network,self.ipAddr))
        else:
            raise NotImplementedError,'Endpoint resource not implemented for %s' %self.ostype

    def __del__(self):
        if self.ostype == 'Linux':
                self.shell.assertCommand('sudo ip addr del %s/16 dev eth%s > /dev/null 2>&1' \
                                % (self.ipAddr, self.network))
        elif self.ostype == 'SunOS':
                self.shell.assertCommand( 'sudo ifconfig e1000g%s removeif %s > /dev/null 2>&1' \
                                % (self.network, self.ipAddr))
        else:
            raise NotImplementedError,'Endpoint resource not implemented for %s' %self.ostype

        self.shell.disconnect()


if __name__ == '__main__':
    pass

########################################################################
# Local Variables:
# mode: python
# indent-tabs-mode: nil
# fill-column: 78
# auto-fill-function: do-auto-fill
# End:
