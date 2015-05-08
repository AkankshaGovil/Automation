#!/usr/local/bin/perl

use Getopt::Std;
# use Time::Timezone;
# use Time::Local;
use POSIX qw(strftime);

getopts('i:a:w:h');
($prog = $0) =~ s/^.*\///g;
$Usage = "Usage: $prog [-h] -i addr [-a sessid] [-w \"string\"]\n";
$Usage1 = "\t-i IP address of MSW from which these data were obtained\n";
$Usage2 = "\t-a starting accounting session id (default 1)\n";
$Usage3 = "\t-w text to wait for before starting to interpret CDRs\n";
$Usage4 = "\tInput from stdin, output to stdout\n";

die "$Usage$Usage1$Usage2$Usage3$Usage4" if ($opt_h || !$opt_i);

if ($opt_a) {
	$acctSessionId = $opt_a;
} else {
	$acctSessionId = 1;
}

my $count = 0;
my $found = 0;

if (!$opt_w) {$found=1;} # else { print "looking for $opt_w\n"; }

while (<>) {
	$count++;
	if (!$found) {
		$found = /$opt_w/;
	}
	if ($found) {
		chomp;
		my @cdr = split (';');

	#	my $cdrlen = @cdr;
	#	print "CDR has $cdrlen fields\n";
	#	foreach $i (0 .. $#cdr) {
	#		print "\$cdr[$i] = '$cdr[$i]'\n";
	#	}


		my $now=time;

		# Current date and time to start
		# Date, Time (Current Date, not related to call)
		printf "%s", strftime("%m/%d/%Y,%T", localtime($now));

		# Called-Station-Id (CDR Field 10)
		printf ",%s", $cdr[9];

		# Calling-Station-Id (CDR Field 35)
		printf ",%s", $cdr[34];

		# Acct-Status-Type (CDR 39, Map startX to Start, endX to Stop)
		if (index $cdr[38],"end" >= 0) {
			print ",Stop";
		} else {
			print ",Start";
		}

		# Acct-Session-Id (**Not in CDR **)
		print ",", $acctSessionId++;

		# Acct-Session-Time (call duration, seconds) (CDR 36)
		print ",", $cdr[35];

		# Service-Type (Constant: "Login")
		print ",Login";

		# NAS-IP-Address (** Not in CDR **)
		print ",", $opt_i;

		# NAS-Port (Constant: 0)
		print ",0";

		# NAS-Identifier (CDR 49)
		print ",", $cdr[48];

		# Proxy-State, Connect-Info, cisco-nas-port (Constants: blank)
		print ",,,";

		# cisco-av-pair  This is a space separated field, no commas for a while.
		$incomingConfId = $cdr[23];
		print ",h323-incoming-conf-id=", $incomingConfId;	# (CDR 37, seems to be blank)
		print " subscriber=Subscriber";
		if ($cdr[10] eq "IF") {				# (CDR 47, convert from x.xxx)
			($dur, $durms) = split ('\.', $cdr[46]);
			$durms += ($dur*1000);
			print (" fax-tx-duration=$durms");
		}
		print " session-protocol=",uc($cdr[37]);								# (CDR 38, convert to UCASE)

		my $leg = substr($cdr[38],-1,1);

	#	print " remote-media-address=", inet_ntoa(pack('N', getval($remoteMediaIpaddr))); # (** Not inCDR ? **)
		print " remote-media-address=0.0.0.0"; # (** Not inCDR ? **)
		print " in-trunkgroup-label=", $cdr[40];		# (CDR 41)
		print " in-carrier-id=", $cdr[40];		# (CDR 41)
		print " gw-rxd-cdn=", $cdr[9];			# (CDR 10)
		print " gk-xlated-cdn=", $cdr[30];		# (CDR 31)
		print " gw-final-xlated-cdn=", $cdr[8];	# (CDR 9)
		print " outgoing-area=";
		print $cdr[27] if ($leg eq '2');	# (CDR 28 if leg 2 else blank)

		# cisco-h323-conf-id
		$x = $cdr[23];
		print ",h323-conf-id=", $x;			# (CDR 24)

		# cisco-h323-call-origin
		print ",h323-call-origin=";
		if ($leg eq '2') { print "originate"; } else { print "answer"; }

		# cisco-h323-call-type
		print ",h323-call-type=VoIP";

		# cisco-h323-remote-address
		print ",h323-remote-address=";
		if ($leg eq '1') { print $cdr[3]; } else { print $cdr[5]; }	# Leg1: CDR 4, Leg2: CDR 6

		# cisco-h323-setup-time		# (CDR 2, formatted as below)
		printf ",h323-setup-time=%s", strftime("%T.000", localtime($cdr[1]));
		print strftime(" %Z %a %b %d %Y", localtime($cdr[1]));

		# cisco-h323-connect-time		# (CDR 2 + seconds(CDR 25) )
		if ($cdr[35] > 0) {
			($h,$m,$s) = split (':', $cdr[24]);
		}else {
			$s = 0;
		}
		my $conntime = $s + $m*60 + $h*3600 + $cdr[1];
		printf ",h323-connect-time=%s", strftime("%T.000", localtime($conntime));
		print strftime(" %Z %a %b %d %Y", localtime($conntime));

		# cisco-h323-disconnect-time	# (CDR 2 + CDR 47, with part of CDR 47 after '.')
		$s = int($cdr[46]);
		$ms = int(($cdr[46]-$s)*1000);
		$s += $conntime;
		printf ",h323-disconnect-time=%s%03d", strftime("%T.", localtime($s)), $ms;
		print strftime(" %Z %a %b %d %Y", localtime($s));

		# cisco-h323-disconnect-cause	# (CDR 30)
		printf ",h323-disconnect-cause=%x", $cdr[29];

		# cisco-h323-voice-quality  (Constant: blank)
		print ",";

		# cisco-h323-gw-id		# (Leg 1: CDR 26, Leg 2: CDR 28)
		print ",h323-gw-id=";
		if ($leg eq '1') { print $cdr[25]; } else { print $cdr[27]; }

		print "\n";
	}
}

exit;

