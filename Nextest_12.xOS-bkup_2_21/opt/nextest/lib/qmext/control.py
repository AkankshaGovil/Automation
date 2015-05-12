"""
Control resources for test automation.

This class is used to establish contact with the MSW.  It can be used
to connect with other hosts if necessary.
"""
import dmalloc_env
import msw
import qm.common
import qm.fields
from   qm.test.resource import *
from   session import *
import realm
import socket
import logging
import ipsetup
import posix
import gatekeeper
import astproxy,os,commands,opensips
import openser

class Session(Resource):
    """
    A resource that sets up a control session to a test system.

    Tests that depend on this resource will have an SSH session
    set up when the test begins.  Test scripts can use this session
    for running call generators or gathering system information
    during a test.

    The property name field is the link between the resource and the
    test.  The context dictionary will contain the name you specify; the
    value associated with that name is a handle to the session object
    created by this resource.
    """
    arguments = [
        qm.fields.TextField(
            name="host",
            title="Host Name/Address",
            not_empty_text=True,
            description="""
            The host name or IP address of the system to be controlled.
            """
            ),
        qm.fields.TextField(
            name='user',
            title='User Name',
            not_empty_text=True,
            description="""
            The user name to log in with.  This account should be set up
            for a no-password login.
            """
            ),
         qm.fields.TextField(
            name='property',
            title='Property Name',
            not_empty_text=True,
            description="""
            The name of the context property that will be used to refer to
            this session in your tests.

            For example, if you enter 'mysession', you can refer to the
            session as "context['mysession']" in your test code.
            """
            )
       ]

    def SetUp(self, context, result):
        """
        Set up the control resource.

        Tests that are not Pythonic, such as ShellScriptTest, may
        also require access to this resource.  Context variables
        that are not strings will not be defined in the shell
        environment, so the above session object cannot be used by
        shell tests.  For this reason, a simple login string will be
        defined so that the shell environments can access the
        resource.

        The environment variable for shells will be

            QMV_<propname>_login

        where propname is the property name defined by the user.
        """
        self.log = logging.getLogger('nextestlog')
        login = self.user + '@' + self.host
        try:
            self.log.debug('control.Session: args: "%s"' % login)
            self.session = SSH([login], ctxt=context)
            context[self.property] = self.session
        except SessionException, exc:
            result['Reason'] = str(exc)
            result.Fail('SSH failed to connect to %s' % login)
        
        loginStrForShells = self.property + '_login'
        context[loginStrForShells] = str(login)

    def CleanUp(self, result):
        if self.session.isConnected:
            self.session.disconnect()

class Realm(Resource):
    """
    Generic endpoint resource.

    This resource contains the minimum attributes necessary to
    represent a Realm such as RSA address.

    """
    arguments = [
        qm.fields.TextField(
            name="ipaddr",
            title="RSA Address",
            not_empty_text=True,
            description="""
            """
            ),
        qm.fields.TextField(
            name="mask",
            title="Net Mask",
            default_value = '255.255.255.0',
            description="""
            """
            ),        
        qm.fields.TextField(
            name='property',
            title='Realm Name',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='type',
            title='Realm Type (private/public)',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='sigpool',
            title='Signalling Pool Id',
            default_value = 'None',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='medpool',
            title='Media Pool Id',
            default_value = 'None',
            not_empty_text = True,
            description="""
            """
            )
        ]

    def SetUp(self, context, result):
        """
        Set up the resource.

        Create a realm object.
        """

        self.log = logging.getLogger('nextestlog')

        if len(self.mask) == 0: self.mask = None

        ipm = ipsetup.IPManager()
        self.ethport = ipm.getNetwork(self.ipaddr)

        self.log.debug('control.Realm: Ethport set to %s for %s' % (self.ethport, self.ipaddr))

        if (self.ethport == '-1'):
            result.Fail('Could not resolve realm ethport')

        try:
            ip = socket.gethostbyname(self.ipaddr)
        except socket.gaierror, exc:
            errno, errstr = exc
            result.Annotate({'Realm name' : self.property,
                             'Realm RSA' : self.ipaddr,
                             'Exception' : str(exc)})
            result.Fail('Could not resolve realm RSA IP address')
        else:
            self.ipaddr = ip
            _realm = realm.Realm(self.ipaddr, self.mask, self.ethport,
                                 self.property, self.type, self.sigpool,
                                 self.medpool)
            context[self.property] = _realm

