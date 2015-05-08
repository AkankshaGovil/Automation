#!/usr/bin/perl


############################################################################
############################################################################
###
###	Restart.pl
###
###	Restarts certain processes.
###
###	Copyright (C) 2002, NexTone Communications.
############################################################################
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



##
## The right sequence of actions.
##
chdir "/usr/local/nextone/bin";

system ("./iserver all stop");

sleep 30;

system ("./iserver all start");


