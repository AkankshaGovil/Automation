"""
Endpoint resource for call testing.
"""
import cdr
import data
import gen
import msw
import logging
import pexpect
import qm.common
import qm.test.base
import qm.fields
from   qm.test.result import *
from   qm.test.resource import *
from   qm.test.test import *
from   session import *
import socket
import posix
import ipsetup
import session
#import pktInspect
#import pktInspect_s9
from   msw_errorwatch import MswErrorWatch
from   nxConfigInterface import *
import iServerUpTime
import gatekeeper
from time import *
import cdrMapping
import mswSCMConfigInterface
import globalVar

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
            title="Name/IP Address",
            not_empty_text=True,
            description="""
            The name or IP address of the endpoint.

            This is the source address of outgoing packets from this
            endpoint.  If an Ethernet interface does not exist for
            this address, it will be created.  Names will be resolved to
            and IP address.
            """
            ),
        qm.fields.TextField(
            name="mask",
            title="Net Mask (Number of Bits)",
            default_value = None,
            description="""
            The network mask in CIDR format - number of bits only.  
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
        self.log = logging.getLogger('nextestlog')
        self.shell = session.LocalShell()

        self.ostype = posix.uname()[0]
        ipm = ipsetup.IPManager()
        self.network = ipm.getNetwork(self.ipaddr)
        self.log.debug('NETWORK ---------%s' %self.network)
        self.ipAddr = socket.gethostbyname(self.ipaddr)


        if self.network == '-1':
            raise OSError, 'IP address %s not found in ipsetup.py' % self.ipaddr

        # Ticket 14783 changes
        if self.network == '0':
            port = context['userConfig.eth_pub_iface']
        else:
            port = context['userConfig.eth_pri_iface']

        if self.ostype == 'Linux':
                self.ethport = "eth%s" % port
                self.shell.assertCommand('sudo ip addr add %s/16 dev %s > /dev/null 2>&1' \
                                % (self.ipAddr, self.ethport))
        elif self.ostype == 'SunOS':
                self.ethport = "e1000g%s" % port
                self.shell.assertCommand( 'sudo ifconfig %s addif %s/16 up > /dev/null 2>&1' \
                                % (self.ethport,self.ipAddr))
        else:
            raise NotImplementedError,'Endpoint resource not implemented for %s' %self.ostype

        self.log.debug('Creating endpoint resource: property: %s, ip: %s, mask: %s eth: %s' % \
                                  (self.property, self.ipaddr, self.mask, self.ethport))

        #Create a generic endpoint object.
        if len(self.mask) == 0: self.mask = None
        if len(self.ethport) == 0: self.ethport = None
        try:
            ip = socket.gethostbyname(self.ipaddr)
        except socket.gaierror, exc:
            errno, errstr = exc
            result.Annotate({'Endpoint name' : self.property,
                             'Endpoint address' : self.ipaddr,
                             'Exception' : str(exc)})
            result.Fail('Could not resolve endpoint IP address')
        else:
            ep = gen.Endpoint(ip, self.mask, self.ethport,
                              self.property)
            context[self.property] = ep
            # the following is DEPRECATED.  to get the ip address within
            # your tests, use: ip = context['my_endpoint'].addr
            addr=self.property + '_addr'
            context[addr]=self.ipaddr

    def CleanUp(self, result):

        #PR 184675 - Added the if condition of eth2 and eth3
        #Incase we are using the physucal ip of eth2 and eth3 as resouce, these shouldnot
        #be deleted.
        if not ((self.ipaddr == 'eth2') or (self.ipaddr=='eth3')):
            self.log.debug('Cleaning endpoint resource: property: %s, ip: %s, mask: %s, eth: %s' % \
                                  (self.property, self.ipaddr, self.mask, self.ethport))
            if self.ostype == 'Linux':
                self.shell.assertCommand('sudo ip addr del %s/16 dev %s > /dev/null 2>&1' \
                                % (self.ipAddr,self.ethport))
            elif self.ostype == 'SunOS':
                self.shell.assertCommand( 'sudo ifconfig %s removeif %s > /dev/null 2>&1' \
                                % (self.ethport,self.ipAddr))
            else:
                raise NotImplementedError,'Endpoint resource not implemented for %s' %self.ostype

        self.shell.disconnect()

class H323Transmitter(Endpoint):
    """
    Manage the H323 generator in transmit mode.
    """
    arguments = [
        qm.fields.TextField(
            name='gateway',
            title='Gateway Address',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='realm',
            title='Associated realm',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='destnum',
            title='Phone Number of Callee',
            default_value = '666',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='numcalls',
            title='Number of calls to run',
            not_empty_text = True,
            default_value = '1',
            description="""
            """
            )
        ]

    def SetUp(self, context, result):
        """
        Set up the resource.

        Create a generic endpoint object.
        """
        if len(self.mask) == 0: self.mask = None
        if len(self.ethport) == 0: self.ethport = None
        ep = gen.Endpoint(self.ipaddr, self.mask, self.ethport)
        ep.setValue('realm', self.realm)
        ep.configure('h323',
              mode='send',
              h323id=self.property,
              gateway=self.gateway,
              numcalls=self.numcalls,
              destnum=self.destnum
              )
        context[self.property] = ep


class H323Receiver(Endpoint):
    """
    Manage the H323 generator.

    TODO  when auto register on, verify registration confirm
    """
    arguments = [
        qm.fields.TextField(
            name='gateway',
            title='Gateway Address',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='realm',
            title='Associated realm',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='callingplan',
            title='Calling Plan',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='extn',
            title='Extension',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='priority',
            title='Priority',
            not_empty_text = True,
            description="""
            """
            )
        ]

    def SetUp(self, context, result):
        """
        Set up the resource.

        Create a generic endpoint object.
        """
        if len(self.mask) == 0: self.mask = None
        if len(self.ethport) == 0: self.ethport = None
        ep = gen.Endpoint(self.ipaddr, self.mask, self.ethport)
        ep.setValue('callingplan', self.callingplan)
        ep.setValue('extn', self.extn)
        ep.setValue('priority', self.priority)
        ep.setValue('realm', self.realm)
        ep.configure('h323',
              mode='receive',
              h323id=self.property,
              gateway=self.gateway
              )
        context[self.property] = ep

class SIPEndpoint(Endpoint):
    """ SIP endpoint"""
    arguments = [
        
        qm.fields.TextField(
            name='extn',
            title='Extension',
            not_empty_text = True,
            description="""
            """
            ),
        qm.fields.TextField(
            name='realm',
            title='Realm',
            not_empty_text = True,
            description="""
            """
            ),
        ]

    def SetUp(self, context, result):
        """
        Set up the resource.

        Create a generic endpoint object.
        """
        ep = gen.Endpoint(self.ipaddr, self.mask, self.ethport)
        #ep.setValue('callingplan', self.callingplan)
        ep.setValue('extn', self.extn)
        ep.setValue('realm', self.realm)
        context[self.property] = ep

import nexhelp

class CDRTest(Test):
    """
    Verify CDR content after making a call.

    This test connects to the MSW and prepares to capture CDR output
    before releasing control to the test code.  Once the test code has
    run, any captured CDRs are parsed and returned to the assertion
    code as a list.  The list is referenced through the context entry
    labeled 'cdr'.  The assertion code is then responsible for
    verifying the CDR information.

    A control resource that defines a connection to the MSW must be
    associated with tests of this class.

    TODO  is this the best way to do this?
    """
    arguments = [
        qm.fields.TextField(
            name="testDocumentation",
            title="Test documentation",
            description=nexhelp.CDRTestHelp.testDocumentation,
            verbatim="true",
            multiline="true",
            default_value="""Write test description here."""
            ),
        qm.fields.TextField(
            name="testsource",
            title="Execution Code",
            description=nexhelp.CDRTestHelp.testsource,
            verbatim="true",
            multiline="true",
            ),

        qm.fields.TextField(
            name="assertsource",
            title="Assertions",
            description=nexhelp.CDRTestHelp.assertsource,
            verbatim="true",
            multiline="true",
            default_value="""clist = context['cdrlist']
test.assertEquals(len(clist), 1, 'One CDR expected, different number or zero captured')
test.assertEquals(clist[0]['15-call-error'],'', 'No call error')"""
            ),
        ]

    def Run(self, context, result):
        """
        Run a call test and capture CDRs.

        After verifying that a control channel to the MSW is open,
        this test monitors the CDR file while running the test code.
        Any exceptions in the test cause the CDR reader to abort.

        If the test completes without exceptions, the CDR count is
        checked.  If one of more CDRs were captured, the assertion
        code is executed. If no CDRs were captured, the test fails.

        TODO  inefficient to instantiate the reader and get the
        TODO  dictionary every time.  could the dict be a resource?
        """
        # Attach to the log file
        self.log = logging.getLogger('nextestlog')
        # Output a banner to mark the start of this call
        self.log.debug('CDRTest: Run: ####')
        self.log.debug('CDRTest: Run: ####')
        self.log.debug('CDRTest: Run: #### Running CDRTest  %s'
                        % context['qmtest.id'])
        self.log.debug('CDRTest: Run: ####')
        self.log.debug('CDRTest: Run: ####')
        if not context.has_key('msw'):
            raise KeyError(
                'CDRTest requires control resource that sets "msw" property.')
        reader = cdr.CDRReader(context['msw'])
        reader.startCapture()
        
        globalSpace = { 'context': context,
                        'result': result,
                        'test': self,
                        'myglobals':globals()}
        localSpace = {}
        try:
            exec self.testsource in globalSpace, localSpace
        except Exception, exc:
            reader.abortCapture()
            if sys.exc_type is AssertionError:
                result.Annotate({"Assertion error" : str(exc)})
                result.Fail("Test assertion is false")
            elif sys.exc_type is gen.EndpointError:
                result.Annotate({'Endpoint error' : str(exc)})
                result.Fail('Call failed; an endpoint exception occurred')
            else:
                result.NoteException(cause="Exception executing source.")
                result.NoteException(cause=str(exc))
            self._stopGens(context)
        else:
            reader.stopCapture()
            context['cdrlist'] = reader.decode()
            if len(context['cdrlist']) < 1:
                result.Fail('No CDRs were captured.')
                # TODO show gen state and command to reproduce gen
                return

            if self.assertsource is not None:
                try:
                    exec self.assertsource in globalSpace, localSpace
                    annotations = { '00 Assertion' : 'Test Passed' }
                    annotations.update(context['cdrlist'][0])
                    result.Annotate(annotations)
                except AssertionError, theException:
                    # put the '00' in so assertion appears first in list
                    annotations = { '00 Assertion' : str(theException) }
                    annotations.update(context['cdrlist'][0])
                    result.Fail('Test assertion is false.', annotations)

        
    ######################################################################
    ## Methods called from test code.
    ######################################################################
    def assert_(self, expr, message=None):
        if not expr:
            raise AssertionError(message)

    def assertEquals(self, first, second, message=None, third=0):
        if third == 0:
            if first != second:
                if not message:
                    message = '"%s" is equal to "%s"' % (first, second)
                raise AssertionError(message)
        else:
            if first == second:
                if not message:
                    message = '"%s" is equal to "%s"' % (first, second)
                raise AssertionError(message)
        
    #######################################################################
    ## private method 
    #######################################################################
    def _stopGens(self,context):
        for key in context.keys():
            try:
                if type(context[key])==gen.Endpoint:
                    context[key].stop()
                    self.log.debug('CDRTest: stopGens: call gen on %s is stopped' % key)
            except Exception, exc:
                self.log.debug('CDRTest: _stopGens: %s' % str(exc))

class CallTestError(Exception):
    "General exception for CallTest"
  
class CallTest(Test):
    """
    Run one or more calls and gather related data.
    """

    arguments = [
        qm.fields.TextField(
            name="testDocumentation",
            title="Test documentation",
            description=nexhelp.CallTestHelp.testDocumentation,
            verbatim="true",
            multiline="true",
            default_value="""Write test description here."""
            ),

        qm.fields.TextField(
            name="testsource",
            title="Execution Code",
            description=nexhelp.CallTestHelp.testsource,
            verbatim="true",
            multiline="true",
            ),

        qm.fields.TextField(
            name="assertsource",
            title="Assertions",
            description=nexhelp.CallTestHelp.assertsource,
            verbatim="true",
            multiline="true",
            default_value="""clist = context['cdrlist']
test.assertEquals(len(clist), 1, 'One CDR expected, unexpected number or zero captured')
test.assertEquals(clist[0]['15-call-error'],'', 'No call error')"""
            ),

        qm.fields.EnumerationField(
            name="pkttrace",
            title="Packet Trace Enabled",
            description=nexhelp.CallTestHelp.pkttrace,
            enumerals=["ON", "OFF"],
            default_value="OFF"
            ),

        qm.fields.EnumerationField(
            name="dbglog",
            title="Debug Log saving Enabled",
            description=nexhelp.CallTestHelp.dbglog,
            enumerals=["ON", "OFF"],
            default_value="ON"
            ),

        # For SCM and redundancy testing
        qm.fields.EnumerationField(
            name="redundancy",
            title="Redundancy Configuration Enabled",
            description=nexhelp.CallTestHelp.redundant,
            enumerals=["ON", "OFF"],
            default_value="OFF"
            ),

        qm.fields.EnumerationField(
            name="cdrcollect",
            title="CDR Collection Enabled",
            description=nexhelp.CallTestHelp.cdrcollect,
            enumerals=["ON", "OFF"],
            default_value="ON"
            ),

       ]

    def Run(self, context, result):
        # Attach to the log file
        self.log = logging.getLogger('nextestlog')
        # Output a banner to mark the start of this call
        self.log.debug('CallTest: Run: ####')
        self.log.debug('CallTest: Run: ####')
        self.log.debug('CallTest: Run: #### Running CallTest  %s'
                        % context['qmtest.id'])
        self.log.debug('CallTest: Run: ####')
        self.log.debug('CallTest: Run: ####')

        #-----------------------------------------------------------------------
        # Kill any errant gen processes that may be running. Ideally, 
        # we need to earn why gen processes errantly persist, but for now
        # we will just kill them.
        #-----------------------------------------------------------------------
        self.killErrantGenProcs()

        if not context.has_key('mswinfo'):
            raise CallTestError("CallTest requires a MSW resource")

        # 18524 - Create a MSWConfig object which is used for configuring certain 
        # parameters of the MSW
        # 26333 - Pass on the SCM configuration information 
        scmConf = context['nextest.scm_configuration'] 
        context['nextest.mswconfig'] = MswConfig('mymsw',scmConf)

        # 22268 - Update the iServer version information
        context['nextest.mswconfig'].mswVersion = context['mswinfo'].iVersion

        
        # For SCM testing
        if (self.redundancy=="ON"):
            if not context.has_key('bkupinfo'):
                raise CallTestError("CallTest requires backup MSW resource")
        
        # If configured, start CDR collection:
        if (self.cdrcollect=="ON"):
            self.ccap = c = data.CDRCollector(context, 'CallTestCDR')
            c.start(context)
        # If configured, start producing a PCAP packet trace file:
        # Ticket-36712: Packet tracing is started if it is enabled either from
        # the testcase or the command line
        if (self.pkttrace=="ON") or (context['nextest.pkttrace']!="OFF"):
            self.pcap = p = data.PktTraceCollector(context,
                                       'pkttracecollector',
                                       '/tmp')
            p.start(context)
        # If configured, collect a debug log file:
        if (self.dbglog=="ON") and (context['nextest.dbglog']!="OFF"):
            self.dbg = d = data.DbgLogCollector(context,
                                       'dbglogcollector',
                                       '/var/log')
            d.start(context)

        # Ticket 13450 fix
        # Instantiate the errorlog watcher
        mswErrwatch = MswErrorWatch(context['mswinfo'])
        
        globalSpace = { 'context': context,
                        'result': result,
                        'test': self,
                        'myglobals':globals()}
        localSpace = {}
        self.runList = []
        
        # Need a flag so that assertions are not run if test has problems
        testException = False

        # 36319 - Compare the pid of the gis process before and after the test
        # to find out if the script has restarted the iserver. 
        # Check the Script Contents to see if it restarts iserver
        mswObj = session.SSH(['root@mymsw'])

        # 38957 - Added function names that restart iserver to the gis pid check
        src = self.testsource.lower()
        chk1 = src.__contains__('iserver all stop') or src.__contains__('ftest') or src.__contains__('checkiserver')
        chk2 = src.__contains__('checkscmstatus') or src.__contains__('restartscm') or src.__contains__('.initialize')
        if chk1 or chk2:
            gispid1 = ''
        else:
            gispid1 = mswObj.filter('pgrep -x gis')

        # 58122 - Added hung call check before the test case run
        if (context['nextest.hung_calls']!='OFF'):
            hungcalls1 = mswObj.getVPorts('Used VPORTS')
    
        # 36319 - Find the HVal value before running the test
        hVal2 = ''  
        hVal1 = mswObj.filter('cli test passive | grep -i hval')
        self.log.debug('CallTest: hVal Value before running test is %s' %hVal1)

        try:
            exec self.testsource in globalSpace, localSpace
            self.log.info("CallTest: run: call test source done")
        except Exception, exc:
            testException = True
            if sys.exc_type is AssertionError:
                result.Annotate({"Assertion error" : str(exc)})
                result.Fail("Failed test assertion")
            elif sys.exc_type is gen.EndpointError:
                result.Annotate({'Endpoint error' : str(exc)})
                result.Fail('Call failed; an endpoint exception occurred')
            else:
                result.NoteException(cause="Exception executing source.")
                result.NoteException(cause=str(exc))
            # At this point, we have handled the exception from the
            # test source.  Before continuing, must tell any outstanding
            # generators to stop or the next test will hang.
            self._stopGens(context)
            # Stop commands created via runCommand
            for cmd in self.runList:
                print "stopping", cmd
                self.stopCommand(cmd)

        # Save all test data and annotate the result with the location
        if (self.cdrcollect=="ON"):
            c.saveResult(context, 'cdr.dat')
            result[Result.NEXTEST_CDR] = c.getRelResultFile()
        # Ticket-36712: Packet tracing is started if it is enabled either from
        # the testcase or the command line
        if (self.pkttrace=="ON") or (context['nextest.pkttrace']!="OFF"):
            p.saveResult(context, 'pkttrace_g0_g1.pcap')
            result[Result.NEXTEST_NET] = p.getRelResultFile()
           
            ostype = posix.uname()[0]
            if ostype == 'Linux':
              # Change for S9 duplicate packet to maintain different behavior than s3.
              if (context['userConfig.automation'] == 's9'):
                 context['pdmlPkts'] = pktInspect_s9.readCapture( p.getPdmlFile() )
              else:
                 context['pdmlPkts'] = pktInspect.readCapture( p.getPdmlFile() )
            elif ostype == 'SunOS':
              self.log.info('CallTest.Run(): pdml files not supported on SunOS, don\'t run any tests using Packet Inspection Feature.')

        #print "pkttrace file saved"   
	#sleep(20)
        if (self.dbglog=="ON") and (context['nextest.dbglog']!="OFF"):
            d.saveResult(context, 'gisdbg.log')
            result[Result.NEXTEST_GIS] = d.getRelResultFile()

        mswObj = session.SSH(['root@mymsw'])
        # 36319 - Verify whether gis has restarted during the test
        if (gispid1 != ''):
            gispid2 = mswObj.filter('pgrep -x gis') 
            if (gispid1 != gispid2):

                annotations = { 'Assertion' : 'GIS has restarted during the test, pid changed %s:%s!!!!' %(gispid1,gispid2) }
                result.Fail('GIS has restarted', annotations)
            #self.assert_((gispid1==gispid2),'GIS has restarted!!!!')

        # 58122 - Verify there are no hung calls after the test case is run
        if (context['nextest.hung_calls']!='OFF'):
            hungcalls2 = mswObj.getVPorts('Used VPORTS')
            self.assert_((hungcalls1==hungcalls2),'VPORTS are not released and calls are hung!!!!')

        # Ticket 13450 fix
        # Collect the errorlog from MSW and verify
        mswErrwatch.watchMswErrors(result,(context['nextest.fail_on_msw_error'] == "ON"))

        # Execute the assertion code only if we didn't get a test exception
        if (not testException) and (self.assertsource is not None):
            try:
                exec self.assertsource in globalSpace, localSpace
            except AssertionError, theException:
                # put the '00' in so assertion appears first in list
                annotations = { 'Assertion' : str(theException) }
                result.Fail('Test assertion(s) false.', annotations)

        # PR 133009 - Cdr and  Radius match based on the global flag
	if (context['nextest.radius_verify']=='ON') and (self.cdrcollect=="ON"):
	    if not ((context['qmtest.id'].__contains__('provision')) or (context['qmtest.id'].__contains__('xit'))):
	        if context.has_key('interimCdr'):
		    interim = context['interimCdr']
                else:
		    interim = '0'

                cdrMapping.CdrMapping(context,result).match(interim)

        
        radpath=context['nextest.result_path']
        radiusfile=radpath+"/rad-pkt.log"
        if os.path.exists(radiusfile):
             #print "rad-pkt.log exists"
             os.system("rm %s" %radiusfile)


        # 10409 - Verify whether Media check was successful for all the endpoints
        self._verifyMedia(context,result)

        # 58451 - Check for Ongoing Calls on Endpoint level
        list1=[]
        for key in context.keys():
	    #print type(context[key])
            try:
                if (type(context[key])== gen.Endpoint or 
				type(context[key])== gatekeeper.gnuGk ): 
		    #type(context[key])== sip.Endpoint or 
		    #type(context[key])== ohphone.Endpoint):
                    if (context[key].checkHung):
                        list1.append(context[key].name)
                        # Reset the value
                        context[key].checkHung = False
            except Exception, exc:
                self.log.debug('CallTest: Ongoing call verification: %s' % str(exc))
        if (len(list1)>1):
            for i in list1:
                val = '0'
                val = mswObj.filter("cli iedge lkup %s 0 |grep 'Ongoing Calls' |awk '{print$3}'"%i)
                tmp = val.split('\r\n')
                if (len(tmp) > 1):
                    val_1 = tmp[1]
                else:
                    val_1 =  '0'
                ##PR 136938 Change the assert statement as this causes other testcases to fail as well
                #self.assert_(((val_1 == '0') or (val_1 == '')),'Endpoint %s contains hung calls!!!!'%i)
                if not ((val_1 == '0') or (val_1 == '')):
                    annotations = { 'Assertion' : 'Endpoint %s contains hung calls!!!!' %i }
                    result.Fail('Hung calls Found.', annotations)

                    if (context['nextest.scm_configuration'] == 'ON'):
                        mswSCMConfigInterface.restartSCM(mswObj,cleanRadius='ON')
                    else:                                        
                        mswObj.assertCommand('iserver all stop',timeout= int(globalVar.iserverStopTimeout))
                        mswObj.assertCommand('iserver all start',timeout=int(globalVar.iserverStartTimeout))


        # 36319 - Compare the HVal value before and after the test,
        # if the script uses a H323 endpoint
        for key in context.keys():
            try:
                if (type(context[key])==gen.Endpoint or type(context[key])==gatekeeper.gnuGk):
                    if (context[key].checkHVal):
                        hVal2 = 'True' 
                        # Reset the value so that it does not affect some other test while run as a suite
                        context[key].checkHVal = False
            except Exception, exc:
                self.log.debug('CallTest: hVal verification: %s' % str(exc))

        # 39583 -  Do not perform the hVal check if the scripts restarts or if the previous
        # script restarts MSW
        if hVal2:
            context['hValNotChecked'] = False
            # Do not perform hVal check if the current or the previous test script restarts iserver
            if chk1 or chk2:
                hVal2 = '' 
            if context['msw_restart']:
                hVal2 = ''
                context['hValNotChecked'] = True  
        else:
            context['hValNotChecked'] = False
   
        if hVal2:
            # 39583 - Wait for 6 seconds before checking the hVal
            sleep(6) 
            hVal2 = mswObj.filter('cli test passive | grep -i hval')
            self.log.debug('CallTest: hVal Value after running test is %s' %hVal2)
            if (hVal1 != hVal2):
                hVal1 = hVal1.strip('|hVal size =').strip('\r\n')
                hVal2 = hVal2.strip('|hVal size =').strip('\r\n')
                msg = '\n  HVAL CHECK WARNING       : HVal before test %s not equal to HVal after test %s!' %(hVal1,hVal2)
                result[Result.CAUSE] = msg

        mswObj.disconnect()

        
    #--------------------------------------------------------------------------
    # This method kills any errant gen processes that may be running.
    #--------------------------------------------------------------------------
    def killErrantGenProcs(self):
        self.log.debug('killErrantGenProcs: Checking for sgen PIDs...')
        ownpid=os.getpid()
        #Ticket 36101 - grep only our child process or whose ppid is 1
        cmd='pgrep -x sgen -P 1,%s >/tmp/nextest_pgrepsgen' % ownpid
        os.system(cmd)
        pgrepgen_fd=open('/tmp/nextest_pgrepsgen','r')
        genpidlist = pgrepgen_fd.readlines()
        pgrepgen_fd.close()
        for genpid in genpidlist:
            cmd='sudo kill -9 %s' % genpid
            os.system(cmd)
            self.log.error('killErrantGenProcs: ****** ERRANT GEN PID: %s DETECTED, KILLED  ******' % genpid)
            print "****** ERRANT SGEN PID DETECTED, KILLED  ******\n"
        self.log.debug('killErrantGenProcs: Checking for gen PIDs...')
        #Ticket 36101 - grep only our child process or whose ppid is 1
        cmd='pgrep -x gen -P 1,%s >/tmp/nextest_pgrepgen' % ownpid
        os.system(cmd)
        pgrepgen_fd=open('/tmp/nextest_pgrepgen','r')
        genpidlist = pgrepgen_fd.readlines()
        pgrepgen_fd.close()
        for genpid in genpidlist:
            cmd='sudo kill -9 %s' % genpid
            os.system(cmd)
            self.log.error('killErrantGenProcs: ****** ERRANT GEN PID: %s DETECTED, KILLED  ******' % genpid)
            print "****** ERRANT GEN PID DETECTED, KILLED  ******\n"
        self.log.debug('killErrantGenProcs: Checking for mgen PIDs...')
        #Ticket 36101 - grep only our child process or whose ppid is 1
        cmd='pgrep -x mgen -P 1,%s >/tmp/nextest_pgrepmgen' % ownpid
        os.system(cmd)
        pgrepgen_fd=open('/tmp/nextest_pgrepgen','r')
        genpidlist = pgrepgen_fd.readlines()
        pgrepgen_fd.close()
        for genpid in genpidlist:
            cmd='sudo kill -9 %s' % genpid
            os.system(cmd)
            self.log.error('killErrantGenProcs: ****** ERRANT GEN PID: %s DETECTED, KILLED  ******' % genpid)
            print "****** ERRANT MGEN PID DETECTED, KILLED  ******\n"
            
    ######################################################################
    ## Methods called from test code.
    ######################################################################
    def assert_(self, expr, message=None):
        if not expr:
            raise AssertionError(message)

    def assertEquals(self, first, second, message=None, third=0):
        if third == 0:
            if first != second:
                if not message:
                    message = '"%s" is equal to "%s"' % (first, second)
                raise AssertionError(message)
        else:
            if first == second:
                if not message:
                    message = '"%s" is equal to "%s"' % (first, second)
                raise AssertionError(message)

    def getCDRCount(self):
        if self.cdrcollect == "ON":
            filename = self.ccap.resultpath + '/cdr.dat'
            f = open(filename, 'r')
            list = f.readlines()
            f.close()
            return len(list)
        else:
            return 0
        
    def runCommand(self, cmd, args):
        """
        Run a command with pexpect.spawn().

        The command will be "registered" so that it can be killed if
        a test exception occurs.
        """
        proc = pexpect.spawn(cmd, args)
        self.runList.append(proc)
        print "registered", proc
        return proc

    def stopCommand(self, proc):
        "Stop (or kill) a command started by runCommand."
        if proc not in self.runList:
            raise CallTestError("command not registered")
        if proc.isalive():
            proc.kill(9)
        self.runList.remove(proc)

    # 36319 - Added packet trace functions to be used in scripts
    def checkFastSlowStart(self,pdmlPkts,txIp,rxIp,fast):
        """
        This function verifies whether the call has been established as a fast start call or as a
        slow start call. It checks whether the Setup sent to the receiver and the Connect message 
        sent to the transmitter are sent with the Open Logical Channel message or not. 

        pdmlPkts - PDML packets captured during the test case run
        txIp     - IP address of the transmitter
        rxIp     - IP address of the receiver
        fast     - True if the call needs to be verified for fast start and False if it needs to
                   be verified for slow start
        """
        epList = [txIp,rxIp]
        msg = {}
        if rxIp != '':
            msg[rxIp] = 'h225.setup'
        if txIp != '':
            msg[txIp] = 'h225.connect'

        for epIp in epList:
            if (epIp != ''): 
                hostname, hostnames, hostaddrs = socket.gethostbyaddr(epIp)
                pkt = pdmlPkts.endpoints[hostname].h323.getPacket(msg[epIp])
                olc = pkt.getFieldFirstValue('h245.OpenLogicalChannel')
                if fast: 
                    self.assert_(olc!=None,'%s from MSW does not contain Fast Start information' %msg[epIp])
                else:
                    self.assert_(olc==None,'%s from MSW contains Fast Start information' %msg[epIp])

    def checkTunneling(self,pdmlPkts,txIp,rxIp,check):
        """
        This function checks whether the tunneling happens or not. The function verifies
        whether the TCS packets sent to the endpoint by MSW contain/do not contain Q931 information.

        pdmlPkts - PDML packets captured during the test case run
        txIp     - IP address of the transmitter
        rxIp     - IP address of the receiver
        check    - True for cases where tunneling needs to be verified. If its False, it verifies whether
                   tunneling is not happening. 
        """
 
        epList = [txIp,rxIp]
        for epIP in epList:
            if epIP=='':
                continue
            hostname, hostnames, hostaddrs = socket.gethostbyaddr(epIP)
            pktFound = False
            i = 0
            while(True):
                i += 1 
                tcsPkt = pdmlPkts.endpoints[hostname].h245.getPacket('h245.terminalCapabilitySet',i)
                if (tcsPkt == None):
                    break
                elif (tcsPkt.getFieldFirstValue('ip.dst').__contains__(epIP)):
                    pktFound = True
                    q931 = tcsPkt.getFieldFirstValue('q931.message_type')
                    if check:
                        self.assert_(q931,'Tunneling - Terminal Capability Set was not sent as part of Q931 message')
                    else:
                        self.assert_((q931==None),'Tunneling - Terminal Capability Set was sent as part of Q931 message')

            self.assert_(pktFound,'Terminal Capability Set packet was not received by %s' %epIP)

    def checkSIPFaxNeg(self,pdmlPkts,txIP,rxIP):
        """ 
        Verifies whether the transmitter receives an INVITE packet with T38 Fax
        information and whether the receiver receives 200 OK with T38 information.

        pdmlPkts - PDML packets captured during the test case run
        txIP     - IP address of the transmitter
        rxIP     - IP address of the receiver
        """
             
        if txIP:
            # Check whether a second INVITE with t38 Fax information is received
            hostname, hostnames, hostaddrs = socket.gethostbyaddr(txIP) 
            invFaxPkt = pdmlPkts.endpoints[hostname].sip.getInvitePacket(2)

            self.assert_(invFaxPkt,'Second INVITE was not received by transmitter')
            fax = invFaxPkt.getFieldFirstShowName('sdp.media.format')
            self.assert_(fax.__contains__('t38'),'INVITE with Fax T38 information was not sent by MSW')
            cseq = invFaxPkt.getFieldFirstShowName('sip.CSeq').strip('INVITE').strip('CSeq:')

            ackPkt = pdmlPkts.endpoints[hostname].sip.getPacket('ACK',2)
            self.assert_(ackPkt,'Second ACK packet was not received')
            cseqres = ackPkt.getFieldFirstShowName('sip.CSeq').__contains__(cseq)
            self.assert_(cseqres,'ACK was not received for INVITE with Fax T38 information')

        if rxIP: 
            hostname, hostnames, hostaddrs = socket.gethostbyaddr(rxIP) 

            # Verify whether 200OK with T38 Fax information is received by the receiver
            okPkt = pdmlPkts.endpoints[hostname].sip.getPacket('200',2)
            fax = okPkt.getFieldFirstShowName('sdp.media.format')
            self.assert_(fax.__contains__('t38'),'200 OK with Fax T38 information was not sent by MSW')
 
    def checkH323FaxNeg(self,pdmlPkts,txIP,rxIP):
        """
        This function verifies whether the first media channel is closed before
        opening a new channel for fax. It also verifies whether the Request Mode
        packet is received by the transmitter from the MSW.

        pdmlPkts - PDML packets captured during the test case run
        txIP     - IP address of the transmitter
        rxIP     - IP address of the receiver
        """  
        if txIP: 
            # Check whether T38 Request Mode message is sent by MSW to the transmitter
            hostname, hostnames, hostaddrs = socket.gethostbyaddr(txIP)
            reqModPkt = pdmlPkts.endpoints[hostname].h245.getPacket('h245.requestMode')
            self.assert_(reqModPkt,'Request Mode packet not received')

            mode = reqModPkt.getFieldFirstShowName('h245.t38fax')
            self.assert_(mode, 'Request Mode packet does not contain T38 fax information')

        ind = 0 
        # Verify whether CLC and OLC were received by both Tx and Rx
        for epip in [rxIP,txIP]:
            if epip == '': 
                continue

            hostname, hostnames, hostaddrs = socket.gethostbyaddr(epip)

             
            # Verify whether Close Logical Channel was received from MSW
            # One is sent by the endpoint and one is received by the endpoint. Hence check the
            # first two packets.
            clcPkt = pdmlPkts.endpoints[hostname].h245.getPacket('h245.closeLogicalChannel',1)
            self.assert_(clcPkt,'Close Logical Channel packet not received')
            if (clcPkt.getFieldFirstValue('ip.dst')!=epip):
                clcPkt = pdmlPkts.endpoints[hostname].h245.getPacket('h245.closeLogicalChannel',2)
                self.assert_(clcPkt.getFieldFirstValue('ip.dst')==epip,'Close Logical Channel was not sent to the Endpoint')

            # Verify whether a OLC packet was sent to the ep with T38 Fax information
            h245Pkts = pdmlPkts.endpoints[hostname].h245.packets
            olcFound = False
            for pkt in h245Pkts:
                if pkt.getFieldFirstShowName('h245.request'):
                    olcinfo = pkt.getFieldFirstShowName('h245.request').__contains__('openLogicalChannel') 
                    if olcinfo and pkt.getFieldFirstValue('ip.dst') == epip:
                        if pkt.getFieldFirstShowName('h245.t38fax'):
                            if pkt.getFieldFirstShowName('h245.t38fax').__contains__('t38'):
                                olcFound = True
                                break 

            self.assert_(olcFound,'Open Logical Channel with T38 Fax information was not sent by MSW to the receiver')

            # Verify whether Close Logical Channel was received before T38 channel was opened
            bres = (pkt.getFieldFirstValue('frame.number') > clcPkt.getFieldFirstValue('frame.number'))
            self.assert_(bres,'Audio Channel was not closed before opening T38 Fax channel')

    #######################################################################
    ## private method 
    #######################################################################
    def _stopGens(self,context):
        """Walk thru the context, find gen.Endpoints and stop them.

        This method is only called when a test exception occurs.
        """
        for key in context.keys():
            try:
                if type(context[key])==gen.Endpoint:
                    context[key].stop()
                    self.log.debug('CallTest: stopGens: call gen on %s is stopped' % key)
            except Exception, exc:
                self.log.debug('CallTest: _stopGens: %s' % str(exc))

    def _verifyMedia(self,context,result):
        """Walk thru the context, find gen.Endpoints and verify for media results

        """
        for key in context.keys():
            if type(context[key])==gen.Endpoint:
                ep = context[key]
                if (ep.verifyMedia == True):
                    for i in range(len(ep.mediaResult)):
                        msg = "Media Check %s for endpoint %s " %(i+1,ep.name)
                        if (ep.mediaResult[i] == False):
                            result.Annotate({'Endpoint Media error' : '%s FAILED' %msg})
                            # Do not overwrite the FAIL verdict 
                            if (result.GetOutcome()=="PASS"):
                                result.Fail('Call failed; Endpoint Media Verification Failed')
                        else:
                            self.log.debug("%s PASSED" %(msg))

                    # Clean up after verification
                    ep.verifyMedia = False
                    ep.mediaResult=[] 

########################################################################
# Local Variables:
# mode: python
# indent-tabs-mode: nil
# fill-column: 78
# auto-fill-function: do-auto-fill
# End:
