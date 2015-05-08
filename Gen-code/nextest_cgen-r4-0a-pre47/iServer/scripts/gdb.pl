#!/usr/local/bin/perl

use Devel::GDB;
use Socket;
use Getopt::Std;
use Time::Timezone;
use Time::Local;
use POSIX qw(strftime);

$month = { 
			'jan' => 0, 
			'feb' => 1, 
			'mar' => 2, 
			'apr' => 3, 
			'may' => 4, 
			'jun' => 5, 
			'jul' => 6, 
			'aug' => 7, 
			'sep' => 8, 
			'oct' => 9, 
			'nov' => 10, 
			'dec' => 11 
		};

$NULL_AVL_NP = '(struct avl_node *) 0x0';

$cache_conf = 	{
					'regCache' => 	{
										'tree' => 'avl_tree',		
										'entry' => 'CacheTableInfo'
									},	
					'callCache' =>  {
										'tree' => 'avl_tree',
										'entry' => 'CallHandle'
									}
				};

$callErrorStr = {
    # case SCC_errorAbandoned
        '2' => "abandoned",
    # case SCC_errorBlockedUser:
        '7' => "user-blocked",
    # case SCC_errorNoRoute:
        '13' => "no-route",
    # case SCC_errorBusy:
        '1' => "busy",
    # case SCC_errorResourceUnavailable:
        '1001' => "resource-unavailable",
    # case SCC_errorInvalidPhone:
        '3' => "invalid-phone",
    # case SCC_errorNetwork:
        '12' => "network-error",
    # case SCC_errorNoPorts:
        '14' => "no-ports",
    # case SCC_errorGeneral:
        '15' => "general-error",
    # case SCC_errorDestinationUnreachable:
        '1002' => "dest-unreach",
    # case SCC_errorUndefinedReason:
        '1003' => "undefined",
    # case SCC_errorInadequateBandwidth:
        '1004' => "no-bandwidth",
    # case SCC_errorH245Incomplete:
        '1005' => "h245-error",
    # case SCC_errorIncompleteAddress:
        '1006' => "incomp-addr",
    # case SCC_errorLocalDisconnect:
        '1007' => "local-disconnect",
    # case SCC_errorH323Internal:
        '1008' => "h323-internal",
    # case SCC_errorH323Protocol:
        '1009' => "h323-proto",
    # case SCC_errorNoCallHandle:
        '1010' => "no-call-handle",
    # case SCC_errorGatewayResourceUnavailable:
        '1011' => "gw-resource-unavailable",
    # case SCC_errorFCECallSetup:
        '1012' => "fce-error-setup",
    # case SCC_errorFCE:
        '1013' => "fce-error",
    # case SCC_errorNoVports:
        '1014' => "no-vports",
    # case SCC_errorHairPin:
        '1015' => "hairpin",
    # case SCC_errorShutdown:
        '1016' => "shutdown",
    # case SCC_errorDisconnectUnreachable:
        '1017' => "disconnnect-unreach",
    # case SCC_errorTemporarilyUnavailable:
        '1018' => "temporarily-unavailable",
    # case SCC_errorSwitchover:
        '1019' => "switchover",
    # case SCC_errorDestRelComp:
        '1020' => "dest-relcomp",
    # case SCC_errorCrash:
        '1021' => "MSW crashed",
    # case SCC_errorSipRedirect:
        '5300' => "sip-redirect",
    # case SCC_errorSipAuthRequired:
        '5401' => "401 authorization required",
    # case SCC_errorSipForbidden:
        '5403' => "403 forbidden",
    # case SCC_errorSipProxyAuthRequired:
        '5407' => "407 proxy authorization required",
    # case SCC_errorSipInternalError:
        '5500' => "500 internal error",
    # case SCC_errorSipNotImplemented:
        '5501' => "501 not implemented",
    # case SCC_errorSipServiceUnavailable:
        '5503' => "503 service unavailable",
};

