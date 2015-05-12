"""
NAT resource for call testing.
"""
import qm.common
import qm.fields
from   qm.test.resource import *
import session
import posix
import socket

class Nat(Resource):
    """
    NAT resource.

    This resource will create a NAT which masquerades devices behind NAT for
    public and private realms. Since this resource does not take any parameter
    this resources class should b instantiated only once.
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
            )
        ]

    def SetUp(self, context, result):
        """
        Create NAT table rules to convert fixed internal addresses into fixed external addresses. 
        """
        #get nat addresses
        pubNat = socket.gethostbyname('pub_nat')
        prvNat = socket.gethostbyname('prv_nat')
        # 54804: Add new nat device prv_nat1
        prvNat1 = socket.gethostbyname('prv_nat1')
        # 38969: Add a new NAT device pubNat1
        pubNat1 = socket.gethostbyname('pub_nat1')
        #pubNat2 = socket.gethostbyname('pub_nat2')
        #prvNat2 = socket.gethostbyname('prv_nat2')
        if (len(pubNat) < 8) or (len(prvNat) < 8) or (len(pubNat1) < 8):
            raise EnvironmentError,'Check pub_nat/prv_nat addresses in /etc/hosts file'

        #self.intAddresses = { '192.168.0.100':'0',
        #                      '192.168.0.101':'0',
        #                      '192.168.1.100':'1',
        #                      '192.168.1.101':'1',
        #                    }

        # 38969: New NAT device pubNat1
        # 54804: Add new nat device prv_nat1
        self.ExtAddresses = [pubNat, prvNat, pubNat1, prvNat1]
        self.If = [context['userConfig.eth_pub_iface'],context['userConfig.eth_pri_iface'],\
                   context['userConfig.eth_pub_iface'],context['userConfig.eth_pri_iface']] 
        self.shell = session.LocalShell()
        
        #get os type
        ostype = posix.uname()[0]

        if ostype == 'Linux':
            # Add the NAT endpoint addresses
            # 38969: Add the new nat device
            # 54804: Add new nat device prv_nat1
            for ipAddr in range(0,4):
                self.shell.assertCommand('sudo ip addr add %s/16 dev eth%s > /dev/null 2>&1' \
                                % (self.ExtAddresses[ipAddr], self.If[ipAddr]))

            # Add NAT configuration
            # 38969: Set NAT rules for the new nat device
            commandLine = 'sudo iptables -t nat -A POSTROUTING -p udp -s 192.168.0.100/24 -j SNAT ' + \
                            '--to-source %s:20000-25000' % pubNat
            self.shell.assertCommand(commandLine)
            commandLine = 'sudo iptables -t nat -A POSTROUTING -p udp -s 192.168.2.100/24 -j SNAT ' + \
                            '--to-source %s:25001-29999' % pubNat1
            self.shell.assertCommand(commandLine)
            # 43494: Modified the port range
            commandLine = 'sudo iptables -t nat -A POSTROUTING -p udp -s 192.168.1.100/24 -j SNAT ' + \
                            '--to-source %s:30000-35000' % prvNat
            self.shell.assertCommand(commandLine)

            # 43494: Add NAT rule for tcp
            commandLine = 'sudo iptables -t nat -A POSTROUTING -p tcp -s 192.168.1.100/24 -j SNAT ' + \
                            '--to-source %s:35001-39999' % prvNat
            self.shell.assertCommand(commandLine)

            # 51005: Add NAT TCP rule for pubNat    
            commandLine = 'sudo iptables -t nat -A POSTROUTING -p tcp -s 192.168.0.100/24 -j SNAT ' + \
                            '--to-source %s:40000-45000' % pubNat
            self.shell.assertCommand(commandLine)

            #PR 137224 Add rules for new NAt device in nat_enhancements
            commandLine = 'sudo iptables -t nat -A POSTROUTING -p udp -s 192.168.3.100/24 -j SNAT ' + \
                            '--to-source %s:20000-25000' % prvNat1
            self.shell.assertCommand(commandLine)

        elif ostype == 'SunOS':
            #Package installation should be done at the install time.
            #self.shell.assertCommand('cd /opt/nextest/etc')
            #self.shell.assertCommand('sudo pkgrm nsf')
            #self.shell.assertCommand('sudo pkgadd -d ipfilter_3.4.27_stock.pkg')
            # Add the NAT endpoint addresses
            for ipAddr in 0,1:
                self.shell.assertCommand( 'sudo ifconfig e1000g%s addif %s/16 up > /dev/null 2>&1' \
                                % (self.If[ipAddr],self.ExtAddresses[ipAddr]))

            # Add nat configuration in /etc/opt/ipf/ipnat.conf
            fileName =  '/etc/opt/ipf/ipnat.conf'
            try:
    	        natFile = open(fileName,'r')
    	        natFileLines = natFile.readlines()
    	        natFile.close()
            except:
                #file does not exist, we will create it later
    	        natFileLines = []
            
            # Ticket 14783 changes
	    self.solNatNewLines = [
                 'map e1000g%s 192.168.0.0/16 -> %s/32 portmap udp 20001:30000\n' % (context['pub_eth_iface'],pubNat),
                 'map e1000g%s 192.168.1.0/16 -> %s/32 portmap udp 30001:40000\n' % (context['pri_eth_iface'],prvNat),
                 ]

	    natFileLinesCleaned = [x for x in natFileLines if x not in self.solNatNewLines]

            self.shell.assertCommand('sudo touch %s' % fileName)
            self.shell.assertCommand('sudo chmod 777 %s' % fileName)
            natFile = open(fileName,'w')
            natFile.writelines(natFileLinesCleaned)
            natFile.writelines(self.solNatNewLines)
            natFile.close()
            self.shell.assertCommand('sudo chmod 644 %s' % fileName)

            self.shell.assertCommand('sudo /etc/init.d/ipfboot stop')
            self.shell.assertCommand('sudo /etc/init.d/ipfboot start')

        else:
            raise NotImplementedError,'Nat resource not implemented for %s' %ostype

        context[self.property] = 'created'

    def CleanUp(self, result):

        #get os type
        ostype = posix.uname()[0]

        if ostype == 'Linux':
            # Removing the IP addresses
            # 38969: Remove the new nat device
            # 54804: Remove new nat device prv_nat1
            for ipAddr in range(0,4):
                self.shell.assertCommand('sudo ip addr delete %s/16 dev eth%s > /dev/null 2>&1' \
                                % (self.ExtAddresses[ipAddr], self.If[ipAddr]))

            # We are flushing the chains assuming that this class is only one 
            # which adds nat rules.
            self.shell.assertCommand('sudo iptables -t nat -F POSTROUTING')

        elif ostype == 'SunOS':
            # Removing the IP addresses
            for ipAddr in 0,1:
                self.shell.assertCommand( 'sudo ifconfig e1000g%s removeif %s/16 up > /dev/null 2>&1' \
                                % (self.If[ipAddr],self.ExtAddresses[ipAddr]))

            # Remove nat configuration in /etc/opt/ipf/ipnat.conf
            fileName =  '/etc/opt/ipf/ipnat.conf'
            try:
    	        natFile = open(fileName,'r')
    	        natFileLines = natFile.readlines()
    	        natFile.close()
            except:
                #file does not exist, nothing to do
    	        return

	    natFileLinesCleaned = [x for x in natFileLines if x not in self.solNatNewLines]

            self.shell.assertCommand('sudo chmod 777 %s' % fileName)
            natFile = open(fileName,'w')
            natFile.writelines(natFileLinesCleaned)
            natFile.close()
            self.shell.assertCommand('sudo chmod 644 %s' % fileName)

            self.shell.assertCommand('sudo /etc/init.d/ipfboot stop')
            self.shell.assertCommand('sudo /etc/init.d/ipfboot start')

        else:
            raise NotImplementedError,'Nat resource not implemented for %s' %ostype

        self.shell.disconnect()
