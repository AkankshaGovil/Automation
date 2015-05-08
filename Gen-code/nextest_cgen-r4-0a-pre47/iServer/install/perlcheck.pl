#!/usr/bin/perl

#############################################################################
#############################################################################
##
## perlcheck.pl
## 
## Check for the correct version of Perl and update files
##
## Copyright 2002, NexTone Communications, Inc.
#############################################################################
#############################################################################

## Check for version of perl

use English;

$CurrentPerl = "/usr/bin/perl";
$LocalBinPerl = "/usr/local/bin/perl";
$NexToneEtc = "/usr/local/nextone/etc";
@FileList = ( cdrpp, dbback, dbop, logpp, cdrmon, 'mail.pl', 'notify.pl', 'phone.pl', 'restart.pl' );

$CurrentVersion = sprintf "%vd", $PERL_VERSION;

if ( -x $LocalBinPerl )
{
	$LocalBinVersion = `$LocalBinPerl -e 'printf \"%vd\n\", \$^V;' `;

	chop $LocalBinVersion;
}
print "Version of Perl in /usr/local/bin is $LocalBinVersion \n";
print "Version of Perl in /usr/bin is $CurrentVersion \n";

if ($CurrentVersion lt $LocalBinVersion)
{
	print "Selecting $LocalBinVersion (/usr/local/bin) \n";

	print "Modifying files...\n";

	chdir $NexToneEtc;

	foreach $f (@FileList)
	{
		next if ( ! -f $f );
		print "Processing $f...\n";
		open (F, "+< $f") or die "Cannot open $f : $! ";
		@ARR = <F>;

		$oldstr = $ARR[0];
		substr ($ARR[0], 2, 4) = "/usr/local";

		seek (F, 0, 0);
		print F @ARR;
		truncate (F, tell(F));
		close (F);
	}

}
else
{
	print "Versions good. Leaving as is.\n";
}
