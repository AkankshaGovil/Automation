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
import pktInspect
import nexhelp
import types
import os
from   nxConfigInterface import *
from time import *
import gatekeeper

class MetaCallTestError(Exception):
    "General exception for MetaCallTest"
  
class MetaCallTest(Test):
    """
    Run one or more calls and gather related data.
    """

    arguments = [
        qm.fields.TextField(
            name="datasource",
            title="Test Data source",
            description="No help, work harder",
            verbatim="true",
            multiline="true"
            ),

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
            default_value=""" """
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

    def __printBanner(self):
        """Output a banner to mark the start of this call.
        """
        self.log.debug('CallTest: Run: ####')
        self.log.debug('CallTest: Run: ####')
        self.log.debug('CallTest: Run: #### Running CallTest  %s' % self.context['qmtest.id'])
        self.log.debug('CallTest: Run: ####')
        self.log.debug('CallTest: Run: ####')

    def __preTestChecks(self):
        """Check if the prerequisites are present and correct.
        """

        #Checks if MSW is configured
        if not self.context.has_key('mswinfo'):
            raise CallTestError("CallTest requires a MSW resource")

        # 31356 - Create a MSWConfig object which is used for configuring certain 
        # parameters of the MSW
        # 31356 - Pass on the SCM configuration information 
        scmConf = self.context['nextest.scm_configuration'] 
        self.context['nextest.mswconfig'] = MswConfig('mymsw',scmConf)

        # 31356 - Update the iServer version information
        self.context['nextest.mswconfig'].mswVersion = self.context['mswinfo'].iVersion

        # Checks for SCM testing
        # 31356 - Change context.has_key to self.context.has_key
        if (self.redundancy=="ON"):
            if not self.context.has_key('bkupinfo'):
                raise CallTestError("CallTest requires backup MSW resource")

    def __startDataCollectors(self):
        """Start all data collectors.
        """
        #Start CDR collection
        if (self.cdrcollect=="ON"):
            self.ccap = c = data.CDRCollector(self.context, 'CallTestCDR')
            c.start(self.context)

        #Start producing a PCAP packet trace file:
        # Ticket-36712: Packet tracing is started if it is enabled either from
        # the testcase or the command line
        if (self.pkttrace=="ON") or (self.context['nextest.pkttrace']!="OFF"):
            #18523 Got error while running the scripts which uses metatestcase
            #Hence modified to resolve this error
            self.pcap = self.p = data.PktTraceCollector(self.context,
                                       'pkttracecollector',
                                       '/tmp')
            self.p.start(self.context)

        #Collect a debug log file:
        if (self.dbglog=="ON") and (self.context['nextest.dbglog']!="OFF"):
            self.dbg = d = data.DbgLogCollector(self.context,
                                       'dbglogcollector',
                                       '/var/log')
            d.start(self.context)
   
    #32414 - # 32414 Included testcase paramter to annotate the
    #result with metatestcase name 
    def __stopDataCollectors(self,testCase):
        """Stop all data collectors.
        """
        #Save CDRs
        if (self.cdrcollect=="ON"):
            c = self.ccap
            # 32414  Modified to include metatestcase name
            # Otherwise only the result of the last meta call test will be stored
            tmp = testCase['testName'] + '-'+ 'cdr.dat' 
            c.saveResult(self.context, tmp)
            self.result[Result.NEXTEST_CDR] = c.getRelResultFile()

        #Save packet capture and convert it into PDML
        # Ticket-36712: Packet tracing is started if it is enabled either from
        # the testcase or the command line
        if (self.pkttrace=="ON") or (self.context['nextest.pkttrace']!="OFF"):
            #18523 Got error while running the scripts which uses metatestcase
            #Hence modified to resolve this error
            #32414  Modified to include metatestcase name
            # Otherwise only the result of the last meta call test will be stored
            tmp = testCase['testName'] + '-'+ 'pkttrace_g0_g1.pcap'
            self.p.saveResult(self.context, tmp)
            self.result[Result.NEXTEST_NET] = self.p.getRelResultFile()
           
            ostype = posix.uname()[0]
            if ostype == 'Linux':
              #18523 Got error while running the scripts which uses metatestcase
              #Hence modified to resolve this error
              self.context['pdmlPkts'] = pktInspect.readCapture( self.p.getPdmlFile() )
            elif ostype == 'SunOS':
              self.log.info('CallTest.Run(): pdml files not supported on SunOS, '
                            'don\'t run any tests using Packet Inspection Feature.')

        #Save debug log
        if (self.dbglog=="ON") and (self.context['nextest.dbglog']!="OFF"):
            #32414  Modified to include metatestcase name
            # Otherwise only the result of the last meta call test will be stored
            tmp = testCase['testName'] + '-'+ 'gisdbg.log'
            d.saveResult(context, tmp)
            result[Result.NEXTEST_GIS] = d.getRelResultFile()

    #32414 -  Added input parameter context to get the context in to 
    #data source section 
    def __executeDataSource(self, dataSource, globalSpace,context):
        """executes a given dataSource in the given global and localSpace. Returns
           execution status in True/False form.
        """
        result = globalSpace['result']
        # 32414 - Create a MSWConfig object which is used for configuring certain
        # parameters of the MSW
        # Pass on the SCM configuration information
        scmConf = context['nextest.scm_configuration']
        context['nextest.mswconfig'] = MswConfig('mymsw',scmConf)
        context['nextest.mswconfig'].mswVersion = context['mswinfo'].iVersion
        try:
            localSpace = {}
            exec dataSource in globalSpace, localSpace
            # 32414 - localSpace will contain all the variables that
            # are present in the datasource section. And only the first list will
            # act as the input for Meta Call Test
            for value in localSpace.values():
                if type(value)==types.ListType:
                    self.context['nextest.metaTestData'] = value
                    break
        except Exception, exc:
            result.SetOutcome(Result.ERROR, "datasource section error: %s" % str(exc))
            return False

        #verify that testName is present in all test data
        try:
            for tests in self.context['nextest.metaTestData']:
                if tests['testName'] != None:
                    pass
        except Exception:
            result.SetOutcome(Result.ERROR, "datasource section error: 'testName' is not defined'")
            return False

        self.log.info("CallTest: run: call data source done")
        return True

    def __executeTestSource(self, testSource, globalSpace, localSpace):
        """executes a given testSource in the given global and localSpace. Returns
           execution status in True/False form.
        """
        try:
            exec testSource in globalSpace, localSpace
            self.log.info("CallTest: run: call test source done")
        except Exception, exc:
            if sys.exc_type is AssertionError:
                self.result.Annotate({"Assertion error" : str(exc)})
                self.result.Fail("Failed test assertion")
            elif sys.exc_type is gen.EndpointError:
                self.result.Annotate({'Endpoint error' : str(exc)})
                self.result.Fail('Call failed; an endpoint exception occurred')
            else:
                self.result.NoteException(cause="Exception executing source.")
                self.result.NoteException(cause=str(exc))
            return False

        return True

    def __executeAssertSource(self, assertSource, globalSpace, localSpace):
        try:
            exec assertSource in globalSpace, localSpace
        except AssertionError, theException:
            annotations = { 'Assertion' : str(theException) }
            self.result.Fail('Test assertion(s) false.', annotations)

    def __cleanup(self):
        """Generally testsource should cleanup all gens by stopping them,
           any long running commands by calling stopCommand, so this is
           just defensive programming against careless programmers.
        """
        #we are stopping the gens that were not stopped in the testsource due
        #to coding error if any.
        self._stopGens(self.context)

        # Stop commands created via runCommand
        for cmd in self.runList:
            print "stopping", cmd
            self.stopCommand(cmd)

    def Run(self, context, metaResult):
        """User level initialization and Execution a test.
        """

        # Attach to the log file
        self.log = logging.getLogger('nextestlog')
        self.context = context
        self.metaResult = metaResult

        # Output a banner to mark the start of this call
        self.__printBanner()

        #pre-requisite checks
        self.__preTestChecks()

        #set the sandbox environment
        globalSpace = { 'context': context,
                        'result': metaResult,
                        'test': self,
                        'myglobals':globals()}
        localSpace = {}
        self.runList = []
        
        #Get test data
        #32414 - Passing the context variable to the data source section
        if not (self.__executeDataSource(self.datasource, globalSpace,context)):
            #problem in reading test data, return
            return

        self.metaResult.subResults = []

        # 39583 - Verify whether the Data Source or Test Source section contains iserver restart
        srcList=[self.datasource,self.testsource]
        chkList=[False,False]
        i = 0
        for src1 in srcList:
            src = src1.lower()
            chk1 = src.__contains__('iserver all stop') or src.__contains__('ftest') or src.__contains__('checkiserver')
            chk2 = src.__contains__('checkscmstatus') or src.__contains__('restartscm') or src.__contains__('.initialize')
            chkList[i] = chk1 or chk2
            i += 1

        mswSess = None
        if not chkList[1]:
            mswSess = SSH(['root@mymsw'], ctxt=context)

        hVal1 = ''
        for testCase in self.context['nextest.metaTestData']:
            for testParam in testCase.keys():
                self.context['testData.%s'%testParam] = testCase[testParam]

            #create a new result object 
            self.result = Result(Result.TEST, self.metaResult.GetId() + '.' + testCase['testName'])
            self.metaResult.subResults.append(self.result)
            globalSpace['result'] = self.result
            

            #Start all data collectors.
            self.__startDataCollectors()

            # 39583 - Get the HVAL if test source does not restart iserver
            if not chkList[1]:
                if not chkList[0]:
                    hVal1 = mswSess.filter('cli test passive | grep -i hval')
                else:
                    chkList[0] = False
             
            # Need a flag so that assertions are not run if test has problems
            testStatus = self.__executeTestSource(self.testsource, globalSpace, localSpace)
    
            # Save all test data and annotate the result with the location
            # 32414 Included to annotate the result with metatestcase name
            self.__stopDataCollectors(testCase)
    
            # Execute the assertion code only if we didn't get a test exception
            if (testStatus) and (self.assertsource is not None):
                self.__executeAssertSource(self.assertsource, globalSpace, localSpace)

            # 39583 - Compare HVAL if the test case does not restart iserver and if the test case uses a H323 endpoint
            if not chkList[1]:
                h323EP = False
                # Verify whether the TC uses H323 endpoint
                for key in context.keys():
                    try:
                        if (type(context[key])==gen.Endpoint or type(context[key])==gatekeeper.gnuGk):
                            if (context[key].checkHVal):
                                h323EP = True
                                # Reset the value so that it does not affect some other test while run as a suite
                                context[key].checkHVal = False
                    except Exception, exc:
                        self.log.debug('CallTest: hVal verification: %s' % str(exc)) 

                if hVal1 and h323EP:
                    sleep(6)
                    hVal2 = mswSess.filter('cli test passive | grep -i hval')            
                    if (hVal1 != hVal2):
                        hVal1 = hVal1.strip('|hVal size =').strip('\r\n')
                        hVal2 = hVal2.strip('|hVal size =').strip('\r\n')
                        msg = '\n  HVAL CHECK WARNING       : HVal before test %s not equal to HVal after test %s!' %(hVal1,hVal2)
                        self.result[Result.CAUSE] = msg

            #cleanup any remains
            self.__cleanup()

        if mswSess:
            mswSess.disconnect()    

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

########################################################################
# Local Variables:
# mode: python
# indent-tabs-mode: nil
# fill-column: 78
# auto-fill-function: do-auto-fill
# End:
