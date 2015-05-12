"""
Endpoint resource for call testing.
"""
import qm.common
import qm.fields
from   qm.test.resource import *
import gen
import posix
import logging

__version__ = '$Id: endpoint.py,v 1.3 2005/09/08 20:41:16 smallonee_c Exp $'

class Endpoint(Resource):
    """
    Generic endpoint resource.

    This resource contains the minimum attributes necessary to
    represent a VOIP endpoint on the network such as IP address.

    Higher level attributes such as protocol type are not defined at
    setup time.

    If this resource is dependent on a session resource, that session
    will be associated with the endpoint.  This lets you control
    remote endpoints.  Otherwise, the endpoint is instantiated on the
    local machine.
    """
    arguments = [
        qm.fields.TextField(
            name="ipaddr",
            title="IP Address",
            not_empty_text=True,
            description="""
            The IP address of the endpoint.

            This is the source address of outgoing packets from this
            endpoint.  If an Ethernet interface does not exist for
            this address, it will be created.
            """
            ),
        qm.fields.TextField(
            name="mask",
            title="Net Mask",
            default_value = None,
            description="""
            The network mask (optional).

            Enter this field only if creating a new Ethernet interface
            """
            ),        
        qm.fields.TextField(
            name='ethport',
            title='Ethernet Interface Name',
            default_value = None,
            description="""
            The name of the interface to bind the IP address to. (optional)

            Enter this field only if creating a new Ethernet interface
            """
            ),
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
            )
        ]

    def SetUp(self, context, result):
        """
        Set up the resource.

        Create a generic endpoint object.  Resolve the address prior to
        creating the gen.
        """
        self.log = logging.getLogger('nextestlog'):
        self.log.debug('Creating endpoint resource: property: %s, ip: %s, mask: %s, eth: %s' % \
                                  (self.property, self.ipaddr, self.mask, self.ethport))

        self.ostype = posix.uname()[0]
        ipm = ipsetup.IPManager()
        self.network = ipm.getNetwork(ipaddr)
        self.ipAddr = socket.gethostbyname(ipaddr)


        if self.network == '-1':
            raise OSError, 'IP address %s not found in ipsetup.py' % ipaddr

        if self.ostype == 'Linux':
                self.shell.assertCommand('sudo ip addr add %s/24 dev eth%s > /dev/null 2>&1' \
                                % (self.ipAddr, self.network)
        elif self.ostype == 'SunOS':
                self.shell.assertCommand( 'sudo ifconfig e1000g%s addif %s/24 up > /dev/null 2>&1' \
                                % (self.network,self.ipAddr)
        else:
            raise NotImplementedError,'Endpoint resource not implemented for %s' %self.ostype

        #Create a endpoint object.
        ep = gen.Endpoint(self.ipaddr, self.mask, self.ethport)
        context[self.property] = ep

    def CleanUp(self, result):
        self.log.debug('Cleaning endpoint resource: property: %s, ip: %s, mask: %s, eth: %s' % \
                                  (self.property, self.ipaddr, self.mask, self.ethport))

        if self.ostype == 'Linux':
                self.shell.assertCommand('sudo ip addr del %s/24 dev eth%s > /dev/null 2>&1' \
                                % (self.ipAddr, self.network)
        elif self.ostype == 'SunOS':
                self.shell.assertCommand( 'sudo ifconfig e1000g%s removeif %s/24 > /dev/null 2>&1' \
                                % (self.network,self.ipAddr)
        else:
            raise NotImplementedError,'Endpoint resource not implemented for %s' %self.ostype