class Dmalloc_Env(Resource):
    """
    Resource for initializing and maintaining values specific to individual 
    dmalloc testing environments.

    This resource contains attributes necessary to:
        1) Communicate via SSH to and from the MSW and Call Generator
        2) Locate supporting source code (scripts) for dmalloc testing
        3) Locate special dmalloc gis builds for dmalloc testing
        4) Locate MSW and Call Generator temp working direcries
        5) Maintain current the test suite name during test execution
    """
    arguments = [
        qm.fields.TextField(
            name='property',
            title='Env Name',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='gen',
            title='Call Generator ssh Login String',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='csn',
            title='Current Suite Name Start Value',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='gen_builddir',
            title='Call Generator Build Directory path',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='gen_srcdir',
            title='Call Generator Source Code Directory path',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='gen_resdir',
            title='Call Generator Test Results Directory path',
            not_empty_text = True,
            description="""
            """
            )
        ]

    def SetUp(self, context, result):
        """
        Set up the resource.
        """
        if len(self.gen) == 0: self.gen = None
        if len(self.csn) == 0: self.csn = None
        #    def __init__(self, name, gen, csn, gen_builddir, gen_srcdir, gen_resdir):
        _dmalloc_env = dmalloc_env.Dmalloc_Env(self.property, self.gen, self.csn, self.gen_builddir, self.gen_srcdir, self.gen_resdir)
        context[self.property] = _dmalloc_env

class MSW(Resource):

    arguments = [
        qm.fields.TextField(
            name='mswname',
            title='MSW Host Name or IP Address',
            default_value = 'mymsw',
            not_empty_text = True,
            description="The name or IP address of the MSW."
            ),
        qm.fields.TextField(
            name='userpcapfilter',
            title='Additional Capture Filter',
            not_empty_text = False,
            description="""Additional capture filters (use tcpdump format). 

               Suggested contents: Public and private interface addresses 
               on both the MSW and the Call Generator. The addresses should 
               be " or "-delimited, as shown in the following string, (which 
               is compliant with tcpdump capture filter "expression" for list 
               of hosts).
            """
            )
        ]

    def SetUp(self, context, result):
        """
        Create an object of class msw.MSWInfo to contain MSW state 
        information, and place it in the test context.
        """
        context['mswinfo'] = msw.MSWInfo(self.mswname)
        context['userpcapfilter'] = self.userpcapfilter
       
#SCM testing addon
class BkupMSW(Resource):

    arguments = [
        qm.fields.TextField(
            name='bkupmsw',
            title='Backup MSW Host Name or IP Address',
            default_value = 'bkupmsw',
            not_empty_text = True,
            description="The name or IP address of the Backup MSW."
            ),
        qm.fields.TextField(
            name='userpcapfilter',
            title='Additional Capture Filter',
            not_empty_text = False,
            description="""Additional capture filters (use tcpdump format). 

               Suggested contents: Public and private interface addresses 
               on both the MSW and the Call Generator. The addresses should 
               be " or "-delimited, as shown in the following string, (which 
               is compliant with tcpdump capture filter "expression" for list 
               of hosts).
            """
            )
        ]

    def SetUp(self, context, result):
        """
        Create an object of class msw.MSWInfo to contain MSW state 
        information, and place it in the test context.
        """
        context['bkupinfo'] = msw.MSWInfo(self.bkupmsw)
        context['userpcapfilter'] = self.userpcapfilter
        