$callEventStr => {
    # case SCC_evtNone:
        '0' => "no-state",
    # case SCC_evtInvalid:
        '1' => "invalid",
    # case SCC_evtUnknown:
        '2' => "unknown",
    # case SCC_evtHunt:
        '3' => "hunt",
    # case SCC_evtResolve:
        '4' => "resolve",
    # case SCC_evt323SetupTx:
        '5' => "setup-tx",
    # case SCC_evt323SetupRx:
        '6' => "setup-rx",
    # case SCC_evt323ProcTx:
        '7' => "proc-tx",
    # case SCC_evt323ProcRx:
        '8' => "proc-rx",
    # case SCC_evt323AlertTx:
        '9' => "alert-tx",
    # case SCC_evt323AlertRx:
        '10' => "alert-rx",
    # case SCC_evt323ConnTx:
        '11' => "conn-tx",
    # case SCC_evt323ConnRx:
        '12' => "conn-rx",
    # case SCC_evt323ARQRx:
        '13' => "arq-rx",
    # case SCC_evt323ACFTx:
        '14' => "acf-tx",
    # case SCC_evt323ARQTx:
        '15' => "arq-tx",
    # case SCC_evt323ACFRx:
        '16' => "acf-rx",
    # case SCC_evt323ARJRx:
        '17' => "arj-rx",
    # case SCC_evt323ARQTmout:
        '18' => "arq-tmout",
    # case SCC_evt323ARJTx:
        '19' => "arj-tx",
    # case SCC_evt323DestARQRx:
        '20' => "destarq-rx",
    # case SCC_evt323DestACFTx:
        '21' => "destacf-tx",
    # case SCC_evt323NewCall:
        '22' => "newcall",
    # case SCC_evt323LRQTx:
        '23' => "lrq-tx",
    # case SCC_evt323LCFRx:
        '24' => "lcf-rx",
    # case SCC_evt323LRJRx:
        '25' => "lrj-rx",
    # case SCC_evt323LRQTmout:
        '26' => "lrq-tmout",
    # case SCC_evtSipInviteTx:
        '27' => "inv-tx",
    # case SCC_evtSipInviteRx:
        '28' => "inv-rx",
    # case SCC_evtSipReInviteTx:
        '29' => "reinv-tx",
    # case SCC_evtSipReInviteRx:
        '30' => "reinv-rx",
    # case SCC_evtSip100xTx:
        '31' => "100x-tx",
    # case SCC_evtSip100xRx:
        '32' => "100x-rx",
    # case SCC_evtSip200Tx:
        '33' => "200-tx",
    # case SCC_evtSip200Rx:
        '34' => "200-rx",
    # case SCC_evtSipAckTx:
        '35' => "ack-tx",
    # case SCC_evtSipAckRx:
        '36' => "ack-rx",
    # case SCC_evtSip3xxRx:
        '37' => "3xx-rx",
    # case SCC_evtSipInviteRedirected:
        '38' => "inv-red",
    # case SCC_evt323ProgTx:
        '39' => "prog-tx",
    # case SCC_evt323ProgRx:
        '40' => "prog-rx",
    # case SCC_evt323DRQForceDropRx:
        '41' => "drq-forcedrop-rx"
};

$cdrfl = {
	'callsetup' => 0,
	'callattempt' => 0,
	'callinp' => 0,
	'calldropped' => 1,
	'originator' => 0,
	'callhunt' => 0
};

$H_t = [ 'SCC_eH323CallHandle' , 'SCC_eSipCallHandle' ];

sub getnv($) {
	my ($np) = @_;
	my $nodestr;

	chomp($nodestr = $gdb->get('p *'.$np));
	return $nodestr;
}

sub getnp($) {
	my ($np) = @_;
	my $nodestr;

	($jnk, $nodestr) = split('=', $gdb->get('p '.$np));
	chomp($nodestr);
	return $nodestr;
}

sub getIPAddr ($) {
	my ($expr) = @_;

	return inet_ntoa(pack('N', getval($expr . '.l')));
}

#sub getIPAddr($) {
#	my ($expr) = @_;
#	my ($val, $ip);
#
#	for $i (0..3) {
#		($jnk, $val) = split('=', $gdb->get('p (int)(' . $expr
#				. '.uc[' . (3-$i) . '])'));
#
#		#remove leading and trailing whitespace including newline
#		$val =~ s/^\s+//;   
#		$val =~ s/\s+$//;   
#		$ip .= "$val.";
#	}
#	
#	#remove the trailing .
#	chop($ip);
#
#	return $ip;
#}

sub getval($) {
	my ($expr) = @_;
	my $val;

	($jnk, $val) = split('=', $gdb->get('p '.$expr));

	#remove leading and trailing whitespace including newline
	$val =~ s/^\s+//;   
	$val =~ s/\s+$//;   
	return $val;
}

sub getarr($) {
	my ($expr) = @_;

	return getstr('((char *)('. $expr . '))');
}

#sub getarr($) {
#	my ($expr) = @_;
#	my $str = getval($expr);
#
#	($str) = split(',' , $str);
#	$str =~ s/^\"//;
#	$str =~ s/\"$//;
#	return $str;
#}

sub getstr($) {
	my ($expr) = @_;
	my (@tmp); 
	my $str = getval($expr);

	if ($str =~ /0x0/ ) { 	## Null Pointer 
		return "";
	}

	@tmp = split('\s+' , $str);
	$str = pop(@tmp);
	$str =~ s/^\"//;
	$str =~ s/\"$//;
	return $str;
}

sub field($$) {
	my ($ds, $fld) = @_;
	
	return '((' . $ds . ')->' . $fld . ')';
}

sub ID2str($$) {
	my ($expr, $bytes) = @_;
	my $val;
	my ($id, $idstr, $i);

	($jnk, $val) = split('=', $gdb->get('p /u *(char (*)['.
		$bytes . '])&' .$expr));

	#remove leading and trailing whitespace including newline
	$val =~ s/^\s+//;   
	$val =~ s/\s+$//;   
	$val =~ s/^./[/;    # make sure the bracket is [
	$val =~ s/.$/]/;	# make sure the bracket is ]
	eval("\$id = $val");

	for $i (0..$#$id) { 
		$idstr .= sprintf("%2.2x", $id->[$i]); 
	}

	return $idstr;
}

sub isnullp($) {
	my ($np) = @_;

	return $np =~ /\s+0x0[\s\)]*$/;
}

sub leftp($) {
	my ($np) = @_;
	
	return getnp('('.$np.')->link[0]');
}

sub rightp($) {
	my ($np) = @_;
	
	return getnp('('.$np.')->link[1]');
}

sub datap($) {
	my ($np) = @_;

	return getnp('('.$np.')->data');
}

sub avl_traverse($$) {
	my ($treep, $trav) = @_;

	if ($trav->{'init'} == 0) {
		$trav->{'init'} = 1;
		$trav->{'nstack'} = 0;
		$trav->{'p'} = leftp('('.$treep.')->root');	
	}
	else {
		$trav->{'p'} = rightp($trav->{'p'});
	}

	for (;;) {
		while (!(isnullp($trav->{'p'}))) {
			$trav->{'stack'}[$trav->{'nstack'}++] = $trav->{'p'};
			$trav->{'p'} = leftp($trav->{'p'});
		}

		if ($trav->{'nstack'} == 0) {
			$trav->{'init'} = 0;
			return $NULL_AVL_NP;
		}

		$trav->{'p'} = $trav->{'stack'}[--$trav->{'nstack'}];

		return datap($trav->{'p'});
	}	
}

sub TimeRound($) {
	my $x=$_;

	return 0 if (abs($x) < 500000);

# Sign or "Signum" function: return -1, 0, 1
	return ($x/abs($x));
}

sub PrintCache($$$) {
	my ($cache, $datap, $printfunc) = @_;
	my $treep;
	my $trav = {};
	my $i;

	unless ($cache_conf->{$cache}) {
		print "$cache is not configured for tracing \n";
		return -1;
	}

	$treep = getnp('(' . $cache_conf->{$cache}{'tree'} . ' *)' . 
					$cache . '->root');

	die "\nCall Cache corrupt, aborting" if ($treep eq undef);

	for ($i=0, $datap->[$i] = avl_traverse($treep, $trav);
		 (!(isnullp($datap->[$i]))); 
		 $datap->[++$i] = avl_traverse($treep, $trav)) { 
		 	$datap->[$i] = '((' . $cache_conf->{$cache}{'entry'} . ' *)('
						. $datap->[$i] . '))';
		&$printfunc($datap->[$i]);
	}
}

sub DumpCallStruct($) {
	my $ch = shift;

	if (!(defined $callcnt)) {
		$callcnt = 0;
	}
	$callcnt++;
	print "$callcnt th call is - \n";
	print getnv($ch)." \n\n";
}

sub PrintCSV($) {
	my ($ch) = @_;

	my $newError = getval(field($ch, 'callError'));
	my $newError2 = getval(field($ch, 
		'callDetails2.callError'));

	my $lastEvent = getval(field($ch, 'lastEvent'));
	my $lastEvent2 = getval(field($ch, 
		'callDetails2.lastEvent'));

	my $callSource = getval(field($ch, 'callSource'));

	if ($callSource) {
		$remote_phone = getarr(field($ch, 'phonode.phone'));
		$remote_regid = getarr(field($ch, 'phonode.regid'));
		$remote_uport = getval(field($ch, 'phonode.uport'));
		$remote_ipaddress = getIPAddr(field($ch,
			'phonode.ipaddress'));

		$local_phone = getarr(field($ch, 'rfphonode.phone'));
		$local_regid = getarr(field($ch, 'rfphonode.regid'));
		$local_uport = getval(field($ch, 'rfphonode.uport'));
		$local_ipaddress = getIPAddr(field($ch, 
			'rfphonode.ipaddress'));
	}
	else {
		$local_phone = getarr(field($ch, 'phonode.phone'));
		$local_regid = getarr(field($ch, 'phonode.regid'));
		$local_uport = getval(field($ch, 'phonode.uport'));
		$local_ipaddress = getIPAddr(field($ch, 
			'phonode.ipaddress'));

		$remote_phone = getarr(field($ch, 'rfphonode.phone'));
		$remote_regid = getarr(field($ch, 'rfphonode.regid'));
		$remote_uport = getval(field($ch, 'rfphonode.uport'));
		$remote_ipaddress = getIPAddr(field($ch, 
			'rfphonode.ipaddress'));
	}

	my $callStartTime = getval(field($ch, 'callStartTime.tv_sec'));
	my $callStartTimeus = getval(field($ch, 'callStartTime.tv_usec'));
	my $callEndTime = getval(field($ch, 'callEndTime.tv_sec'));
	my $callEndTimeus = getval(field($ch, 'callEndTime.tv_usec'));
	my $callConnectTime = getval(field($ch, 'callConnectTime.tv_sec'));
	my $callConnectTimeus = getval(field($ch, 'callConnectTime.tv_usec'));

	my $peerIp = getval(field($ch, 'peerIp'));
	my $peerPort = getval(field($ch, 'peerPort'));
	my $custID = getstr(field($ch, 'custID'));
	my $inputNumber = getarr(field($ch, 'inputNumber'));
	my $flags = getval(field($ch, 'flags'));
	my $inputANI = getarr(field($ch, 'inputANI'));
	my $cause = getval(field($ch, 'cause'));
	my $dialledNumber = getarr(field($ch, 'dialledNumber'));
	my $incoming_conf_id = getval(field($ch, 'incoming_conf_id'));
	($jnk, $incoming_conf_id, $jnk2) = split ('"', $incoming_conf_id);

	my $handleType = getval(field($ch, 'handleType'));
	my $nhunts = getval(field($ch, 'nhunts'));
	my $conf_id = getval(field($ch, 'conf_id'));
	($jnk, $conf_id, $jnk2) = split ('"', $conf_id);

	my $callID = field($ch, 'callID');
	my $acctSessionId = field($ch, 'acct_session_id');
	my $remoteMediaIpaddr = field($ch, 'lastMediaIp');
	my $tg = field($ch, 'tg');

	## Do all the checks whether this call should be put in CSV
	return if (!($callStartTime) or ($callSource and !$opt_l));

	my $now=time;

	# Ending Date and time of call on local gw

	unless ($callEndTime) {
		$callEndTime = $crashTime;
		$callEndTimeus = 0;
	}

	# Current date and time to start
	# Date, Time (Current Date, not related to call)
	printf "%s", strftime("%m/%d/%Y,%T", localtime($now));

	# Called-Station-Id
	printf ",%s", $inputNumber;

	# Calling-Station-Id
	printf ",%s", $local_phone;

	# Acct-Status-Type
	print ",Stop";

	# Acct-Session-Id
	print ",", getarr ($acctSessionId);

	# Acct-Session-Time (call duration, seconds)
	if ($dur = $callConnectTime) {
		$dur = $callEndTime - $dur;
		$durms = $callEndTimeus - $callConnectTimeus;
		$dur += TimeRound($durms);
		$durms = $dur*1000 + ($durms+500)/1000; 	# might lose 1ms here
		$dur = 0 if ($dur < 0); 
	}
	print ",", $dur;

	# Service-Type (Constant: "Login")
	print ",Login";

	# NAS-IP-Address
	print ",", $nas_ipaddr;

	# NAS-Port (Constant: 0)
	print ",0";

	# NAS-Identifier
	print ",", $sipservername;

	# Proxy-State, Connect-Info, cisco-nas-port (Constants: blank)
	print ",,,";

	# cisco-av-pair  This is a space separated field, no commas for a while.
	print ",h323-incoming-conf-id=", $incoming_conf_id;
	print " subscriber=Subscriber";
	print (" fax-tx-duration=", $durms) if ($flags & 0x04);
	print " session-protocol=";
	if ($handleType eq $H_t->[0]) { #H323
		printf "H323" ;
	}
	elsif ($handleType eq  $H_t->[1]) { #Sip
		printf "SIP" ;
	}
	else {
		printf "unknown" ;
	}
	print " remote-media-address=", inet_ntoa(pack('N', getval($remoteMediaIpaddr))); # May need 'V' not 'N'
	print " in-trunkgroup-label=", getstr($tg);
	print " in-carrier-id=", getstr($tg);
	print " gw-rxd-cdn=", $inputNumber;
	print " gk-xlated-cdn=", $dialledNumber;
	print " gw-final-xlated-cdn=", $remote_phone;
	print " outgoing-area=";
	print $remote_regid unless $callSource;

	# cisco-h323-conf-id
	print ",h323-conf-id=", $conf_id;

	# cisco-h323-call-origin
	print ",h323-call-origin=", $callSource ? "originate" : "answer";

	# cisco-h323-call-type
	print ",h323-call-type=VoIP";

	# cisco-h323-remote-address
	print ",h323-remote-address=", inet_ntoa(pack('N', $peerIp));

	# cisco-h323-setup-time
	printf ",h323-setup-time=%s%03d", strftime("%T.", localtime($callStartTime)), int(($callStartTimeus+500)/1000);
	print strftime(" %Z %a %b %d %Y", localtime($callStartTime));

	# cisco-h323-connect-time
	printf ",h323-connect-time=%s%03d", strftime("%T.", localtime($callConnectTime)),int(($callConnectTimeus+500)/1000);
	print strftime(" %Z %a %b %d %Y", localtime($callConnectTime));

	# cisco-h323-disconnect-time
	printf ",h323-disconnect-time=%s%03d", strftime("%T.", localtime($callEndTime)), int(($callEndTimeus+500)/1000);
	print strftime(" %Z %a %b %d %Y", localtime($callEndTime));

	# cisco-h323-disconnect-cause
	printf ",h323-disconnect-cause=%x", 41;

	# cisco-h323-voice-quality  (Constant: blank)
	print ",";

	# cisco-h323-gw-id
	print ",h323-gw-id=", $callSource ? $remote_regid : $local_regid;

	print "\n";

}

