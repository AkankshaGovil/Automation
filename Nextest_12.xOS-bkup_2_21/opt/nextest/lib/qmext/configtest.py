
"""Test class for tests written in Python."""

########################################################################
# imports
########################################################################

import data
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

########################################################################
# classes
########################################################################

class ConfigTestError(Exception):
    "General exception for ConfigTest"

class OutCome(object):
	def __init__(self):
	      self.result = []

      	def appendResult(self , out):
             self.result.append(out)

	def showResult(self ):
          message = " <br> There are %d Errors \n  <br>" %(len(self.result))
          if len(self.result):
             for i in self.result:
                 message +=" <br>" 
                 message += i
                 message +=" <br>" 
             return False , message
          else:
             return True ,message 


class ConfigTest(Test):
    """
	Execute one or more CLI commands .
    """

    arguments = [
        qm.fields.TextField(
            name="testDocumentation",
            title="Test documentation",
            description="""Test Case Documentation.
            This argument is used to document the test case using special markup tags.
            The documentation is extracted by a scipt and converted into a word formatted
            test plan. This argument is not used by the test case run environment. The following
            markup tags are defined:

            @name - name of the test case. This also become the title of the test.

            @objective - objective of the test case. A newline character breaks a paragraphs.
                         Multiple paragraphs are allowed.

            @procedure - starts a procedure section followed by a few paragraphs of description. 
                         it is followed by a sequence of @procedure_step and @expect paragraphs. 
            
            @procedure_step - one step in the procedure.

            @expect - result expected from the system under test.
           
            @reference any references

            @caveats  any caveats.  """,
            verbatim="true",
            multiline="true",
            default_value="""Write test description here."""
            ),

        qm.fields.TextField(
            name="testsource",
            title="Execution Code",
            description="Source code to be executed",
            verbatim="true",
            multiline="true",
            ),

        qm.fields.TextField(
            name="assertsource",
            title="Assertions",
            description="Assertion code",
            verbatim="true",
            multiline="true",
            default_value="True"
            )


       ]


    def Run(self, context, result):
        # Attach to the log file
        self.log = logging.getLogger('nextestlog')
        # Output a banner to mark the start of this call
        self.log.debug('ConfigTest: Run: ####')
        self.log.debug('ConfigTest: Run: ####')
        self.log.debug('ConfigTest: Run: #### Running ConfigTest  %s'
                        % context['qmtest.id'])
        self.log.debug('ConfigTest: Run: ####')
        self.log.debug('ConfigTest: Run: ####')

        if not context.has_key('mswinfo'):
            raise ConfigTestError("ConfigTest requires a MSW resource")


        globalSpace = { 'context': context,
                        'result': result,
                        'test': self,
                        'myglobals':globals()}
        localSpace = {}
        self.runList = []

        self.outCome = OutCome()
        # Need a flag so that assertions are not run if test has problems
        testException = False

        try:
            exec self.testsource in globalSpace, localSpace
            self.log.info("ConfigTest: run: call test source done")
        except Exception, exc:
            testException = True
            if sys.exc_type is AssertionError:
                result.Annotate({"Assertion error" : str(exc)})
                result.Fail("Failed test assertion")
            else:
                result.NoteException(cause="Exception executing source.")
                result.NoteException(cause=str(exc))

            # Stop commands created via runCommand
            for cmd in self.runList:
                print "stopping", cmd
                self.stopCommand(cmd)

        # Save all test data and annotate the result with the location

        # Execute the assertion code only if we didn't get a test exception
        if (not testException) and (self.assertsource is not None):
            try:
                exec self.assertsource in globalSpace, localSpace
            except AssertionError, theException:
                # put the '00' in so assertion appears first in list
                annotations = { 'Assertion' : str(theException) }
                result.Fail('Test assertion(s) false.', annotations)

    ######################################################################
    ## Methods called from test code.
    ######################################################################
    def assert_(self, expr, message=None):
        if not expr:
            raise AssertionError(message)


    def assertEquals(self, first, second, message=None, third=0,stop=True):
        if third == 0:
            if first != second:
                if not message:
                    message = '"%s" is not equal to "%s"' % (first, second)
                self.outCome.appendResult(message)
        else:
            if first == second:
                if not message:
                    message = '"%s" is equal to "%s"' % (first, second)
                self.outCome.appendResult(message)

        if stop:
            finalRet, finalMsg = self.outCome.showResult()
            if not finalRet  :
                   raise AssertionError(finalMsg)

    def assertFinalResult(self):
        finalRet, finalMsg = self.outCome.showResult()
        if not finalRet  :
           raise AssertionError(finalMsg)



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
            raise ConfigTestError("command not registered")
        if proc.isalive():
            proc.kill(9)
        self.runList.remove(proc)

    #######################################################################
    ## private method
    #######################################################################

