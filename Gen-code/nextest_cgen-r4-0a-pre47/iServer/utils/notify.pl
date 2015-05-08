#!/usr/bin/perl

#######################################################
#######################################################
###
### notify.pl
###
### Example mail based notification program for use 
### with cdrpp.
###
### (c) 2002 NexTone Communications, Inc.
#######################################################
#######################################################


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

#use lib "/usr/local/nextone/lib/perl5/site_perl";

if ($ENV{BASE})
{
	use lib "$ENV{BASE}/lib/perl5/site_perl";
}

use Mail::Sendmail;

$date = `date`;
chomp $date;

##############################
##############################
## Setup mail variables here.
##############################
##############################

## Who the message is from
$MailFrom = 'sridhar@nextone.com';

## Subject of the mail.
$MailSubject = "cdrpp Report " . $date;

## List of Receipients
$MAILLIST = 'greene@nextone.com, sridhar@nextone.com';

## CC list
$MailCC = '';

## BCC list
$MailBCC = '';


##############################
## Process Mail message.
##############################
$InFile = $ARGV[0];

open (INF, $InFile) or die "Cannot open $InFile \n";

$MailMessage = "";

while (<INF>)
{
	$MailMessage .= $_;
}

close (INF);


##############################
## Send the mail.
##############################
print "Using default server: $Mail::Sendmail::mailcfg{smtp}->[0] \n";
print "Using default sender: $Mail::Sendmail::mailcfg{from} \n";


%mail = (
		To      => $MAILLIST,
		From    => $MailFrom,
		Cc      => $MailCC,
		Bcc     => $MailBCC,
		Subject => $MailSubject,
		Message => $MailMessage,
	);

if (sendmail(%mail))
{
	print "Mail sent ok\n";
}
else
{
	print "Error sending mail: $Mail::Sendmail::error \n";
}

print "OK. Log says $Mail::Sendmail::log \n";

