########################################################################
#
# File:   html_result_stream.py
# Author: Scott Lowrey
# Date:   2004/06/10
#
# Contents:
#   QMTest TextResultStream class.
#
# Copyright (c) 2001, 2002, 2003 by CodeSourcery, LLC.  All rights reserved. 
#
# For license terms see the file COPYING.
#
########################################################################

########################################################################
# imports
########################################################################

import formatter
import htmllib
import StringIO
import qm.common
from   qm.test.base import *
from   qm.test.result import *
from   qm.test.file_result_stream import FileResultStream
import time

########################################################################
# classes
########################################################################

class HtmlResultStream(FileResultStream):
    """A 'HtmlResultStream' displays test results in HTML format.

    A 'HtmlResultStream' displays information hyper-textually, in human
    readable form.  This 'ResultStream' is used when QMTest is run
    without a graphical user interface."""

    arguments = [
        qm.fields.EnumerationField(
            name = "format",
            title = "Format",
            description = """The output format used by this result stream.

            There are three sections to the output:

            (S) Summary statistics.

            (I) Individual test-by-test results.

            (U) Individual test-by-test results for tests with unexpected
                outcomes.

            For each of the sections of individual test-by-test results, the
            results can be shown either in one of three modes:

            (A) Show all annotations.

            (N) Show no annotations.

            (U) Show annotations only if the test has an unexpected outcome.

            In the "brief" format, results for all tests are shown as
            they execute, with unexpected results displayed in full
            detail, followed by a list of all the tests with
            unexpected results in full detail, followed by the summary
            information.  This format is useful for interactive use:
            the user can see that the tests are running as they go,
            can attempt to fix failures while letting the remainder of
            the tests run, and can easily see the summary of the
            results later if the window in which the run is occurring
            is left unattended.

            In the "batch" format, statistics are displayed first
            followed by full results for tests with unexpected
            outcomes.  The batch format is useful when QMTest is run
            in batch mode, such as from an overnight job.  The first
            few lines of the results (often presented by email) give
            an overview of the results; the remainder of the file
            gives details about any tests with unexpected outcomes.

            The "full" format is like "brief" except that all
            annotations are shown for tests as they are run.

            In the "stats" format only the summary statistics are
            displayed.""",
            enumerals = ["brief", "batch", "full", "stats"]),
        qm.fields.TextField(
            name = "statistics_format",
            title = "Statistics Format",
            verbatim = "true",
            multiline = "true",
            description = """The format string used to display statistics.

            The format string is an ordinary Python format string.
            The following fill-ins are available:

            'TOTAL' -- The total number of tests.

            'EXPECTED' -- The total number of tests that had an
            expected outcome.

            'EXPECTED_PERCENT' -- The percentage of tests with
            expected outcomes.

            'UNEXPECTED' -- The total number of tests that had an 
            unexpected outcome.

            For each outcome 'O', there are additional fill-ins:

            'O' -- The total number of tests with outcome 'O'.
            
          n    NexTestbranch
      'O_PERCENT' -- The percentage of tests with outcome 'O' to
            total tests, as a floating point value.

            'O_UNEXPECTED' -- The total number of tests with an
            unexpected outcome of 'O'.

            'O_UNEXEPECTED_PERCENT' -- The ratio of tests without an
            unexpected outcome of 'O' to total tests, as a floating
            point value."""),
        ]
    
    def __init__(self, arguments):
        """Construct an 'HtmlResultStream'.

        'arguments' -- The arguments to this result stream.

        'suite_ids' -- The suites that will be executed during the
        test run."""

        # Initialize the base class.
        super(HtmlResultStream, self).__init__(arguments)

        # Pick a default format.
        if not self.format:
            self.format = "batch"
            try:
                if self.file.isatty():
                    self.format = "brief"
            except:
                pass
            
        self.__first_test = 1
        # Keep track of tests and resources that had unexpected outcomes.
        self.__unexpected_test_results = []
        self.__unexpected_resource_results = []
        # And how many tests were run.
        self.__num_tests = 0
        # And how many tests there are with each outcome.
        self.__outcome_counts = {}
        for o in Result.outcomes:
            self.__outcome_counts[o] = 0
        # And how many unexpected tests there are with each outcome.
        self.__unexpected_outcome_counts = {}
        for o in Result.outcomes:
            self.__unexpected_outcome_counts[o] = 0
        # For table closure
        self.__table = 0
        self.__resultTable = []

    def WriteAnnotation(self, key, value):
        fmt = "<b>%s:</b> %s<br>"
        if key == 'qmtest.run.start_time':
            bintime = parse_time_iso(value)
            strtime = qm.common.format_time(bintime)
            self.file.write(fmt % ('Test Start Time', strtime))
        elif key == 'qmtest.run.end_time':
            bintime = parse_time_iso(value)
            strtime = qm.common.format_time(bintime)
            self.file.write(fmt %('Test End Time', strtime))
    
    def WriteResult(self, result):
        """Output a test or resource result.

        'result' -- A 'Result'."""

        # Record the results as they are received.
        if result.GetKind() == Result.TEST:
            # Remember how many tests we have processed.
            self.__num_tests += 1
            # Increment the outcome count.
            outcome = result.GetOutcome()
            self.__outcome_counts[outcome] += 1
            # Remember tests with unexpected results so that we can
            # display them at the end of the run.
            test_id = result.GetId()
            expected_outcome \
                = self.expected_outcomes.get(result.GetId(), Result.PASS)
            if self.format != "stats" and outcome != expected_outcome:
                self.__unexpected_outcome_counts[outcome] += 1
                self.__unexpected_test_results.append(result)
        else:
            if (self.format != "stats"
                and result.GetOutcome() != result.PASS):
                self.__unexpected_resource_results.append(result)
            
        # In some modes, no results are displayed as the tests are run.
        if self.format == "batch" or self.format == "stats":
            return

        # Start a row
        outcome = result.GetOutcome()
        color = self._GetColor(outcome)
        self.__row = ('<tr bgcolor=%s>' % color)
        
	# Display the result.
	self._DisplayResult(result, self.format)

        # Display annotations associated with the test.
        if outcome != Result.PASS:
           self._DisplayAnnotations(result)

        # Display a link to the test data
        self._DisplayDataURL(result)
        
        # End the row
        self.__row += ("</tr>")
        self.__resultTable.append(self.__row)

    def Summarize(self):
        """Output summary information about the results.

        When this method is called, the test run is complete.  Summary
        information should be displayed for the user, if appropriate.
        Any finalization, such as the closing of open files, should
        also be performed at this point."""

        self._SummarizeTestStats()
        self._DisplayResultTable()
        
        # Show results for tests with unexpected outcomes.
        if self.format in ("full", "brief", "batch"):
            compare_ids = lambda r1, r2: cmp(r1.GetId(), r2.GetId())

            # Sort test results by ID.
            self.__unexpected_test_results.sort(compare_ids)
            # Print individual test results.
            if self.expected_outcomes:
                self._DisplayHeading("Tests With Unexpected Outcomes")
            else:
                self._DisplayHeading("Tests That Did Not Pass")
            self._SummarizeResults(self.__unexpected_test_results)

            if self.__unexpected_resource_results:
                # Sort resource results by ID.
                self.__unexpected_resource_results.sort(compare_ids)
                # Print individual resource results.
                self._DisplayHeading("Resources That Did Not Pass")
                self._SummarizeResults(self.__unexpected_resource_results)

        # Invoke the base class method.
        super(HtmlResultStream, self).Summarize()


    def _DisplayStatistics(self):
        """Write out statistical information about the results.

        Write out statistical information about the results."""

        # Summarize the test statistics.
        if self.statistics_format:
            self._FormatStatistics(self.statistics_format)
        elif self.expected_outcomes:
            self._SummarizeRelativeTestStats()
        else:
            self._SummarizeTestStats()

    def _SummarizeTestStats(self):
        """Generate statistics about the overall results."""

        self._DisplayHeading("Test Statistics")
        head = []
        self._StartTable(('Category', 'Count', 'Percentage'))
        # Build the format string.  If there are no tests we do not
        # provide any output.
        format = ""
        for o in Result.outcomes:
            color = self._GetColor(o)
            format += ("<tr bgcolor=%s>"
                       "<td>%s</td>"
                       "<td align=right>%%(%s)6d</td>"
                       "<td align=right>%%(%s)3.0f%%%%</td></tr>"
                       % (color, o, o, o + '_PERCENT'))
	
        self._FormatStatistics(format)
        self._EndTable()

        
    def _SummarizeRelativeTestStats(self):
        """Generate statistics showing results relative to expectations."""

        self._DisplayHeading("Relative Stats")

        # Build the format string.  If there are no tests we do not
        # provide any output.
        if self.__num_tests != 0:
            # Indicate the total number of tests.
            format = ("  %(EXPECTED)6d (%(EXPECTED_PERCENT)3.0f%%) "
                      "tests as expected\n")
            # Include a line for each outcome.
            for o in Result.outcomes:
                if self.__unexpected_outcome_counts[o] != 0:
                    format += ("  %%(%s)6d (%%(%s)3.0f%%%%) tests "
                               "unexpected %s\n"
                               % (o + "_UNEXPECTED",
                                  o + "_UNEXPECTED_PERCENT",
                                  o))
            format += "<br>"
        else:
            format = ""

        self._FormatStatistics(format)


    def _FormatStatistics(self, format):
        """Output statistical information.

        'format' -- A format string with (optional) fill-ins
        corresponding to statistical information.

        The formatted string is written to the result file."""

        # Build the dictionary of format fill-ins.
        num_tests = self.__num_tests
        unexpected = len(self.__unexpected_test_results)
        expected = num_tests - unexpected
        values = { "TOTAL" : num_tests,
                   "EXPECTED" : expected,
                   "UNEXPECTED" : unexpected }
        if num_tests:
            values["EXPECTED_PERCENT"] = (100. * expected) / num_tests
        else:
            values["EXPECTED_PERCENT"] = 0.0
        for o in Result.outcomes:
            count = self.__outcome_counts[o]
            values[o] = count
            if num_tests:
                values[o + "_PERCENT"] = (100. * count) / num_tests
            else:
                values[o + "_PERCENT"] = 0.0
            count = self.__unexpected_outcome_counts[o]
            values[o + "_UNEXPECTED"] = count
            if num_tests:
                values[o + "_UNEXPECTED_PERCENT"] = (100. * count) / num_tests
            else:
                values[o + "_UNEXPECTED_PERCENT"] = 0.0

        self.file.write(format % values)

        
    def _SummarizeResults(self, results):
        """Summarize each of the results.

        'results' -- The sequence of 'Result' objects to summarize."""

        if len(results) == 0:
            self.file.write("  None.\n\n")
            return

        # Generate them.
        self._StartTable(('Test ID', 'Outcome', 'Cause'))
	for result in results:
            self._DisplayResult(result, self.format)
        self._EndTable()


    def _DisplayResult(self, result, format):
	"""Display 'result'.

	'result' -- The 'Result' of a test or resource execution.

        'format' -- The format to use when displaying results."""

	id_ = result.GetId()
        kind = result.GetKind()
	outcome = result.GetOutcome()
            
	# Print the ID and outcome.
	if self.expected_outcomes:
	    # If expected outcomes were specified, print the expected
	    # outcome too.
	    expected_outcome = \
	        self.expected_outcomes.get(result, Result.PASS)
            if (outcome == Result.PASS
                and expected_outcome == Result.FAIL):
                self._WriteOutcome(result, "XPASS")
            elif (outcome == Result.FAIL
                  and expected_outcome == Result.FAIL):
                self._WriteOutcome(result, "XFAIL")
            elif outcome != expected_outcome:
                self._WriteOutcome(result, expected_outcome)
            else:
                self._WriteOutcome(result)
	else:
            self._WriteOutcome(result)


    def _DisplayAnnotations(self, result):
        """Display the annotations associated with 'result'.

        'result' -- The 'Result' to dispay."""

        keys = result.keys()
        keys.sort()
        exclusions = (Result.CAUSE, "qmtest.target")
        for name in keys:
            # The CAUSE property has already been displayed."
            if (name in exclusions) or (name[:7] == "nextest"):
                continue
            self.__row += "<td><b>%s:</b> %s</td>" % (name, result[name]) 

    def _DisplayDataURL(self, result):
        for key in result.keys():
            if key[:7] == "nextest":
                val = result[key]
                self.__row += '<td><a href="%s">%s</a></td>' % (val, key)
        
    def _WriteOutcome(self, result, expected_outcome=None):
        """Write a line indicating the outcome of a test or resource.

        Each outcome is written to a single row in a table.  The row
        must be created by the caller.
        
        'name' -- The name of the test or resource.

        'kind' -- The kind of result being displayed.
        
        'outcome' -- A string giving the outcome.

        'expected_outcome' -- If not 'None', the expected outcome."""

	name = result.GetId()
        kind = result.GetKind()
	outcome = result.GetOutcome()

        if kind == Result.RESOURCE_SETUP:
            type = "Resource Setup"
        elif kind == Result.RESOURCE_CLEANUP:
            type = "Resource Cleanup"
        else:
            type = "Test"
        # Each result is a new row in a table.  Table is created by
        # the caller.
           
        if expected_outcome:
	    self.__row += ("<td>%s</td> <td>%s</td> <td>%s</td> <td>%s</td>"
                   % (type, name, outcome, expected_outcome))
	else:
	    self.__row += ("<td>%s</td> <td>%s</td> <td>%s</td>" % \
                   (type, name, outcome))
        # Print the cause of the failure.
        if result.has_key(Result.CAUSE):
            self.__row += ("<td>%s</td>" % result[Result.CAUSE])
        else:
            self.__row += ("<td></td>")

    def _DisplayResultTable(self):
        # Display a heading before the first result.
        self._DisplayHeading("Test Results")
        self._StartTable(('Type', 'Test ID', 'Result', 'Cause'))
        self.__resultTable.sort()
        self.__resultTable.reverse()
        for row in self.__resultTable:
            self.file.write(row)
            self.file.write("\n")
        self._EndTable()
            
    def _DisplayHeading(self, heading):
        """Display 'heading'.

        'heading' -- The string to use as a heading for the next
        section of the report."""

        self.file.write("<h3>%s</h3>" % heading)

    def _StartTable(self, headings):
        format = "<table><thead align=center><tr bgcolor=#d0d0d0>"
        for i in headings:
            format += "<td><b>%s</b></td>"
        format += "</thead></tr>\n"
        self.file.write(format % headings)

    def _EndTable(self):
        self.file.write("</table>\n")

    def _GetColor(self, outcome):
        if outcome == Result.FAIL:
            return '#ff9090'
        elif outcome == Result.ERROR:
            return '#ffffe0'
        elif outcome == Result.PASS:
            return '#a0f0a0'
        elif outcome == Result.UNTESTED:
            return '#d0d0d0'

# This function is included in qm.common latest version.  Need to
# upgrade the libs.
def parse_time_iso(time_string):
    """Parse a ISO8601-compliant formatted date and time.

    See also 'format_time_iso'.

    'time_string' -- The string to be parsed, as returned by
    e.g. 'format_time_iso'.

    returns -- The time as a float, like that returned by
    'time.time'."""

    return time.mktime(time.strptime(time_string, "%Y-%m-%dT%H:%M:%SZ"))