sub PrintMindCDR($) {
	my ($ch) = @_;

	my $newError = getval(field($ch, 'callError'));
	my $newError2 = getval(field($ch, 
		'callDetails2.callError'));

	my $lastEvent = getval(field($ch, 'lastEvent'));
	my $lastEvent2 = getval(field($ch, 
		'callDetails2.lastEvent'));

	my $callSource = getval(field($ch, 'callSource'));

	if ($callSource) {
		$remote_phone = getarr(field($ch, 'phonode.phone'));
		$remote_regid = getarr(field($ch, 'phonode.regid'));
		$remote_uport = getval(field($ch, 'phonode.uport'));
		$remote_ipaddress = getIPAddr(field($ch,
			'phonode.ipaddress'));

		$local_phone = getarr(field($ch, 'rfphonode.phone'));
		$local_regid = getarr(field($ch, 'rfphonode.regid'));
		$local_uport = getval(field($ch, 'rfphonode.uport'));
		$local_ipaddress = getIPAddr(field($ch, 
			'rfphonode.ipaddress'));
	}
	else {
		$local_phone = getarr(field($ch, 'phonode.phone'));
		$local_regid = getarr(field($ch, 'phonode.regid'));
		$local_uport = getval(field($ch, 'phonode.uport'));
		$local_ipaddress = getIPAddr(field($ch, 
			'phonode.ipaddress'));

		$remote_phone = getarr(field($ch, 'rfphonode.phone'));
		$remote_regid = getarr(field($ch, 'rfphonode.regid'));
		$remote_uport = getval(field($ch, 'rfphonode.uport'));
		$remote_ipaddress = getIPAddr(field($ch, 
			'rfphonode.ipaddress'));
	}

	my $callStartTime = getval(field($ch, 'callStartTime.tv_sec'));
	my $callStartTimeus = getval(field($ch, 'callStartTime.tv_usec'));
	my $callEndTime = getval(field($ch, 'callEndTime.tv_sec'));
	my $callEndTimeus = getval(field($ch, 'callEndTime.tv_usec'));
	my $callConnectTime = getval(field($ch, 'callConnectTime.tv_sec'));
	my $callConnectTimeus = getval(field($ch, 'callConnectTime.tv_usec'));
	my $peerPort = getval(field($ch, 'peerPort'));
	my $custID = getstr(field($ch, 'custID'));
	my $inputNumber = getarr(field($ch, 'inputNumber'));
	my $flags = getval(field($ch, 'flags'));
	my $inputANI = getarr(field($ch, 'inputANI'));
	my $cause = getval(field($ch, 'cause'));
	my $dialledNumber = getarr(field($ch, 'dialledNumber'));
	my $incoming_conf_id = getstr(field($ch, 'incoming_conf_id'));
	my $handleType = getval(field($ch, 'handleType'));
	my $nhunts = getval(field($ch, 'nhunts'));
	my $conf_id = getstr(field($ch, 'conf_id'));

	my $callID = field($ch, 'callID');
	my $acctSessionId = field($ch, 'acct_session_id');

	## Do all the checks whether this call should be put in CDR
	return if (!($callStartTime) or ($callSource and !$opt_l));

	# 1. Starting Date and time of call on local gw

	$callEndTime = $crashTime unless ($callEndTime);

	# we want to print call start time in "THEIR" context
	printf "%s", strftime("%Y-%m-%d %T", localtime($callStartTime + $offset));

	# 2. (1.) in int format
	printf ";%d", $callStartTime;

	# 3. Duration of call
	if ($dur = $callConnectTime) {
		$dur = $callEndTime - $dur;
		$dur = 0 if ($dur < 0); 
	}	
	$h = int($dur/3600);
	$m = int($dur/60) - $h*60;
	$secs = $dur -$h*3600 - $m*60;
	printf ";%3.3d:%2.2d:%2.2d", $h, $m, $secs;

	# 4. Originator IP 
	printf ";" ;
	unless ($callSource) {
		printf "%s", $local_ipaddress;				
	}

	# 5. Originator line
	printf ";%d", $peerPort;

	# 6. Terminator IP 
	printf ";%s", $remote_ipaddress;				

	# 7. Terminator Line
	printf ";" ;

	# 8. User Id of originator
	printf ";%s", $custID ? $custID : "";

	# 9. Called number in E.164 format 
	printf ";%s", $remote_phone;

	# 10. Called number as dialed by user
	printf ";%s", $callSource ? "" : $inputNumber;

	# 11. Call Type
	if ($flags & 0x4) {
		printf ";IF" ;
	}
	else {
		printf ";IV" ;
	}

	# 12. Call Parties
	printf ";01" ;

	# 13. Call Disconnect Reason -TBF
	printf ";E"; 

#
#	if ((!$cdrfl->{'callsetup'}) && 
#		($callConnectTime==0) && ($newError == 0)) {
#			$newError = 1021; 		#SCC_errorCrash
#	}
#
#	if ($newError == 2) {
#		printf ";A"; 
#	}
#	elsif ($newError == 1) {
#		printf ";B"; 
#	}
#	elsif ($newError == 0) {
#		printf ";N"; 
#	}
#	else {
#		printf ";E"; 
#	}

	# 14,15 Optional Fields 
	if ($newError) {
		printf ";%d;%s", $newError, 
			$callErrorStr->{$newError};
	}
	else {
		printf ";;" ; 
	}

	# 16,17 error descr, fax pages priority
	printf ";;" ;

	# 18. Orig ANI
	printf ";%s", $inputANI;

	# 19,20,21. DNIS num bytes sent
	printf ";;;" ;

	# 22. 
	if ((!$cdrfl->{'callsetup'}) && (!$callSource)) {
		printf ";%d", $cdrseqnum++;
	}
	else {
		printf ";" ;
	}

	# 23. 
	printf ";" ;

	# 24. 
	if ($conf_id) {
		printf ";%s", substr($conf_id,0, 48) ;
	}
	else {
		printf ";%s", ID2str($callID, 16) ;
	}

	# 25. holdtime 
	if ($callConnectTime) {
		$holdtime = $callConnectTime - $callStartTime;
	}
	else {
		$holdtime = $callEndTime - $callStartTime;
		if ($holdtime < 0) {
			$holdtime = 0;
		}
	}

	$h = int($holdtime/3600);
	$m = int($holdtime/60) - $h*60;
	$secs = $holdtime - $h*3600 - $m*60;
	printf ";%3.3d:%2.2d:%2.2d", $h, $m, $secs ;

	# 26,27. 
	if ($callSource) {
		printf ";;" ;
	}
	else {
		printf ";%s;%d", length($local_regid) ? 
			$local_regid : "", $local_uport;
	}

	# 28,29. 
	printf ";%s;%d", length($remote_regid) ? 
		$remote_regid : "", $remote_uport;

	# 30 ISDN cause code
	printf ";%d", $cause;

	# 31. Called number prior to dest selection
	printf ";%s", $dialledNumber;

	# 32,33. Leg 2 Error  - TBF
	if ($newError2) {
		printf ";%d;%s", $newError2, 
			$callErrorStr->{$newError2};
	}
	else {
		printf ";;" ;
	}

	# 34. 
	if ($callSource) {
		printf ";na#%s", $callEventStr->{$lastEvent};	
	}
	else {
		printf ";%s#%s", $callEventStr->{$lastEvent},
			$lastEvent2 ? $callEventStr->{$lastEvent2}
			: "na";	
	}

	# 35. new ANI 
	printf ";%s", $local_phone;

	# 36. duration in seconds
	printf ";%d", $dur;

	# 37. Call id 
	if ($callSource) {
		if ($incoming_conf_id) {
			printf ";%s", substr($incoming_conf_id, 0, 48);
		}
		else {
			printf ";%s", ID2str($callID, 16);
		}
	}
	else {
		printf ";" ;
	}

	# 38.
	if ($handleType eq $H_t->[0]) { #H323
		printf ";h323" ;
	}
	elsif ($handleType eq  $H_t->[1]) { #Sip
		printf ";sip" ;
	}
	else {
		printf ";unknown" ;
	}

	# 39. 
	if ($callSource) {
		printf ";%s", ($cdrfl->{'callsetup'}) ? 
			"start2" : "end2" ;
	}
	else {
		printf ";%s", ($cdrfl->{'callsetup'}) ? 
			"start1" : ($cdrfl->{'callhunt'}) ? 
			"hunt" : "end1" ;
	}

	# 40.
	printf ";" ;
	unless ( $callSource ) {
		printf "%d", $nhunts;
	}

	printf "\n";

}

sub gettime($) {
	my @t = split(' ', $_[0]) ; 
	my $time;

	die "Time format not correct - incorrect arguments.\n$help4" 
		if ($#t != 2); 

	die "Time format incorrect (date) - $t[0].\n$help4" 
		if (system("/usr/sadm/bin/valdate -f '%Y-%m-%d' '$t[0]'"));
	die "Time format incorrect (time) - $t[1].\n$help4" 
		if (system("/usr/sadm/bin/valtime -f '%H:%M:%S' '$t[1]'"));
	die "Time format incorrect (timezone)- $t[2].\n$help4" 
		if (($offset = tz_offset($t[2])) eq undef);

	# Calculate their offset from us i.e. (them -us)
	$offset -= tz_offset();

	($hour, $min, $sec) = split(':', $t[1]);
	($year, $mon, $day) = split('-', $t[0]);
	## timelocal takes a month range of 0-11
	$time = timelocal($sec, $min, $hour, $day, $mon-1, $year); 	
	return $time;	
}

getopts('c:g:ht:xrl');
($prog = $0) =~ s/^.*\///g;
$Usage = "Usage: $prog [-h] [-g gis_file] [-c core_file] [-t crash_time] [-x] [-l] [-r]\n";
$help1 = "  Use -l to include end records from leg2 in addition to leg1\n";
$help2 = "  Use -r to output in CSV format\n";
$help3 = "  Use -x to display times for core_file and exit\n";
$help3 .= "  crash_time is the localtime of the host on which msw cored.\n";
$help4 = "  date '+%Y-%m-%d %H:%M:%S %Z' represents the format expected .\n";
$help4 .= "  e.g. 2002-09-13 15:40:04 EST .\n";

$help_time = <<'EOF' ;
perl -e 'use POSIX; $| = 1;  print "filename> "; $f = <>; chomp($f); 
($at, $mt, $ct) = (POSIX::fstat(POSIX::open($f)))[8..10];
$fmt = "%Y-%m-%d %H:%M:%S %Z";
printf "access time       = %s\n", strftime($fmt, localtime($at));
printf "modification time = %s\n", strftime($fmt, localtime($mt));
printf "inode change time = %s\n", strftime($fmt, localtime($ct));' 
EOF

$Usage = $Usage.$help1.$help2.$help3.$help4;
die "$Usage" if $opt_h;

die "$help_time\n" if $opt_x; 

$gis_file = $opt_g ? $opt_g : '/export/home/test/gis';
$core_file = $opt_c ? $opt_c : '/export/home/test/core.gis';

$gdb = new Devel::GDB (-execfile => "gdb $gis_file $core_file");

$crashTime = getval ("timeNow");
$crashTime = $opt_t ? gettime($opt_t) : $crashTime;
($jnk, $TZ) = split ('=', $gdb->get('show environment TZ'));

my @callHandles;

if ($opt_r)
{
	local $sipservername;
	local $nas_ipaddr = inet_ntoa(pack('V', getval(iServerIP)));
	($jnk, $sipservername, $jnk2) = split ('"', $gdb->get('p (char *) sipservername'));
	# print "sipservername='", $sipservername, "'\n ipaddr='", $nas_ipaddr, "'\n";

	PrintCache('callCache', \@callHandles, \&PrintCSV);
}
else
{
	PrintCache('callCache', \@callHandles, \&PrintMindCDR);
}

#PrintCache('callCache', \@callHandles, \&DumpCallStruct);