class LocalServer(Resource):
    """
    A resource that starts up a locally-running server on the NexTest system.
    """
    arguments = [
        qm.fields.TextField(
            name           = 'startcommand',
            title          = 'Command to start the Local Server',
            not_empty_text = True,
            description    = """
            Complete command with path and options if needed, (or script), to start the LocalServer.
            """
            ),
        qm.fields.TextField(
            name           = 'stopcommand',
            title          = 'Command to stop the LocalServer',
            not_empty_text = False,
            description    = """
            Complete command with path and options if needed, (or script), to stop the LocalServer.
            """
            ),
         qm.fields.TextField(
            name           = 'property',
            title          = 'Property Name',
            not_empty_text = True,
            description    = """
            The name of the context property that will be used to refer to
            this LocalServer in your tests.
            """
            )
       ]

    def SetUp(self, context, result):
        """
        Set up the LocalServer resource.
        """
        self.log = logging.getLogger('nextestlog')
        cmd = self.startcommand

        # 22519 - Verify whether its a SER resource and if so, add the ip address to the
        # public interface
        if cmd.__contains__("run_ser"): 
            shell = LocalShell() 
            ostype = posix.uname()[0]
            ser_ip = socket.gethostbyname('sipproxy')
            interface = context['userConfig.eth_pub_iface']

            ipCmd = "" 
            ipAddCmd = ""
            if ostype == 'Linux':
                ipCmd = 'sudo ip addr show | grep %s' %ser_ip
                ipAddCmd = 'sudo ip addr add %s/24 dev eth%s > /dev/null 2>&1' %(ser_ip,interface)
            elif ostype == 'SunOS':
                ipCmd = 'sudo ifconfig -a | grep %s' %ser_ip
                ipAddCmd = 'sudo ifconfig addif e1000g%s %s/24 up > /dev/null 2>&1' %(interface,ser_ip)

            # Find if the ip already exists
            resultString = shell.filter(ipCmd)
            if not resultString.__contains__(ser_ip):
                shell.assertCommand(ipAddCmd)
                # Find if the ip has been successfully added
                resultString = shell.filter(ipCmd)
                if not resultString.__contains__(ser_ip):
                    raise OSError, 'IP address %s could not be added to interface %s' %(ser_ip,interface)
                else:
                    self.log.debug('control.LocalServer: SER IP address %s added to interface %s' %(ser_ip,interface))
            else:
                self.log.debug('control.LocalServer: SER IP address %s already added' %ser_ip)   

        self.log.debug('control.LocalServer: Starting %s, command: %s' % (self.property, cmd))
        os.system(cmd)
        shell.disconnect()
        
    def CleanUp(self, result):
        self.log.debug('LocalServer: Cleaning up %s' % (self.property))
        if self.stopcommand:
            cmd = self.stopcommand
            self.log.debug('LocalServer: Cleaning up %s, command: %s' % (self.property, cmd))
            os.system(cmd)
        
class gnuGatekeeper(Resource):
    """
    A resource that starts up a gnuGatekeeper.
    """
    arguments = [
        qm.fields.TextField(
            name='gkname',
            title='GateKeeper Host Name or IP Address',
            default_value = '',
            not_empty_text = True,
            description="The name or IP address of the gnuGateKeeper."
            )

       ]

    def SetUp(self, context, result):
        """
          Set up the gnuGatekeeper resource.
        """
        self.log = logging.getLogger('nextestlog')
        self.ipAddr = socket.gethostbyname(self.gkname)

        if self.gkname == 'gatekeeper1':
            interface = context['userConfig.eth_pub_iface']
        else:
             interface = context['userConfig.eth_pri_iface'] 
        ipCmd = ""
        ipAddCmd = ""
        shell = LocalShell()
        ostype = posix.uname()[0]
        if ostype == 'Linux':
            ipCmd = 'sudo ip addr show | grep %s' %self.ipAddr
            ipAddCmd = 'sudo ip addr add %s/24 dev eth%s > /dev/null 2>&1' %(self.ipAddr,interface)
        elif ostype == 'SunOS':
            ipCmd = 'sudo ifconfig -a | grep %s' %self.ipAddr
            ipAddCmd = 'sudo ifconfig addif e1000g%s %s/24 up > /dev/null 2>&1' %(interface,self.ipAddr)

        # Find if the ip already exists
        resultString = shell.filter(ipCmd)
        if not resultString.__contains__(self.ipAddr):
            shell.assertCommand(ipAddCmd)
            # Find if the ip has been successfully added
            resultString = shell.filter(ipCmd)
            if not resultString.__contains__(self.ipAddr):
                raise OSError, 'GK IP address %s could not be added to interface %s' %(self.ipAddr,interface)
            else:
                self.log.debug('control.LocalServer: GK IP address %s added to interface %s' %(self.ipAddr,interface))
        else:
            self.log.debug('control.LocalServer: GK IP address %s already added' %self.ipAddr)
        if self.gkname == 'gatekeeper2':
            context[self.gkname] = gatekeeper.gnuGk(self.gkname,switch='prv_rsa')
        else:
            context[self.gkname] = gatekeeper.gnuGk(self.gkname)
        self.gnuGKObj = context[self.gkname]
        self.gnuGKObj.addr = self.ipAddr


# 43887 - Add new resource for ASTERISK
class asterisk(Resource):
    """
    ASTERISK sip proxy resource
    """
    arguments = [
        qm.fields.TextField(
            name='name',
            title='Asterisk Host Name or IP Address',
            default_value = '',
            not_empty_text = True,
            description="The name or IP address of the asterisk proxy server"
            )

       ]

    def SetUp(self, context, result):
        """
          Set up the asterisk resource.
        """
        self.log = logging.getLogger('nextestlog')
        self.ipAddr = socket.gethostbyname(self.name)

        self.interface = context['userConfig.eth_pub_iface']

        ipCmd = ""
        ipAddCmd = ""
        self.shell = LocalShell()
        ipCmd = 'sudo ip addr show | grep %s' %self.ipAddr

        # Find if the ip already exists
        resultString = self.shell.filter(ipCmd)
        if not resultString.__contains__(self.ipAddr):
            ipAddCmd = 'sudo ip addr add %s/24 dev eth%s > /dev/null 2>&1' %(self.ipAddr,self.interface)
            self.shell.assertCommand(ipAddCmd)

            # Find if the ip has been successfully added
            resultString = self.shell.filter(ipCmd)
            if not resultString.__contains__(self.ipAddr):
                raise OSError, 'Asterisk IP address %s could not be added to interface %s' %(self.ipAddr,self.interface)
            else:
                self.log.debug('control.asterisk: Asterisk IP address %s added to interface %s' %(self.ipAddr,self.interface))
        else:
            self.log.debug('control.asterisk: Asterisk IP address %s already added' %self.ipAddr)

        self.prox = astproxy.asteriskProxy(self.ipAddr)
        context[self.name] = self.prox

    def CleanUp(self, result):
        self.log.debug('Asterisk: Cleaning up %s' % (self.name))
        tmpstr1 = commands.getoutput('pgrep -x asterisk')
        if tmpstr1:
            # Kill the asterisk process if it is already running
            os.system('sudo pkill asterisk')
            self.log.debug('Asterisk.CleanUp: Killed the asterisk process')

        # Delete the ip address
        ipDelCmd = 'sudo ip addr delete %s/24 dev eth%s > /dev/null 2>&1' %(self.ipAddr,self.interface)
        self.shell.assertCommand(ipDelCmd) 
        self.shell.disconnect()

# 57621 - Add new resource for OPENSIPS
class opensipsServer(Resource):
    """
    OPENSIPS sip proxy resource
    """
    arguments = [
        qm.fields.TextField(
            name='name',
            title='OpenSips Proxy Host Name or IP Address',
            default_value = '',
            not_empty_text = True,
            description="The name or IP address of the OpenSips proxy server"
            )

       ]

    def SetUp(self, context, result):
        """
          Set up the opensips resource.
        """
        self.log = logging.getLogger('nextestlog')
        self.ipAddr = socket.gethostbyname(self.name)

        self.interface = context['userConfig.eth_pub_iface']

        ipCmd = ""
        ipAddCmd = ""
        self.shell = LocalShell()
        ipCmd = 'sudo ip addr show | grep %s' %self.ipAddr

        # Find if the ip already exists
        resultString = self.shell.filter(ipCmd)
        if not resultString.__contains__(self.ipAddr):
            ipAddCmd = 'sudo ip addr add %s/24 dev eth%s > /dev/null 2>&1' %(self.ipAddr,self.interface)
            self.shell.assertCommand(ipAddCmd)

            # Find if the ip has been successfully added
            resultString = self.shell.filter(ipCmd)
            if not resultString.__contains__(self.ipAddr):
                raise OSError, 'Opensips IP address %s could not be added to interface %s' %(self.ipAddr,self.interface)
            else:
                self.log.debug('control.opensipsSErver: Opensips IP address %s added to interface %s' %(self.ipAddr,self.interface))
        else:
            self.log.debug('control.opensipsServer: Asterisk IP address %s already added' %self.ipAddr)

        self.prox = opensips.opensipsProxy(self.ipAddr)
        context[self.name] = self.prox

    def CleanUp(self, result):
        self.log.debug('Opensips: Cleaning up %s' % (self.name))
        tmpstr1 = commands.getoutput('pgrep -x opensips')
        if tmpstr1:
            # Kill the opensips process if it is already running
            os.system('sudo pkill opensips')
            self.log.debug('Opensips.CleanUp: Killed the opensips process')

        # Delete the ip address
        ipDelCmd = 'sudo ip addr delete %s/24 dev eth%s > /dev/null 2>&1' %(self.ipAddr,self.interface)
        self.shell.assertCommand(ipDelCmd)

        # Delete the config file
        self.shell.assertCommand('sudo rm -rf %s' %self.prox.confFile)
        self.shell.disconnect()

