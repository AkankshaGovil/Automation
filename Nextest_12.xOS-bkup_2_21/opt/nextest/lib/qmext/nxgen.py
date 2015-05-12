#!/usr/bin/python
"""
Control resources for test automation.

This class is used to store the Nxgen information.
"""
import pexpect
import qm.common
import qm.fields
from   qm.test.resource import *
import logging
import nxgenInfo

class Nxgen(Resource):
    """
    This class contains the information required by the Nxgen resource.
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
            name="mediaipaddr",
            title="Media IP address",
            not_empty_text=True,
            description="""
            """
            ),
        qm.fields.TextField(
            name="mgenmgmtipaddr",
            title="MGEN Management IP address",
            not_empty_text=True,
            description="""
            """
            )
        ]

    def SetUp(self, context, result):
        """
        Create a NxGenInfo object 
        """
        Info = nxgenInfo.NxgenInfo(self.mediaipaddr,self.mgenmgmtipaddr)
        context[self.property] = Info

########################################################################
# Local Variables:
# mode: python
# indent-tabs-mode: nil
# fill-column: 78
# auto-fill-function: do-auto-fill
# End:
