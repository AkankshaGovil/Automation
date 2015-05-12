"""
Benchmark test classes
"""
import os
import qm.fields
from   qm.test.test import *
import signal
import string

class SIPStone(Test):
    """
    Run the SIPP test tool and collect statistical output.
    """
    arguments = [
        qm.fields.IntegerField(
            name="cps",
            title="Call Rate",
            description="Number of calls per second to attempt.",
            default_value=10
            ),
        qm.fields.IntegerField(
            name="acd",
            title="Average Call Duration",
            description="Average call duration in seconds.",
            default_value=0
            ),
        qm.fields.IntegerField(
            name="maxcalls",
            title="Maximum Calls",
            description="The max number of calls to make.  Must be > 0.",
            default_value=10
            ),
        qm.fields.TextField(
            name="uacaddr",
            title="UAC Local Address",
            description="The name or IP address of the SIP client",
            default_value="pub_ep"
            ),
        qm.fields.TextField(
            name="uasaddr",
            title="UAS Remote Address",
            description="The name or IP address of the SIP server",
            default_value="prv_ep"
            ),
        qm.fields.TextField(
            name="proxyaddr",
            title="Proxy Address",
            description="The name or IP address of the SIP proxy."
            ),
        qm.fields.TextField(
            name="remoteuser",
            title="Remote User",
            description="The user name/number to call."
            ),
        qm.fields.EnumerationField(
            name="transmode",
            title="Transport Mode",
            description="The network transport mode for messages.",
            enumerals=['UDP single socket',
                       'UDP socket-per-call',
                       'TCP single socket',
                       'TCP socket-per-call']
            ),
        qm.fields.BooleanField(
            name="tracemsg",
            title="Trace Messages",
            description="Save all messages in sipp_messages.log",
            default_value="false"
            ),
        qm.fields.BooleanField(
            name="tracestat",
            title="Trace Statistics",
            description="Save statistical information in scenario_name.csv",
            default_value="true"
            ),
        qm.fields.IntegerField(
            name="statfreq",
            title="Statistics Update Frequency",
            description="The interval between stat file updates (seconds)",
            default_value=60
            )
        ]

    def Run(self, context, result):
        """
        Run two sipp processes, client and server on the local host.
        """
        transmode_arg = { 'UDP single socket' : 'u1',
                          'UDP socket-per-call' : 'un',
                          'TCP single socket' : 't1',
                          'TCP socket-per-call' : 'tn'}
        uas_args = ['sipp',
                    '-bg',
                    '-i', self.uasaddr,
                    '-sn', 'uas',
                    '-t', transmode_arg[self.transmode],
                    ]
        print "startserver: sipp %s" % string.join(uas_args)
        uas = os.spawnvp(os.P_NOWAIT, 'sipp', uas_args)

        trace_stat = ''
        if self.tracestat == "true": trace_stat = '-trace_stat'
        trace_msg = ''
        if self.tracemsg == "true": trace_msg = '-trace_msg'
        
        uac_args = ['sipp',
                    '-bg',
                    self.proxyaddr,
                    '-i', self.uacaddr,
                    '-r', str(self.cps),
                    '-d', str(self.acd),
                    '-t', transmode_arg[self.transmode],
                    '-s', self.remoteuser,
                    '-m', str(self.maxcalls),
                    trace_stat,
                    trace_msg,
                    '-fd', str(self.statfreq)]
        print 'client: sipp %s' % string.join(uac_args)
        uac = os.spawnvp(os.P_WAIT, 'sipp', uac_args)
        while uac.isalive():
            print "client is alive"
            time.sleep(2)
        print "client is done"
        uas.kill(signal.SIGINT)
    
########################################################################
# Local Variables:
# mode: python
# indent-tabs-mode: nil
# fill-column: 78
# auto-fill-function: do-auto-fill
# End:
