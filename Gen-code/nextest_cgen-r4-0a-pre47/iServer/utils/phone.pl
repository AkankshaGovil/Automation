#!/usr/bin/perl

############################################################################
############################################################################
###
###	Phone.pl
###
###	Calls two numbers automatically.
###
###	Copyright (C) 2002, NexTone Communications.
############################################################################
############################################################################

############################################################################
### Phone Numbers Defaults
### Edit the following two values to modify defaults.
############################################################################
$phone1 = "2404536251";
$phone2 = "3014424001";
############################################################################


## Preliminary
use English;

my $CurrentVersion = sprintf "%vd", $PERL_VERSION;

if ($CurrentVersion lt "5.6")
{
	use lib "/usr/local/nextone/lib/perl5/site_perl/5.005";
}
else
{
	use lib "/usr/local/nextone/lib/perl5/site_perl";
}

if ($ENV{BASE})
{
	use lib "$ENV{BASE}/lib/perl5/site_perl";
}

use Getopt::Std;
use Net::Telnet;

$pProgName = "phone.pl";
$pProgVersion = "v0.1, 05/07/2002";

getopts "vh";


if ($opt_v)
{
	PrintVersion ();
	exit (0);
}

if ($opt_h)
{
	PrintHelp ();
	exit (0);
}


if ($ARGV[0])
{
	$phone1 = $ARGV[0];
	$phone2 = $ARGV[1];
}

$iPortalHost = "207.113.13.230";
$iPortalPort = "12979";

$t = new Net::Telnet (Timeout => 15);

$t->open (Host => $iPortalHost,
	  Port => $iPortalPort);

$res = $t->print ("$phone1 $phone2");

$t->close;


sub PrintHelp ()
{
	print <<eEOF
$pProgName, $pProgVersion
Usage:
$pProgName phone1 phone2, where phone1 and phone2 are numbers to be called.
$pProgName uses butterfly signaling (3pcc) through an iServer to place the calls.
Options are:
-h                - prints this
-v                - prints out version
eEOF
;
}

sub PrintVersion ()
{
	print "$pProgName, $pProgVersion \n";
	print "Copyright 2002, NexTone Communications, Inc.\n";
}

