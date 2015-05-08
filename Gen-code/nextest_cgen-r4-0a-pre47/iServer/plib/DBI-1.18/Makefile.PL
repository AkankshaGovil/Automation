# -*- perl -*-
#
# $Id: Makefile.PL,v 1.1.2.1 2002/12/18 19:35:09 santosh Exp $
#
#  Copyright (c) 1994,1995,1996,1997  Tim Bunce  England
#
# See COPYRIGHT section in DBI.pm for usage and distribution rights.

BEGIN {
    require 5.004;
    unshift @INC, "lib";
}

use ExtUtils::MakeMaker 5.16, qw(WriteMakefile $Verbose);
use Getopt::Long;
use Config;
use File::Find;
use strict;

use Test::Harness;	# tests use it so check it's loadable early


my $os = $^O;
my $osvers = $Config{osvers};
$osvers =~ s/^\s*(\d+\.\d+).*/$1/;  # drop sub-sub-version: 2.5.1 -> 2.5
my $ext_pl = $^O eq 'VMS' ? '.pl' : '';

$::opt_v = 0;
$::opt_g = 0;
$::opt_thread = 1;	# unset later if not a threaded perl

GetOptions(qw(v! g! thread!))
	or die "Invalid arguments\n";

$::opt_g &&= '-g';  # convert to actual string

if ($Config{archname} =~ /-thread\b/i) {
    print "\n";
    print "*** You are using a perl with experimental threading enabled!\a\n";
    print "*** You should be aware that using multiple threads is unstable\n";
    print "*** are should NOT be done in production environments.\n";
    print "DBI thread mutex protection is ", $::opt_thread ? "enabled\n" : "disabled!\n";
    print "\n";
    sleep 5;
}
else {
    $::opt_thread = 0;
}


my @missing;

eval "use RPC::PlServer 0.2001 ();";
if ($@) {
    push @missing, 'RPC::PlServer';
    print "\a",<<'MSG';
*** Note:
    The optional PlRPC-modules (RPC::PlServer etc) are not installed.
    If you want to use the DBD::Proxy driver and DBI::ProxyServer
    modules, then you'll need to install the RPC::PlServer, RPC::PlClient,
    Storable and Net::Daemon modules. The CPAN Bundle::DBI may help you.
    You can install them any time after installing the DBI.
    You do *not* need these modules for typical DBI usage.

MSG
    sleep 2;
}

if (@missing) {
    print <<'MSG';
Optional modules are available from any CPAN mirror, in particular
    http://www.perl.com/CPAN/modules/by-module
    http://www.perl.org/CPAN/modules/by-module
    ftp://ftp.funet.fi/pub/languages/perl/CPAN/modules/by-module

MSG
	sleep 5;
}

 
my %opts = (
    NAME=> 'DBI',
    VERSION_FROM=> 'DBI.pm',
    EXE_FILES => [ "dbish$ext_pl", "dbiproxy$ext_pl" ],
    DIR => [],
    dynamic_lib => { OTHERLDFLAGS => "$::opt_g" },
    clean => { FILES=> "\$(DISTVNAME) Perl.xsi "
			."dbish$ext_pl dbiproxy$ext_pl ndtest.prt" },
    dist  => {
	DIST_DEFAULT=> 'clean distcheck disttest ci tardist',
	PREOP => '$(MAKE) -f Makefile.old distdir',
	COMPRESS => 'gzip -v9', SUFFIX => 'gz',
    },
);

if ($ExtUtils::MakeMaker::VERSION >= 5.43) {
    $opts{ABSTRACT_FROM} = 'DBI.pm';
    $opts{AUTHOR} = 'Tim Bunce (dbi-users@perl.org)';
    $opts{CAPI} = 'TRUE' if $Config{archname} =~ /-object\b/i;
}

$opts{DEFINE} = '-Wall -Wno-comment'
    if $Config{cc} eq 'gcc';	# ask gcc to be mildly pedantic

$opts{DEFINE} .= ' -DDBI_NO_THREADS' unless $::opt_thread;

# HP-UX 9 cannot link a non-PIC object file into a shared library.
# Since the # .a libs that Oracle supplies contain non-PIC object
# files, we sadly have to build static on HP-UX 9 :(
if ($os eq 'hpux' and $osvers < 10) {
    $opts{LINKTYPE} = 'static';
    print "Warning: Forced to build static not dynamic on $os $osvers.\a\n";
    print "** Note: DBI will be built *into* a NEW perl binary. You MUST use that new perl.\n";
    print "         See README and Makefile.PL for more information.\a\n";
}