# 58638 - Add a new resource for Openser 
class openSerServer(Resource):
    """
    OPENSER sip proxy resource
    """
    arguments = [
        qm.fields.TextField(
            name='name',
            title='OpenSer Proxy Host Name or IP Address',
            default_value = '',
            not_empty_text = True,
            description="The name or IP address of the OpenSer proxy server"
            ),
	
        qm.fields.TextField(
            name='domain',
            title='OpenSer Proxy Domain',
            default_value = '',
            not_empty_text = True,
            description="The name or IP address of the OpenSer proxy server"
            ),
	
        qm.fields.TextField(
            name='authentication',
            title='OpenSer DB authenticaion',
            default_value = 'OFF',
            not_empty_text = False,
            description="The name or IP address of the OpenSer proxy server"
	    )

       ]

    def SetUp(self, context, result):
        """
          Set up the openser resource.
        """
        self.log = logging.getLogger('nextestlog')
        self.ipAddr = socket.gethostbyname(self.name)

        self.interface = context['userConfig.eth_pub_iface']
        self.log.debug("Setting up openser resource")

        ipCmd = ""
        ipAddCmd = ""
        self.shell = LocalShell()
        ipCmd = 'sudo ip addr show | grep %s' %self.ipAddr

        # Find if the ip already exists
        resultString = self.shell.filter(ipCmd)
        if not resultString.__contains__(self.ipAddr):
            ipAddCmd = 'sudo ip addr add %s/24 dev eth%s > /dev/null 2>&1' %(self.ipAddr,self.interface)
            self.shell.assertCommand(ipAddCmd)

            # Find if the ip has been successfully added
            resultString = self.shell.filter(ipCmd)
            if not resultString.__contains__(self.ipAddr):
                raise OSError, 'Openser IP address %s could not be added to interface %s' %(self.ipAddr,self.interface)
            else:
                self.log.debug('control.openserServer: Openser IP address %s added to interface %s' %(self.ipAddr,self.interface))
        else:
            self.log.debug('control.openserServer: Openser IP address %s already added' %self.ipAddr)

        self.prox = openser.openSerProxy(self.ipAddr, self.domain, self.authentication)
        context[self.name] = self.prox

    def CleanUp(self, result):
        self.log.debug('Opensips: Cleaning up %s' % (self.name))
        self.prox.cleanMysqlDb()
        tmpstr1 = commands.getoutput('pgrep -x opensips')
        if tmpstr1:
            # Kill the opensips process if it is already running
            os.system('sudo pkill openser')
            self.log.debug('Openser.CleanUp: Killed the openser process')

        # Delete the ip address
        ipDelCmd = 'sudo ip addr delete %s/24 dev eth%s > /dev/null 2>&1' %(self.ipAddr,self.interface)
        self.shell.assertCommand(ipDelCmd)

        # Delete the config file
        #self.shell.assertCommand('sudo rm -rf %s' %self.prox.confFile)
	
########################################################################
# Local Variables:
# mode: python
# indent-tabs-mode: nil
# fill-column: 78
# auto-fill-function: do-auto-fill
# End:
