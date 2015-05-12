"Long help buffers used by NexTone QM extenstions."

class CDRTestHelp(object):
    """Help text for QMTest fields.

    Moved here to make the code easier to read.
    """
    assertsource =  """The assertion source code.

    This Python code examines assertions about the calls that
    took place during the Execution phase.  Any CDRs collected
    are available as a list of CDR objects via the context key
    'cdrlist'. 

    The test is verified by calling the assertion methods of
    this test class (see Python documentation for the CDRTest
    class).  If any assertion is false, the test will fail and
    the result will be annotated with the reason and the
    contents of the first CDR.

    The keys for accessing CDR fields follow

    - 01-start-time

    - 02-start-time

    - 03-call-duration

    - 04-call-source

    - 05-call-source-q931sig-port

    - 06-call-dest

    - 07-

    - 08-call-source-custid

    - 09-called-party-on-dest

    - 10-called-party-from-src

    - 11-call-type

    - 12-

    - 13-disconnect-error-type

    - 14-call-error

    - 15-call-error

    - 16-

    - 17-

    - 18-ani

    - 19-

    - 20-

    - 21-

    - 22-cdr-seq-no

    - 23-

    - 24-callid

    - 25-call-hold-time

    - 26-call-source-regid

    - 27-call-source-uport

    - 28-call-dest-regid

    - 29-call-dest-uport

    - 30-isdn-cause-code

    - 31-called-party-after-src-calling-plan

    - 32-call-error-dest

    - 33-call-error-dest

    - 34-call-error-event-str

    - 35-new-ani

    - 36-call-duration

    - 37-incoming-leg-callid

    - 38-protocol

    - 39-cdr-type

    - 40-hunting-attempts

    - 41-caller-trunk-group

    - 42-call-pdd

    - 43-h323-dest-ras-error

    - 44-h323-dest-h225-error

    - 45-sip-dest-respcode

    - 46-dest-trunk-group

    - 47-call-duration-fractional

    - 48-timezone

    - 49-msw-name

    - 50-called-party-after-transit-route

    - 51-called-party-on-dest-num-type

    - 52-called-party-from-src-num-type
    """

    #test documentation
    testDocumentation =  """Test Case Documentation.

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

    @caveats  any caveats.

    """

    testsource = """The code that generates one or more CDRs.

    This Python code may contain class definitions, function
    definitions, statements, and so forth.  If this code
    throws an uncaught exception, the test will fail.
    
    Use the configure() method of the gen class to set up a
    call generator.  The configure() method takes a protocol
    as its first argument: 'sip' or 'h323'.  The remainder of
    the arguments are call gen properties and may be specified
    in any order using keyword arguments.  There are a few
    basic rules about options which are enforced by the
    session module.

    *TODO  Document the rules*

    The call gen properties common to both SIP and H.323 are
    as follows (square brackets indicate the default value).

    - mode --		send or receive

    - regid --		registration ID (not H.323 ID)

    - srcnum --		starting phone number of caller ['555']

    - destnum --		starting phone number of callee ['666']

    - srcaddr --		source name or IP of outgoing calls

    - gateway --		destination gateway name or IP

    - gwport --		dest gateway port

    - numcalls --		number of calls to make ['1']

    - media --		enable or disable

    - response --		cause code from when listening

    - mediastartport --	starting port for RTP

    - origtrunkgroup --	originating trunk group

    - desttrunkgroup --	destination trunk group


    Properties specific to H.323:

    - file --		 name of gen config file

    - start --		 fast, slow, fast-no-245

    - h323id --		 H.323 ID

    - fax --			 FAX call

    - gkaddr --		 gatekeeper address

    - maxcalls --		 number of calls to run to [20] ?


    Properties specific to SIP:

    - desturi --		 destination URI

    - srcuri --		 source URI

    - register --		 register before call

    """

class CallTestHelp(object):
    """Help text for QMTest fields.

    Moved here to make the code easier to read.
    """
    assertsource =  """The assertion source code.

    The test is verified by calling the assertion methods of
    this test class (see Python documentation for the CDRTest
    class).  If any assertion is false, the test will fail and
    the result will be annotated with the reason and the
    contents of the first CDR.

    The keys for accessing CDR fields follow

    - 01-start-time

    - 02-start-time

    - 03-call-duration

    - 04-call-source

    - 05-call-source-q931sig-port

    - 06-call-dest

    - 07-

    - 08-call-source-custid

    - 09-called-party-on-dest

    - 10-called-party-from-src

    - 11-call-type

    - 12-

    - 13-disconnect-error-type

    - 14-call-error

    - 15-call-error

    - 16-

    - 17-

    - 18-ani

    - 19-

    - 20-

    - 21-

    - 22-cdr-seq-no

    - 23-

    - 24-callid

    - 25-call-hold-time

    - 26-call-source-regid

    - 27-call-source-uport

    - 28-call-dest-regid

    - 29-call-dest-uport

    - 30-isdn-cause-code

    - 31-called-party-after-src-calling-plan

    - 32-call-error-dest

    - 33-call-error-dest

    - 34-call-error-event-str

    - 35-new-ani

    - 36-call-duration

    - 37-incoming-leg-callid

    - 38-protocol

    - 39-cdr-type

    - 40-hunting-attempts

    - 41-caller-trunk-group

    - 42-call-pdd

    - 43-h323-dest-ras-error

    - 44-h323-dest-h225-error

    - 45-sip-dest-respcode

    - 46-dest-trunk-group

    - 47-call-duration-fractional

    - 48-timezone

    - 49-msw-name

    - 50-called-party-after-transit-route

    - 51-called-party-on-dest-num-type

    - 52-called-party-from-src-num-type
    """

    cdrcollect =  """Enable CDR Collection.

    Enable CDR Collection. CDR Collection will be performed during 
    this test and will be saved in the results database after the 
    test completes.
    """
    
    pkttrace =  """Enable packet tracing.

    Choose "ON" to enable packet tracing. Packet trace will be 
    performed during this test on interfaces e1000g0 and e1000g1, 
    and will be combined in one file of type: .pcap. This file will 
    be placed in the results database after the test completes. It 
    will be of TCPDump format, readable by Ethereal.
    """
    
    dbglog =  """Enable debug log saving.

    Choose "ON" to enable debug log saving. The debug log files for 
    tests that have this optin set will be saved on a per-test 
    basis in the output database.
    """
   
    #SCM testing addon
    redundant =  """Enable Redundancy Configuration.

    Choose "ON" to enable redundancy configuration. The CDR etc. will 
    be captured from 1 or 2 MSWs accordingly. Tests that have this optin 
    set will be saved on a per-test basis in the database.
    """
    
    testsource = """The code that generates one or more CDRs.

    This Python code may contain class definitions, function
    definitions, statements, and so forth.  If this code
    throws an uncaught exception, the test will fail.
    
    Use the configure() method of the gen class to set up a
    call generator.  The configure() method takes a protocol
    as its first argument: 'sip' or 'h323'.  The remainder of
    the arguments are call gen properties and may be specified
    in any order using keyword arguments.  There are a few
    basic rules about options which are enforced by the
    session module.

    *TODO  Document the rules*

    The call gen properties common to both SIP and H.323 are
    as follows (square brackets indicate the default value).

    - mode --		send or receive

    - regid --		registration ID (not H.323 ID)

    - srcnum --		starting phone number of caller ['555']

    - destnum --		starting phone number of callee ['666']

    - srcaddr --		source name or IP of outgoing calls

    - gateway --		destination gateway name or IP

    - gwport --		dest gateway port

    - numcalls --		number of calls to make ['1']

    - media --		enable or disable

    - response --		cause code from when listening

    - mediastartport --	starting port for RTP

    - origtrunkgroup --	originating trunk group

    - desttrunkgroup --	destination trunk group


    Properties specific to H.323:

    - file --		 name of gen config file

    - start --		 fast, slow, fast-no-245

    - h323id --		 H.323 ID

    - fax --			 FAX call

    - gkaddr --		 gatekeeper address

    - maxcalls --		 number of calls to run to [20] ?


    Properties specific to SIP:

    - desturi --		 destination URI

    - srcuri --		 source URI

    - register --		 register before call

    """

    #test documentation
    testDocumentation =  """Test Case Documentation.

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

    @caveats  any caveats.

    """