if ($os eq 'MSWin32'  &&  $Config{libs} =~ /\bPerlCRT.lib\b/
    &&  -f "$Config{archlib}/CORE/PerlCRT.lib") {
    # ActiveState Perl needs this; should better be done in MakeMaker, but
    # as a temporary workaround it seems ok.
    $opts{LIBS} = "-L$Config{archlib}/CORE";
}

# Set some private WriteMakefile options if this is 'me' :-)
if ($ENV{S_ARCH_SW} && $ENV{LOGNAME} eq 'timbo'){	# a reasonable guess
    $opts{DEFINE} .= ' -g -Wpointer-arith -Wconversion';
}

warn "WARNING: Your GNU C compiler is very old. Please upgrade.\n"
	if ($Config{gccversion} and $Config{gccversion} =~ m/^(1|2\.[1-5])/);

# Set aside some values for post_initialize() in package MY
my ( $cfg_privlibexp, $cfg_archlibexp, $cfg_sitelibexp, $cfg_sitearchexp,
     $cfg_man3direxp ) =
     @Config{qw( privlibexp archlibexp sitelibexp sitearchexp man3direxp ) };

$Verbose = $::opt_v;
WriteMakefile( %opts );

warn <<EOT;

    Remember to actually *read* the README file!
    Use  'make' to build the software (dmake or nmake on Windows).
    Then 'make test' to execute self tests.
    Then 'make install' to install the DBI and then delete this working
    directory before unpacking and building any DBD::* drivers.

EOT
warn <<EOT if $os eq 'MSWin32';
    Windows users need to use the correct make command.
    That may be nmake or dmake depending on which Perl you are using.
    If using the Win32 ActiveState build then it is recommended that you
    use the ppm utility to fetch and install a prebuilt DBI instead.
EOT


# =====================================================================

package MY;

sub libscan {
    my($self, $path) = @_;
    ($path =~ /\~$/) ? undef : $path;
}

sub const_cccmd {
    my $self = shift;
    local($_) = $self->SUPER::const_cccmd(@_);
    # If perl Makefile.PL *-g* then switch on debugging
    if ($::opt_g) {
	s/\s-O\d?\b//;      # delete optimise option
	s/\s-/ -g -/;       # add -g option
    }
    $_;
}


sub post_initialize {
    my($self) = shift;

    if ($cfg_privlibexp ne $cfg_sitelibexp) {
	# this block could probably be removed now
	my %old;
	File::Find::find( sub {
	    local $_ = $File::Find::name;
	    s:\\:/:g if $os eq 'MSWin32';
	    $File::Find::prune = 1, return
		if -d $_ && ( $_ eq $cfg_sitelibexp ||
			      $_ eq $cfg_sitearchexp ||
			      $_ eq $cfg_man3direxp );
	    ++$old{$_} if m:\bDB(I|D$):; # DBI files, but just DBD dirs
	}, $cfg_privlibexp, $cfg_archlibexp );
	if ( %old ) {
	warn "
Warning: By default new modules are installed into your 'site_lib'
         directories. Since site_lib directories come after the normal library
         directories you must delete old DBI files and directories from your
         'privlib' and 'archlib' directories and their auto subdirectories.

Reinstall DBI and your DBD::* drivers after deleting the old directories.

Here's a list of probable old files and directories:

 " . join( "\n ", ( sort keys %old ), "\n" );
	}
    }

    # install files that DBD's may need
    File::Find::find( sub {
	$File::Find::prune = 1, return if -d $_ && '.' ne $_;
	$self->{PM}->{$_} = '$(INST_ARCHAUTODIR)/' . $_
	    if '.h' eq substr( $_, -2 ) || '.xst' eq substr( $_, -4 );
        }, '.' );

    return '';
}


sub post_constants {

    # ensure that Driver.xst and related code gets tested

    BEGIN {
	require 'lib/DBI/DBD.pm';
	import DBI::DBD;
    }
    my $xst = dbd_postamble();
    $xst =~ s/\$\(BASEEXT\)/Perl/g;
    $xst .= '
DBI.c: Perl$(OBJ_EXT)
';

    return $xst;
}

# end.
