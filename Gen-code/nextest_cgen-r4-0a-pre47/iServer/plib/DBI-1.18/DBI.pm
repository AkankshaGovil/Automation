# $Id: DBI.pm,v 1.1.2.1 2002/12/18 19:35:09 santosh Exp $
#
# Copyright (c) 1994-2000  Tim Bunce  England
#
# See COPYRIGHT section in pod text below for usage and distribution rights.
#

require 5.004;

BEGIN {
$DBI::VERSION = "1.18"; # ==> ALSO update the version in the pod text below!
}

=head1 NAME

DBI - Database independent interface for Perl

=head1 SYNOPSIS

  use DBI;

  @driver_names = DBI->available_drivers;
  @data_sources = DBI->data_sources($driver_name, \%attr);

  $dbh = DBI->connect($data_source, $username, $auth, \%attr);

  $rv  = $dbh->do($statement);
  $rv  = $dbh->do($statement, \%attr);
  $rv  = $dbh->do($statement, \%attr, @bind_values);

  $ary_ref = $dbh->selectall_arrayref($statement);
  $ary_ref = $dbh->selectall_hashref($statement);

  $ary_ref = $dbh->selectcol_arrayref($statement);

  $ary_ref = $dbh->selectrow_arrayref($statement);
  @row_ary = $dbh->selectrow_array($statement);

  $sth = $dbh->prepare($statement);
  $sth = $dbh->prepare_cached($statement);

  $rv = $sth->bind_param($p_num, $bind_value);
  $rv = $sth->bind_param($p_num, $bind_value, $bind_type);
  $rv = $sth->bind_param($p_num, $bind_value, \%attr);

  $rv = $sth->execute;
  $rv = $sth->execute(@bind_values);

  $rc = $sth->bind_col($col_num, \$col_variable);
  $rc = $sth->bind_columns(@list_of_refs_to_vars_to_bind);

  @row_ary  = $sth->fetchrow_array;
  $ary_ref  = $sth->fetchrow_arrayref;
  $hash_ref = $sth->fetchrow_hashref;

  $ary_ref  = $sth->fetchall_arrayref;

  $rv  = $sth->rows;

  $rc  = $dbh->commit;
  $rc  = $dbh->rollback;

  $sql = $dbh->quote($string);

  $rc  = $h->err;
  $str = $h->errstr;
  $rv  = $h->state;

  $rc  = $dbh->disconnect;

I<This synopsis above only lists the major methods.>


=head2 GETTING HELP

If you have questions about DBI, you can get help from
the I<dbi-users@perl.org> mailing list.
You can subscribe to the list by emailing:

  dbi-users-help@perl.org

Also worth a visit is the DBI home page at:

  http://dbi.perl.org/

Before asking any questions, reread this document, consult the
archives and read the DBI FAQ. The archives are listed
at the end of this document.
The FAQ is installed as a DBI::FAQ module so
you can read it by executing C<perldoc DBI::FAQ>.

Please note that Tim Bunce does not maintain the mailing lists or the
web page (generous volunteers do that).  So please don't send mail
directly to him; he just doesn't have the time to answer questions
personally. The I<dbi-users> mailing list has lots of experienced
people who should be able to help you if you need it.

=head2 NOTE

This is the DBI specification that corresponds to the DBI version 1.17
(C<$Date: 2002/12/18 19:35:09 $>).

The DBI specification is evolving at a steady pace, so it's
important to check that you have the latest copy. The RECENT CHANGES
section below has a summary of user-visible changes. The F<Changes>
file supplied with the DBI holds more detailed change information.

Note also that whenever the DBI changes, the drivers take some time to
catch up. Recent versions of the DBI have added new features
(marked I<NEW> in the text) that may not yet be supported by the drivers
you use. Talk to the authors of those drivers if you need the features.

Extensions to the DBI and other DBI related modules use the C<DBIx::*>
namespace. See L</Naming Conventions and Name Space> and:

  http://www.perl.com/CPAN/modules/by-module/DBIx/

=head2 RECENT CHANGES 

Here is a brief summary of significant user-visible changes in recent versions.
(If a recent version isn't mentioned, it simply means that there were no
significant user-visible changes in that version.)

=over 4 

=item DBI 1.15

Added selectall_hashref, selectrow_hashref, selectrow_arrayref methods.

=back 

=cut

# The POD text continues at the end of the file.

# DBI file-private variables
my %installed_rootclass;


{
package DBI;

my $Revision = substr(q$Revision: 1.1.2.1 $, 10);

use Carp;
use DynaLoader ();
use Exporter ();

BEGIN {
@ISA = qw(Exporter DynaLoader);

# Make some utility functions available if asked for
@EXPORT    = ();		    # we export nothing by default
@EXPORT_OK = qw(%DBI %DBI_methods); # also populated by export_ok_tags:
%EXPORT_TAGS = (
   sql_types => [ qw(
	SQL_ALL_TYPES
	SQL_CHAR SQL_NUMERIC SQL_DECIMAL SQL_INTEGER SQL_SMALLINT
	SQL_FLOAT SQL_REAL SQL_DOUBLE SQL_VARCHAR
	SQL_DATE SQL_TIME SQL_TIMESTAMP
	SQL_LONGVARCHAR SQL_BINARY SQL_VARBINARY SQL_LONGVARBINARY
	SQL_BIGINT SQL_TINYINT
	SQL_WCHAR SQL_WVARCHAR SQL_WLONGVARCHAR
	SQL_BIT
   ) ],
   preparse_flags => [ qw(
        DBIpp_cm_cs DBIpp_cm_hs DBIpp_cm_dd DBIpp_cm_br
        DBIpp_ph_qm DBIpp_ph_cn DBIpp_ph_cs DBIpp_ph_sp
   ) ],
   utils     => [ qw(
	neat neat_list dump_results looks_like_number
   ) ],
);
Exporter::export_ok_tags('sql_types', 'utils');

$DBI::dbi_debug = $ENV{DBI_TRACE} || $ENV{PERL_DBI_DEBUG} || 0;

# If you get an error here like "Can't find loadable object ..."
# then you haven't installed the DBI correctly. Read the README
# then install it again.
bootstrap DBI;

}
*trace_msg = \&DBD::_::common::trace_msg;

use strict;

my $connect_via = "connect";

# check if user wants a persistent database connection ( Apache + mod_perl )
if ($INC{'Apache/DBI.pm'} && substr($ENV{GATEWAY_INTERFACE}||'',0,8) eq 'CGI-Perl') {
    $connect_via = "Apache::DBI::connect";
    DBI->trace_msg("DBI connect via $INC{'Apache/DBI.pm'}\n");
}


if ($DBI::dbi_debug) {
    @DBI::dbi_debug = ($DBI::dbi_debug);

    if ($DBI::dbi_debug !~ m/^\d$/) {
	# dbi_debug is a file name to write trace log to.
	# Default level is 2 but if file starts with "digits=" then the
	# digits (and equals) are stripped off and used as the level
	unshift @DBI::dbi_debug, 2;
	@DBI::dbi_debug = ($1,$2) if $DBI::dbi_debug =~ m/^(\d+)=(.*)/;
    }
    DBI->trace(@DBI::dbi_debug);
}

%DBI::installed_drh = ();  # maps driver names to installed driver handles


# Setup special DBI dynamic variables. See DBI::var::FETCH for details.
# These are dynamically associated with the last handle used.
tie $DBI::err,    'DBI::var', '*err';    # special case: referenced via IHA list
tie $DBI::state,  'DBI::var', '"state';  # special case: referenced via IHA list
tie $DBI::lasth,  'DBI::var', '!lasth';  # special case: return boolean
tie $DBI::errstr, 'DBI::var', '&errstr'; # call &errstr in last used pkg
tie $DBI::rows,   'DBI::var', '&rows';   # call &rows   in last used pkg
sub DBI::var::TIESCALAR{ my $var = $_[1]; bless \$var, 'DBI::var'; }
sub DBI::var::STORE    { Carp::croak("Can't modify \$DBI::${$_[0]} special variable") }
sub DBI::var::DESTROY  { }

{   package DBI::DBI_tie;	# used to catch DBI->{Attrib} mistake
    sub TIEHASH { bless {} }
    sub STORE   { Carp::carp("DBI->{$_[1]} is invalid syntax (you probably want \$h->{$_[1]})");}
    *FETCH = \&STORE;
}
tie %DBI::DBI => 'DBI::DBI_tie';


# --- Dynamically create the DBI Standard Interface

my $std = undef;
my $keeperr = { O=>0x04 };

my @TieHash_IF = (	# Generic Tied Hash Interface
	'STORE'   => { O=>0x10 },
	'FETCH'   => $keeperr,
	'FIRSTKEY'=> $keeperr,
	'NEXTKEY' => $keeperr,
	'EXISTS'  => $keeperr,
	'CLEAR'   => $keeperr,
	'DESTROY' => undef,	# hardwired internally
);
my @Common_IF = (	# Interface functions common to all DBI classes
	func    =>	{				O=>0x06	},
	event   =>	{ U =>[2,0,'$type, @args'],	O=>0x04 },
	'trace' =>	{ U =>[1,3,'[$trace_level, [$filename]]'],	O=>0x04 },
	trace_msg =>	{ U =>[2,3,'$message_text [, $min_level ]' ],	O=>0x04, T=>8 },
	debug   =>	{ U =>[1,2,'[$debug_level]'],	O=>0x04 }, # old name for trace
	private_data =>	{ U =>[1,1],			O=>0x04 },
	err     =>	$keeperr,
	errstr  =>	$keeperr,
	state   =>	{ U =>[1,1], O=>0x04 },
	_not_impl => $std,
);

%DBI::DBI_methods = ( # Define the DBI interface methods per class:

    dr => {		# Database Driver Interface
	@Common_IF,
	@TieHash_IF,
	'connect'  =>	{ U =>[1,5,'[$db [,$user [,$passwd [,\%attr]]]]'], H=>3 },
	'connect_cached'=>{U=>[1,5,'[$db [,$user [,$passwd [,\%attr]]]]'], H=>3 },
	'disconnect_all'=>{ U =>[1,1] },
	data_sources => { U =>[1,2,'[\%attr]' ] },
	default_user => { U =>[3,4,'$user, $pass [, \%attr]' ] },
    },
    db => {		# Database Session Class Interface
	@Common_IF,
	@TieHash_IF,
	commit     	=> { U =>[1,1] },
	rollback   	=> { U =>[1,1] },
	'do'       	=> { U =>[2,0,'$statement [, \%attr [, @bind_params ] ]'] },
	prepare    	=> { U =>[2,3,'$statement [, \%attr]'] },
	prepare_cached	=> { U =>[2,4,'$statement [, \%attr [, $allow_active ] ]'] },
	selectrow_array	=> { U =>[2,0,'$statement [, \%attr [, @bind_params ] ]'] },
	selectrow_arrayref=>{U =>[2,0,'$statement [, \%attr [, @bind_params ] ]'] },
	selectrow_hashref=>{ U =>[2,0,'$statement [, \%attr [, @bind_params ] ]'] },
	selectall_arrayref=>{U =>[2,0,'$statement [, \%attr [, @bind_params ] ]'] },
	selectall_hashref=>{ U =>[2,0,'$statement [, \%attr [, @bind_params ] ]'] },
	selectcol_arrayref=>{U =>[2,0,'$statement [, \%attr [, @bind_params ] ]'] },
	handler    	=> { U =>[2,2,'\&handler'] },
	ping       	=> { U =>[1,1] },
	disconnect 	=> { U =>[1,1] },
	quote      	=> { U =>[2,3, '$string [, $data_type ]' ], O=>0x30 },
	rows       	=> $keeperr,

	tables          => { U =>[1,2,'[ \%attr ]' ] },
	table_info      => { U =>[1,2,'[ \%attr ]' ] },
	type_info_all	=> { U =>[1,1] },
	type_info	=> { U =>[1,2] },
	get_info	=> { U =>[2,2] },
    },
    st => {		# Statement Class Interface
	@Common_IF,
	@TieHash_IF,
	bind_col	=> { U =>[3,4,'$column, \\$var [, \%attr]'] },
	bind_columns	=> { U =>[2,0,'\\$var1 [, \\$var2, ...]'] },
	bind_param	=> { U =>[3,4,'$parameter, $var [, \%attr]'] },
	bind_param_inout=> { U =>[4,5,'$parameter, \\$var, $maxlen, [, \%attr]'] },
	execute		=> { U =>[1,0,'[@args]'] },

	fetch    	  => undef, # alias for fetchrow_arrayref
	fetchrow_arrayref => undef,
	fetchrow_hashref  => undef,
	fetchrow_array    => undef,
	fetchrow   	  => undef, # old alias for fetchrow_array

	fetchall_arrayref => { U =>[1,2] },

	blob_read  =>	{ U =>[4,5,'$field, $offset, $len [, \\$buf [, $bufoffset]]'] },
	blob_copy_to_file => { U =>[3,3,'$field, $filename_or_handleref'] },
	dump_results => { U =>[1,5,'$maxfieldlen, $linesep, $fieldsep, $filehandle'] },
	more_results => { U =>[1,1] },
	finish     => 	{ U =>[1,1] },
	cancel     => 	{ U =>[1,1] },
	rows       =>	$keeperr,

	_get_fbav	=> undef,
	_set_fbav	=> { T=>6 },
    },
);

my($class, $method);
foreach $class (keys %DBI::DBI_methods){
    my %pkgif = %{ $DBI::DBI_methods{$class} };
    foreach $method (keys %pkgif){
	DBI->_install_method("DBI::${class}::$method", 'DBI.pm',
			$pkgif{$method});
    }
}

# End of init code


END {
    return unless defined &DBI::trace_msg; # return unless bootstrap'd ok
    local ($!,$?);
    DBI->trace_msg("    -- DBI::END\n", 2);
    # Let drivers know why we are calling disconnect_all:
    $DBI::PERL_ENDING = $DBI::PERL_ENDING = 1;	# avoid typo warning
    DBI->disconnect_all() if %DBI::installed_drh;
}



# --- The DBI->connect Front Door methods

sub connect_cached {
    # XXX we expect Apache::DBI users to still call connect()
    my ($class, $dsn, $user, $pass, $attr) = @_;
    ($attr ||= {})->{dbi_connect_method} = 'connect_cached';
    return $class->connect($dsn, $user, $pass, $attr);
}

sub connect {
    my $class = shift;
    my($dsn, $user, $pass, $attr, $old_driver) = @_;
    my $driver;
    my $dbh;

    # switch $old_driver<->$attr if called in old style
    ($old_driver, $attr) = ($attr, $old_driver) if $attr and !ref($attr);

    my $connect_meth = (ref $attr) ? $attr->{dbi_connect_method} : undef;
    $connect_meth ||= $connect_via;	# fallback to default

    $dsn ||= $ENV{DBI_DSN} || $ENV{DBI_DBNAME} || '' unless $old_driver;

    if ($DBI::dbi_debug) {
	local $^W = 0;
	pop @_ if $connect_meth ne 'connect';
	my @args = @_; $args[2] = '****'; # hide password
	DBI->trace_msg("    -> $class->$connect_meth(".join(", ",@args).")\n");
    }
    Carp::croak('Usage: $class->connect([$dsn [,$user [,$passwd [,\%attr]]]])')
	if (ref $old_driver or ($attr and not ref $attr) or ref $pass);

    # extract dbi:driver prefix from $dsn into $1
    $dsn =~ s/^dbi:(\w*?)(?:\((.*?)\))?://i
			or '' =~ /()/; # ensure $1 etc are empty if match fails
    my $driver_attrib_spec = $2;

    # Set $driver. Old style driver, if specified, overrides new dsn style.
    $driver = $old_driver || $1 || $ENV{DBI_DRIVER}
	or Carp::croak("Can't connect(@_), no database driver specified "
		."and DBI_DSN env var not set");

    if ($ENV{DBI_AUTOPROXY} && $driver ne 'Proxy' && $driver ne 'Switch') {
	$dsn = "$ENV{DBI_AUTOPROXY};dsn=dbi:$driver:$dsn";
	$driver = 'Proxy';
	DBI->trace_msg("       DBI_AUTOPROXY: dbi:$driver:$dsn\n");
    }

    unless ($old_driver) { # new-style connect so new default semantics
	$driver_attrib_spec = { split /\s*=>?\s*|\s*,\s*/, $driver_attrib_spec }
	    if $driver_attrib_spec;
	$attr = {
	    PrintError=>1, AutoCommit=>1,
	    ref $attr ? %$attr : (),
	    ref $driver_attrib_spec ? %$driver_attrib_spec : (),
	};
	# XXX to be enabled for DBI v2.0
	#Carp::carp("AutoCommit attribute not specified in DBI->connect")
	#    if $^W && !defined($attr->{AutoCommit});
    }

    my $drh = $class->install_driver($driver)
	|| die "panic: install_driver($driver) failed";

    ($user, $pass) = $drh->default_user($user, $pass, $attr)
	if !(defined $user && defined $pass);

    unless ($dbh = $drh->$connect_meth($dsn, $user, $pass, $attr)) {
	my $msg = "$class->connect($dsn) failed: ".$drh->errstr;
	if (ref $attr) {
	    Carp::croak($msg) if $attr->{RaiseError};
	    Carp::carp ($msg) if $attr->{PrintError};
	}
	DBI->trace_msg("       $msg\n");
	$! = 0; # for the daft people who do DBI->connect(...) || die "$!";
	return undef;
    }

    # XXX this is inelegant but practical in the short term, sigh.
    if ($installed_rootclass{$class}) {
	$dbh->{RootClass} = $class;
	bless $dbh => $class.'::db';
	my ($outer, $inner) = DBI::_handles($dbh);
	bless $inner => $class.'::db';
    }

    if (ref $attr) {
	my %a = %$attr;
	my $a;
	# handle these attributes first
	foreach $a (qw(RaiseError PrintError AutoCommit)) {
	    next unless exists $a{$a};
	    $dbh->{$a} = $a{$a};
	    delete $a{$a};
	}
	foreach $a (keys %a) {
	    $dbh->{$a} = $a{$a};
	}
    }
    DBI->trace_msg("    <- connect= $dbh\n") if $DBI::dbi_debug;

    $dbh;
}


sub disconnect_all {
    foreach(keys %DBI::installed_drh){
	my $drh = $DBI::installed_drh{$_};
	next unless ref $drh;	# avoid problems on premature death
	$drh->disconnect_all();
    }
}


sub disconnect {	# a regular beginners bug
    Carp::croak("DBI->disconnect is not a DBI method. Read the DBI manual.");
}


sub install_driver {		# croaks on failure
    my $class = shift;
    my($driver, $attr) = @_;
    my $drh;
    local $@;

    $driver ||= $ENV{DBI_DRIVER} || '';

    # allow driver to be specified as a 'dbi:driver:' string
    $driver = $1 if $driver =~ s/^DBI:(.*?)://i;

    Carp::croak("usage: $class->install_driver(\$driver [, \%attr])")
		unless ($driver and @_<=3);

    # already installed
    return $drh if $drh = $DBI::installed_drh{$driver};

    DBI->trace_msg("    -> $class->install_driver($driver"
			.") for perl=$] pid=$$ ruid=$< euid=$>\n")
	if $DBI::dbi_debug;

    # --- load the code
    my $driver_class = "DBD::$driver";
    eval "package DBI::_firesafe; require $driver_class";
    if ($@) {
	my $err = $@;
	my $advice = "";
	if ($err =~ /Can't find loadable object/) {
	    $advice = "Perhaps DBD::$driver was statically linked into a new perl binary."
		 ."\nIn which case you need to use that new perl binary."
		 ."\nOr perhaps only the .pm file was installed but not the shared object file."
	}
	elsif ($err =~ /Can't locate.*?DBD\/$driver\.pm in \@INC/) {
	    my @drv = DBI->available_drivers(1);
	    $advice = "Perhaps the DBD::$driver perl module hasn't been fully installed,\n"
		     ."or perhaps the capitalisation of '$driver' isn't right.\n"
		     ."Available drivers: ".join(", ", @drv).".";
	}
	elsif ($err =~ /Can't load .*? for module DBD::/) {
	    $advice = "Perhaps a required shared library or dll isn't installed where expected";
	}
	elsif ($err =~ /Can't locate .*? in \@INC/) {
	    $advice = "Perhaps a module that DBD::$driver requires hasn't been fully installed";
	}
	Carp::croak("install_driver($driver) failed: $err$advice\n");
    }
    if ($DBI::dbi_debug) {
	no strict 'refs';
	(my $driver_file = $driver_class) =~ s/::/\//g;
	my $dbd_ver = ${"$driver_class\::VERSION"} || "undef";
	DBI->trace_msg("       install_driver: $driver_class version $dbd_ver"
		." loaded from $INC{qq($driver_file.pm)}\n");
    }

    # --- do some behind-the-scenes checks and setups on the driver
    _setup_driver($driver_class);

    # --- run the driver function
    $drh = eval { $driver_class->driver($attr || {}) };
    unless ($drh && ref $drh && !$@) {
	my $advice = "";
	# catch people on case in-sensitive systems using the wrong case
	$advice = "\nPerhaps the capitalisation of DBD '$driver' isn't right."
		if $@ =~ /locate object method/;
	croak("$driver_class initialisation failed: $@$advice");
    }

    $DBI::installed_drh{$driver} = $drh;
    DBI->trace_msg("    <- install_driver= $drh\n") if $DBI::dbi_debug;
    $drh;
}

*driver = \&install_driver;	# currently an alias, may change


sub _setup_driver {
    my $driver_class = shift;
    my $type;
    foreach $type (qw(dr db st)){
	my $class = $driver_class."::$type";
	no strict 'refs';
	push @{"${class}::ISA"},     "DBD::_::$type";
	push @{"${class}_mem::ISA"}, "DBD::_mem::$type";
    }
}


sub init_rootclass {
    my $rootclass = shift;
    no strict 'refs';
    croak("Can't init '$rootclass' without '$rootclass\::db' class.")
	unless defined ${"$rootclass\::db::"}{ISA};

    $installed_rootclass{$rootclass} = 1;
    # may do checks on ::db and ::st classes later
    return 1;
}


*internal = \&DBD::Switch::dr::driver;
#sub internal { return DBD::Switch::dr::driver(@_); }


sub available_drivers {
    my($quiet) = @_;
    my(@drivers, $d, $f);
    local(*DBI::DIR, $@);
    my(%seen_dir, %seen_dbd);
    my $haveFileSpec = eval { require File::Spec };
    foreach $d (@INC){
	chomp($d); # Perl 5 beta 3 bug in #!./perl -Ilib from Test::Harness
	my $dbd_dir =
	    ($haveFileSpec ? File::Spec->catdir($d, 'DBD') : "$d/DBD");
	next unless -d $dbd_dir;
	next if $seen_dir{$d};
	$seen_dir{$d} = 1;
	# XXX we have a problem here with case insensitive file systems
	# XXX since we can't tell what case must be used when loading.
	opendir(DBI::DIR, $dbd_dir) || Carp::carp "opendir $dbd_dir: $!\n";
	foreach $f (readdir(DBI::DIR)){
	    next unless $f =~ s/\.pm$//;
	    next if $f eq 'NullP' || $f eq 'Sponge';
	    if ($seen_dbd{$f}){
		Carp::carp "DBD::$f in $d is hidden by DBD::$f in $seen_dbd{$f}\n"
		    unless $quiet;
            } else {
		push(@drivers, $f);
	    }
	    $seen_dbd{$f} = $d;
	}
	closedir(DBI::DIR);
    }
    return sort @drivers;
}

sub data_sources {
    my ($class, $driver, @attr) = @_;
    my $drh = $class->install_driver($driver);
    my @ds = $drh->data_sources(@attr);
    return @ds;
}

sub neat_list {
    my ($listref, $maxlen, $sep) = @_;
    $maxlen = 0 unless defined $maxlen;	# 0 == use internal default
    $sep = ", " unless defined $sep;
    join($sep, map { neat($_,$maxlen) } @$listref);
}


sub dump_results {	# also aliased as a method in DBD::_::st
    my ($sth, $maxlen, $lsep, $fsep, $fh) = @_;
    return 0 unless $sth;
    $maxlen ||= 35;
    $lsep   ||= "\n";
    $fh ||= \*STDOUT;
    my $rows = 0;
    my $ref;
    while($ref = $sth->fetch) {
	print $fh $lsep if $rows++ and $lsep;
	my $str = neat_list($ref,$maxlen,$fsep);
	print $fh $str;	# done on two lines to avoid 5.003 errors
    }
    print $fh "\n$rows rows".($DBI::err ? " ($DBI::err: $DBI::errstr)" : "")."\n";
    $rows;
}



sub connect_test_perf {
    my($class, $dsn,$dbuser,$dbpass, $attr) = @_;
	croak("connect_test_perf needs hash ref as fourth arg") unless ref $attr;
    # these are non standard attributes just for this special method
    my $loops ||= $attr->{dbi_loops} || 5;
    my $par   ||= $attr->{dbi_par}   || 1;	# parallelism
    my $verb  ||= $attr->{dbi_verb}  || 1;
    print "$dsn: testing $loops sets of $par connections:\n";
    require Benchmark;
    require "FileHandle.pm";	# don't let toke.c create empty FileHandle package
    $| = 1;
    my $t0 = new Benchmark;		# not currently used
    my $drh = $class->install_driver($dsn) or Carp::croak("Can't install $dsn driver\n");
    my $t1 = new Benchmark;
    my $loop;
    for $loop (1..$loops) {
	my @cons;
	print "Connecting... " if $verb;
	for (1..$par) {
	    print "$_ ";
	    push @cons, ($drh->connect($dsn,$dbuser,$dbpass)
		    or Carp::croak("Can't connect # $_: $DBI::errstr\n"));
	}
	print "\nDisconnecting...\n" if $verb;
	for (@cons) {
	    $_->disconnect or warn "bad disconnect $DBI::errstr"
	}
    }
    my $t2 = new Benchmark;
    my $td = Benchmark::timediff($t2, $t1);
    printf "Made %2d connections in %s\n", $loops*$par, Benchmark::timestr($td);
	print "\n";
    return $td;
}


# Help people doing DBI->errstr, might even document it one day
# XXX probably best moved to cheaper XS code
sub err    { $DBI::err    }
sub errstr { $DBI::errstr }


# --- Private Internal Function for Creating New DBI Handles

sub _new_handle {
    my ($class, $parent, $attr, $imp_data, $imp_class) = @_;

    Carp::croak('Usage: DBI::_new_handle'
	    .'($class_name, parent_handle, \%attr, $imp_data)'."\n"
	    .'got: ('.join(", ",$class, $parent, $attr, $imp_data).")\n")
	unless (@_ == 5	and (!$parent or ref $parent)
			and ref $attr eq 'HASH'
			and $imp_class);

    $attr->{ImplementorClass} = $imp_class
	or Carp::croak("_new_handle($class): 'ImplementorClass' attribute not given");

    DBI->trace_msg("    New $class (for $imp_class, parent=$parent, id=".($imp_data||'').")\n")
	if $DBI::dbi_debug >= 3;

    # This is how we create a DBI style Object:
    my(%hash, $i, $h);
    $i = tie    %hash, $class, $attr;  # ref to inner hash (for driver)
    $h = bless \%hash, $class;         # ref to outer hash (for application)
    # The above tie and bless may migrate down into _setup_handle()...
    # Now add magic so DBI method dispatch works
    DBI::_setup_handle($h, $imp_class, $parent, $imp_data);

    return $h unless wantarray;
    ($h, $i);
}
# minimum constructors for the tie's (alias to XS version)
sub DBI::st::TIEHASH { bless $_[1] => $_[0] };
*DBI::dr::TIEHASH = \&DBI::st::TIEHASH;
*DBI::db::TIEHASH = \&DBI::st::TIEHASH;


# These three special constructors are called by the drivers
# The way they are called is likey to change.

sub _new_drh {	# called by DBD::<drivername>::driver()
    my ($class, $initial_attr, $imp_data) = @_;
    # Provide default storage for State,Err and Errstr.
    # Note that these are shared by all child handles by default! XXX
    # State must be undef to get automatic faking in DBI::var::FETCH
    my ($h_state_store, $h_err_store, $h_errstr_store) = (undef, 0, '');
    my $attr = {
	# these attributes get copied down to child handles by default
	'Handlers'	=> [],
	'State'		=> \$h_state_store,  # Holder for DBI::state
	'Err'		=> \$h_err_store,    # Holder for DBI::err
	'Errstr'	=> \$h_errstr_store, # Holder for DBI::errstr
	'Debug' 	=> 0,
	%$initial_attr,
    };
    _new_handle('DBI::dr', '', $attr, $imp_data, $class);
}

sub _new_dbh {	# called by DBD::<drivername>::dr::connect()
    my ($drh, $attr, $imp_data) = @_;
    my $imp_class = $drh->{ImplementorClass}
	or Carp::croak("DBI _new_dbh: $drh has no ImplementorClass");
    substr($imp_class,-4,4) = '::db';
    my $app_class = ref $drh;
    substr($app_class,-4,4) = '::db';
    _new_handle($app_class, $drh, $attr||{}, $imp_data, $imp_class);
}

sub _new_sth {	# called by DBD::<drivername>::db::prepare)
    my ($dbh, $attr, $imp_data) = @_;
    my $imp_class = $dbh->{ImplementorClass}
	or Carp::croak("DBI _new_sth: $dbh has no ImplementorClass");
    substr($imp_class,-4,4) = '::st';
    my $app_class = ref $dbh;
    substr($app_class,-4,4) = '::st';
    _new_handle($app_class, $dbh, $attr, $imp_data, $imp_class);
}


} # end of DBI package scope



# --------------------------------------------------------------------
# === The internal DBI Switch pseudo 'driver' class ===

{   package DBD::Switch::dr;
    DBI::_setup_driver('DBD::Switch');	# sets up @ISA
    require Carp;

    $imp_data_size = 0;
    $imp_data_size = 0;	# avoid typo warning
    $err = 0;

    sub driver {
	return $drh if $drh;	# a package global

	my $inner;
	($drh, $inner) = DBI::_new_drh('DBD::Switch::dr', {
		'Name'    => 'Switch',
		'Version' => $DBI::VERSION,
		# the Attribution is defined as a sub as an example
		'Attribution' => sub { "DBI $DBI::VERSION by Tim Bunce" },
	    }, \$err);
	Carp::croak("DBD::Switch init failed!") unless ($drh && $inner);
	return $drh;
    }

    sub FETCH {
	my($drh, $key) = @_;
	return DBI->trace if $key eq 'DebugDispatch';
	return undef if $key eq 'DebugLog';	# not worth fetching, sorry
	return $drh->DBD::_::dr::FETCH($key);
	undef;
    }
    sub STORE {
	my($drh, $key, $value) = @_;
	if ($key eq 'DebugDispatch') {
	    DBI->trace($value);
	} elsif ($key eq 'DebugLog') {
	    DBI->trace(-1, $value);
	} else {
	    $drh->DBD::_::dr::STORE($key, $value);
	}
    }
}


# --------------------------------------------------------------------
# === OPTIONAL MINIMAL BASE CLASSES FOR DBI SUBCLASSES ===

# We only define default methods for harmless functions.
# We don't, for example, define a DBD::_::st::prepare()

{   package DBD::_::common; # ====== Common base class methods ======
    use strict;

    # methods common to all handle types:

    sub _not_impl {
	my ($h, $method) = @_;
	$h->trace_msg("Driver does not implement the $method method.\n");
	return;	# empty list / undef
    }

    # generic TIEHASH default methods:
    sub FIRSTKEY { }
    sub NEXTKEY  { }
    sub EXISTS   { defined($_[0]->FETCH($_[1])) } # XXX undef?
    sub CLEAR    { Carp::carp "Can't CLEAR $_[0] (DBI)" }

    *dump_handle = \&DBI::dump_handle;
}


{   package DBD::_::dr;  # ====== DRIVER ======
    @ISA = qw(DBD::_::common);
    use strict;

    sub default_user {
	my ($drh, $user, $pass) = @_;
	unless (defined $user) {
	    $user = $ENV{DBI_USER};
	    carp("DBI connect: user not defined and DBI_USER env var not set")
		if 0 && !defined $user && $drh->{Warn};	# XXX enable later
	}
	unless (defined $pass) {
	    $pass = $ENV{DBI_PASS};
	    carp("DBI connect: password not defined and DBI_PASS env var not set")
		if 0 && !defined $pass && $drh->{Warn};	# XXX enable later
	}
	return ($user, $pass);
    }

    sub connect { # normally overridden, but a handy default
	my ($drh, $dsn, $user, $auth) = @_;
	my ($this) = DBI::_new_dbh($drh, {
	    'Name' => $dsn,
	    'User' => $user,
	});
	$this;
    }


    sub connect_cached {
	my $drh = shift;
	my ($dsn, $user, $auth, $attr)= @_;

	# Needs support at dbh level to clear cache before complaining about
	# active children. The XS template code does this. Drivers not using
	# the template must handle clearing the cache themselves.
	my $cache = $drh->FETCH('CachedKids');
	$drh->STORE('CachedKids', $cache = {}) unless $cache;

	my @attr_keys = $attr ? sort keys %$attr : ();
	my $key = join "~~", $dsn, $user||'', $auth||'',
		$attr ? (@attr_keys,@{$attr}{@attr_keys}) : ();
	my $dbh = $cache->{$key};
	return $dbh if $dbh && $dbh->FETCH('Active') && eval { $dbh->ping };
	$dbh = $drh->connect(@_);
	$cache->{$key} = $dbh;	# replace prev entry, even if connect failed
	return $dbh;
    }


    sub disconnect_all {	# Driver must take responsibility for this
	# XXX Umm, may change later.
	Carp::croak("Driver has not implemented the disconnect_all method.");
    }

    sub data_sources {
	shift->_not_impl('data_sources');
    }

}


{   package DBD::_::db;  # ====== DATABASE ======
    @ISA = qw(DBD::_::common);
    use strict;

    sub disconnect  {
	shift->_not_impl('disconnect');
    }

    # Drivers are required to implement *::db::DESTROY to encourage tidy-up
    sub DESTROY  { Carp::croak("Driver has not implemented DESTROY for @_") }

    sub quote {
	my ($dbh, $str, $data_type) = @_;
	return "NULL" unless defined $str;
	unless ($data_type) {
	    $str =~ s/'/''/g;		# ISO SQL2
	    return "'$str'";
	}
	# Optimise for standard numerics which need no quotes
	return $str if $data_type == DBI::SQL_INTEGER
		    || $data_type == DBI::SQL_SMALLINT
		    || $data_type == DBI::SQL_DECIMAL
		    || $data_type == DBI::SQL_FLOAT
		    || $data_type == DBI::SQL_REAL
		    || $data_type == DBI::SQL_DOUBLE
		    || $data_type == DBI::SQL_NUMERIC;
	my $ti = $dbh->type_info($data_type);
	# XXX needs checking
	my $lp = $ti ? $ti->{LITERAL_PREFIX} || "" : "'";
	my $ls = $ti ? $ti->{LITERAL_SUFFIX} || "" : "'";
	# XXX don't know what the standard says about escaping
	# in the 'general case' (where $lp != "'").
	# So we just do this and hope:
	$str =~ s/$lp/$lp$lp/g
		if $lp && $lp eq $ls && ($lp eq "'" || $lp eq '"');
	return "$lp$str$ls";
    }

    sub rows { -1 }	# here so $DBI::rows 'works' after using $dbh

    sub do {
	my($dbh, $statement, $attr, @params) = @_;
	my $sth = $dbh->prepare($statement, $attr) or return undef;
	$sth->execute(@params) or return undef;
	my $rows = $sth->rows;
	($rows == 0) ? "0E0" : $rows;
    }

    sub _do_selectrow {
	my ($method, $dbh, $stmt, $attr, @bind) = @_;
	my $sth = (ref $stmt) ? $stmt
			      : $dbh->prepare($stmt, $attr);
	return unless $sth;
	$sth->execute(@bind) || return;
	my $row = $sth->$method();
	$sth->finish if $row;
	return $row;
    }

    sub selectrow_arrayref { return _do_selectrow('fetchrow_arrayref', @_); }
    sub selectrow_hashref {  return _do_selectrow('fetchrow_hashref',  @_); }

    sub selectrow_array {
	my ($dbh, $stmt, $attr, @bind) = @_;
	my $sth = (ref $stmt) ? $stmt
			      : $dbh->prepare($stmt, $attr);
	return unless $sth;
	$sth->execute(@bind) || return;
	my @row = $sth->fetchrow_array;
	$sth->finish if @row;
	return $row[0] unless wantarray;
	return @row;
    }

    sub selectall_arrayref {
	my ($dbh, $stmt, $attr, @bind) = @_;
	my $sth = (ref $stmt) ? $stmt
			      : $dbh->prepare($stmt, $attr);
	return unless $sth;
	$sth->execute(@bind) || return;
	my $slice = $attr->{dbi_fetchall_arrayref_attr}; # typically undef
	return $sth->fetchall_arrayref($slice);
    }

    sub selectall_hashref {
	my ($dbh, $stmt, $attr, @bind) = @_;
	my $sth = (ref $stmt) ? $stmt
			      : $dbh->prepare($stmt, $attr);
	return unless $sth;
	$sth->execute(@bind) || return;
	my ($row, @rows);
	push @rows, $row while ($row = $sth->fetchrow_hashref());
	return \@rows;
    }

    sub selectcol_arrayref {
	my ($dbh, $stmt, $attr, @bind) = @_;
	my $sth = (ref $stmt) ? $stmt
			      : $dbh->prepare($stmt, $attr);
	return unless $sth;
	$sth->execute(@bind) || return;
	my $column = 1;
	my $value;
	$sth->bind_col($column, \$value) || return;
	my @col;
	push @col, $value while $sth->fetch;
	return \@col;
    }

    sub prepare_cached {
	my ($dbh, $statement, $attr, $allow_active) = @_;
	# Needs support at dbh level to clear cache before complaining about
	# active children. The XS template code does this. Drivers not using
	# the template must handle clearing the cache themselves.
	my $cache = $dbh->FETCH('CachedKids');
	$dbh->STORE('CachedKids', $cache = {}) unless $cache;
	my @attr_keys = ($attr) ? sort keys %$attr : ();
	my $key = ($attr) ? join("~~", $statement, @attr_keys, @{$attr}{@attr_keys}) : $statement;
	my $sth = $cache->{$key};
	if ($sth) {
	    if ($sth->FETCH('Active')) {
		Carp::carp("prepare_cached($statement) statement handle $sth was still active")
		    if !$allow_active;
		$sth->finish;
	    }
	    return $sth;
	}
	$sth = $dbh->prepare($statement, $attr);
	$cache->{$key} = $sth if $sth;
	return $sth;
    }

    sub ping {
	shift->_not_impl('ping');
	"0 but true";	# special kind of true 0
    }

    sub commit {
	shift->_not_impl('commit');
    }
    sub rollback {
	shift->_not_impl('rollback');
    }

    sub get_info {
	shift->_not_impl("get_info @_");
    }

    sub table_info {
	shift->_not_impl('table_info');
    }

    sub tables {
	my ($dbh, @args) = @_;
	my $sth = $dbh->table_info(@args);
	return unless $sth;
	my ($row, @tables);
	while($row = $sth->fetch) {
	    my $name = $row->[2];
	    if ($row->[1]) {
		my $schema = $row->[1];
		# a little hack
		my $quote = ($schema eq uc($schema)) ? '' : '"';
		$name = "$quote$schema$quote.$name"
	    }
	    push @tables, $name;
	}
	return @tables;
    }

    sub type_info_all {
	my ($dbh) = @_;
	$dbh->_not_impl('type_info_all');
	my $ti = [ {} ];
	return $ti;
    }

    sub type_info {
	my ($dbh, $data_type) = @_;
	my $tia = $dbh->type_info_all;
	return unless @$tia;
	my $idx_hash = shift @$tia;

	my $dt_idx   = $idx_hash->{DATA_TYPE} || $idx_hash->{data_type};
	Carp::croak("type_info_all returned non-standard DATA_TYPE index value ($dt_idx != 1)")
	    if $dt_idx && $dt_idx != 1;

	# --- simple DATA_TYPE match filter
	my @ti;
	my @data_type_list = (ref $data_type) ? @$data_type : ($data_type);
	foreach $data_type (@data_type_list) {
	    if (defined($data_type) && $data_type != DBI::SQL_ALL_TYPES()) {
		push @ti, grep { $_->[$dt_idx] == $data_type } @$tia;
	    }
	    else {	# SQL_ALL_TYPES
		push @ti, @$tia;
	    }
	    last if @ti;	# found at least one match
	}

	# --- format results into list of hash refs
	my $idx_fields = keys %$idx_hash;
	my @idx_names  = map { uc($_) } keys %$idx_hash;
	my @idx_values = values %$idx_hash;
	Carp::croak "type_info_all result has $idx_fields keys but ".(@{$ti[0]})." fields"
		if @ti && @{$ti[0]} != $idx_fields;
	my @out = map {
	    my %h; @h{@idx_names} = @{$_}[ @idx_values ]; \%h;
	} @ti;
	return $out[0] unless wantarray;
	return @out;
    }
}


{   package DBD::_::st;  # ====== STATEMENT ======
    @ISA = qw(DBD::_::common);
    use strict;

    sub cancel  { undef }
    sub bind_param { Carp::croak("Can't bind_param, not implement by driver") }

    sub fetchall_arrayref {
	my $sth = shift;
	my $slice= shift || [];
	my $mode = ref $slice;
	my @rows;
	my $row;
	if ($mode eq 'ARRAY') {
	    # we copy the array here because fetch (currently) always
	    # returns the same array ref. XXX
	    if (@$slice) {
		push @rows, [ @{$row}[ @$slice] ] while($row = $sth->fetch);
	    }
	    else {
		push @rows, [ @$row ] while($row = $sth->fetch);
	    }
	}
	elsif ($mode eq 'HASH') {
	    my @o_keys = keys %$slice;
	    if (@o_keys) {
		my %i_names = map {  (lc($_)=>$_) } @{ $sth->FETCH('NAME') };
		my @i_keys  = map { $i_names{lc($_)} } @o_keys;
		while ($row = $sth->fetchrow_hashref) {
		    my %hash;
		    @hash{@o_keys} = @{$row}{@i_keys};
		    push @rows, \%hash;
		}
	    }
	    else {
		# XXX assumes new ref each fetchhash
		push @rows, $row while ($row = $sth->fetchrow_hashref);
	    }
	}
	else { Carp::croak("fetchall_arrayref($mode) invalid") }
	return \@rows;
    }

    *dump_results = \&DBI::dump_results;

    sub blob_copy_to_file {	# returns length or undef on error
	my($self, $field, $filename_or_handleref, $blocksize) = @_;
	my $fh = $filename_or_handleref;
	my($len, $buf) = (0, "");
	$blocksize ||= 512;	# not too ambitious
	local(*FH);
	unless(ref $fh) {
	    open(FH, ">$fh") || return undef;
	    $fh = \*FH;
	}
	while(defined($self->blob_read($field, $len, $blocksize, \$buf))) {
	    print $fh $buf;
	    $len += length $buf;
	}
	close(FH);
	$len;
    }

    sub more_results {
	shift->{syb_more_results};	# handy grandfathering
    }

    # Drivers are required to implement *::st::DESTROY to encourage tidy-up
    sub DESTROY  { Carp::croak("Driver has not implemented DESTROY for @_") }
}

{   # See install_driver
    { package DBD::_mem::dr; @ISA = qw(DBD::_mem::common);	}
    { package DBD::_mem::db; @ISA = qw(DBD::_mem::common);	}
    { package DBD::_mem::st; @ISA = qw(DBD::_mem::common);	}
    # DBD::_mem::common::DESTROY is implemented in DBI.xs
}

1;
__END__

=head1 DESCRIPTION

The DBI is a database access module for the Perl programming language.  It defines
a set of methods, variables, and conventions that provide a consistent
database interface, independent of the actual database being used.

It is important to remember that the DBI is just an interface. 
The DBI is a layer
of "glue" between an application and one or more database I<driver>
modules.  It is the driver modules which do most of the real work. The DBI
provides a standard interface and framework for the drivers to operate
within.


=head2 Architecture of a DBI Application

             |<- Scope of DBI ->|
                  .-.   .--------------.   .-------------.
  .-------.       | |---| XYZ Driver   |---| XYZ Engine  |
  | Perl  |       | |   `--------------'   `-------------'
  | script|  |A|  |D|   .--------------.   .-------------.
  | using |--|P|--|B|---|Oracle Driver |---|Oracle Engine|
  | DBI   |  |I|  |I|   `--------------'   `-------------'
  | API   |       | |...
  |methods|       | |... Other drivers
  `-------'       | |...
                  `-'

The API, or Application Programming Interface, defines the 
call interface and variables for Perl scripts to use. The API
is implemented by the Perl DBI extension.

The DBI "dispatches" the method calls to the appropriate driver for
actual execution.  The DBI is also responsible for the dynamic loading
of drivers, error checking and handling, providing default
implementations for methods, and many other non-database specific duties.

Each driver 
contains implementations of the DBI methods using the
private interface functions of the corresponding database engine.  Only authors
of sophisticated/multi-database applications or generic library
functions need be concerned with drivers.

=head2 Notation and Conventions

The following conventions are used in this document:

  $dbh    Database handle object
  $sth    Statement handle object
  $drh    Driver handle object (rarely seen or used in applications)
  $h      Any of the handle types above ($dbh, $sth, or $drh)
  $rc     General Return Code  (boolean: true=ok, false=error)
  $rv     General Return Value (typically an integer)
  @ary    List of values returned from the database, typically a row of data
  $rows   Number of rows processed (if available, else -1)
  $fh     A filehandle
  undef   NULL values are represented by undefined values in Perl
  \%attr  Reference to a hash of attribute values passed to methods

Note that Perl will automatically destroy database and statement handle objects
if all references to them are deleted.


=head2 Outline Usage

To use DBI, 
first you need to load the DBI module:

  use DBI;
  use strict;

(The C<use strict;> isn't required but is strongly recommended.)

Then you need to L</connect> to your data source and get a I<handle> for that
connection:

  $dbh = DBI->connect($dsn, $user, $password,
                      { RaiseError => 1, AutoCommit => 0 });

Since connecting can be expensive, you generally just connect at the
start of your program and disconnect at the end.

Explicitly defining the required C<AutoCommit> behavior is strongly
recommended and may become mandatory in a later version.  This
determines whether changes are automatically committed to the
database when executed, or need to be explicitly committed later.

The DBI allows an application to "prepare" statements for later
execution.  A prepared statement is identified by a statement handle
held in a Perl variable.
We'll call the Perl variable C<$sth> in our examples.

The typical method call sequence for a C<SELECT> statement is:

  prepare,
    execute, fetch, fetch, ...
    execute, fetch, fetch, ...
    execute, fetch, fetch, ...

for example:

  $sth = $dbh->prepare("SELECT foo, bar FROM table WHERE baz=?");

  $sth->execute( $baz );

  while ( @row = $sth->fetchrow_array ) {
    print "@row\n";
  }

The typical method call sequence for a I<non>-C<SELECT> statement is:

  prepare,
    execute,
    execute,
    execute.

for example:

  $sth = $dbh->prepare("INSERT INTO table(foo,bar,baz) VALUES (?,?,?)");

  while(<CSV>) {
    chomp;
    my ($foo,$bar,$baz) = split /,/;
	$sth->execute( $foo, $bar, $baz );
  }

The C<do()> method can be used for non repeated I<non>-C<SELECT> statement
(or with drivers that don't support placeholders):

  $rows_affected = $dbh->do("UPDATE your_table SET foo = foo + 1");

To commit your changes to the database (when L</AutoCommit> is off):

  $dbh->commit;  # or call $dbh->rollback; to undo changes

Finally, when you have finished working with the data source, you should
L</disconnect> from it:

  $dbh->disconnect;


=head2 General Interface Rules & Caveats

The DBI does not have a concept of a "current session". Every session
has a handle object (i.e., a C<$dbh>) returned from the C<connect> method.
That handle object is used to invoke database related methods.

Most data is returned to the Perl script as strings. (Null values are
returned as C<undef>.)  This allows arbitrary precision numeric data to be
handled without loss of accuracy.  Beware that Perl may not preserve
the same accuracy when the string is used as a number.

Dates and times are returned as character strings in the current
default format of the corresponding database engine.  Time zone effects
are database/driver dependent.

Perl supports binary data in Perl strings, and the DBI will pass binary
data to and from the driver without change. It is up to the driver
implementors to decide how they wish to handle such binary data.

Most databases that understand multiple character sets have a
default global charset. Text stored in the database is, or should
be, stored in that charset; if not, then that's the fault of either
the database or the application that inserted the data. When text is
fetched it should be automatically converted to the charset of the
client, presumably based on the locale. If a driver needs to set a
flag to get that behavior, then it should do so; it should not require
the application to do that.

Multiple SQL statements may not be combined in a single statement
handle (C<$sth>), although some databases and drivers do support this
(notably Sybase and SQL Server).

Non-sequential record reads are not supported in this version of the DBI.
In other words, records can only be fetched in the order that the
database returned them, and once fetched they are forgotten.

Positioned updates and deletes are not directly supported by the DBI.
See the description of the C<CursorName> attribute for an alternative.

Individual driver implementors are free to provide any private
functions and/or handle attributes that they feel are useful.
Private driver functions can be invoked using the DBI C<func()> method.
Private driver attributes are accessed just like standard attributes.

Many methods have an optional C<\%attr> parameter which can be used to
pass information to the driver implementing the method. Except where
specifically documented, the C<\%attr> parameter can only be used to pass
driver specific hints. In general, you can ignore C<\%attr> parameters
or pass it as C<undef>.


=head2 Naming Conventions and Name Space

The DBI package and all packages below it (C<DBI::*>) are reserved for
use by the DBI. Extensions and related modules use the C<DBIx::>
namespace (see C<http://www.perl.com/CPAN/modules/by-module/DBIx/>).
Package names beginning with C<DBD::> are reserved for use
by DBI database drivers.  All environment variables used by the DBI
or by individual DBDs begin with "C<DBI_>" or "C<DBD_>".

The letter case used for attribute names is significant and plays an
important part in the portability of DBI scripts.  The case of the
attribute name is used to signify who defined the meaning of that name
and its values.

  Case of name  Has a meaning defined by
  ------------  ------------------------
  UPPER_CASE    Standards, e.g.,  X/Open, ISO SQL92 etc (portable)
  MixedCase     DBI API (portable), underscores are not used.
  lower_case    Driver or database engine specific (non-portable)

It is of the utmost importance that Driver developers only use
lowercase attribute names when defining private attributes. Private
attribute names must be prefixed with the driver name or suitable
abbreviation (e.g., "C<ora_>" for Oracle, "C<ing_>" for Ingres, etc).

Driver Specific Prefix Registry:

  ad_      DBD::AnyData
  ado_     DBD::ADO
  best_    DBD::BestWins
  csv_     DBD::CSV
  db2_     DBD::DB2
  f_       DBD::File
  file_    DBD::TextFile
  ib_      DBD::InterBase
  ing_     DBD::Ingres
  ix_      DBD::Informix
  msql_    DBD::mSQL
  mysql_   DBD::mysql
  odbc_    DBD::ODBC
  ora_     DBD::Oracle
  pg_      DBD::Pg
  proxy_   DBD::Proxy
  rdb_     DBD::RDB
  sapdb_   DBD::SAP_DB
  solid_   DBD::Solid
  syb_     DBD::Sybase
  tdat_    DBD::Teradata
  tuber_   DBD::Tuber
  uni_     DBD::Unify
  xbase_   DBD::XBase


=head2 SQL - A Query Language

Most DBI drivers require applications to use a dialect of SQL 
(Structured Query Language) to interact with the database engine.
The following links provide useful information and further links about
SQL:

  http://www.altavista.com/query?q=sql+tutorial
  http://www.jcc.com/sql_stnd.html
  http://www.contrib.andrew.cmu.edu/~shadow/sql.html

The DBI itself does not mandate or require any particular language to
be used; it is language independent. In ODBC terms, the DBI is in
"pass-thru" mode, although individual drivers might not be. The only requirement
is that queries and other statements must be expressed as a single
string of characters passed as the first argument to the L</prepare> or
L</do> methods.

For an interesting diversion on the I<real> history of RDBMS and SQL,
from the people who made it happen, see:

  http://ftp.digital.com/pub/DEC/SRC/technical-notes/SRC-1997-018-html/sqlr95.html

Follow the "And the rest" and "Intergalactic dataspeak" links for the
SQL history.

=head2 Placeholders and Bind Values

Some drivers support placeholders and bind values. 
I<Placeholders>, also called parameter markers, are used to indicate
values in a database statement that will be supplied later,
before the prepared statement is executed.  For example, an application
might use the following to insert a row of data into the SALES table:

  INSERT INTO sales (product_code, qty, price) VALUES (?, ?, ?)

or the following, to select the description for a product:

  SELECT description FROM products WHERE product_code = ?

The C<?> characters are the placeholders.  The association of actual
values with placeholders is known as I<binding>, and the values are
referred to as I<bind values>.

When using placeholders with the SQL C<LIKE> qualifier, you must
remember that the placeholder substitutes for the whole string.
So you should use "C<... LIKE ? ...>" and include any wildcard
characters in the value that you bind to the placeholder.

B<Null Values>

Undefined values, or C<undef>, can be used to indicate null values.
However, care must be taken in the particular case of trying to use
null values to qualify a C<SELECT> statement. Consider:

  SELECT description FROM products WHERE product_code = ?

Binding an C<undef> (NULL) to the placeholder will I<not> select rows
which have a NULL C<product_code>! Refer to the SQL manual for your database
engine or any SQL book for the reasons for this.  To explicitly select
NULLs you have to say "C<WHERE product_code IS NULL>" and to make that
general you have to say:

  ... WHERE (product_code = ? OR (? IS NULL AND product_code IS NULL))

and bind the same value to both placeholders.

B<Performance>

Without using placeholders, the insert statement shown previously would have to
contain the literal values to be inserted and would have to be
re-prepared and re-executed for each row. With placeholders, the insert
statement only needs to be prepared once. The bind values for each row
can be given to the C<execute> method each time it's called. By avoiding
the need to re-prepare the statement for each row, the application
typically runs many times faster. Here's an example:

  my $sth = $dbh->prepare(q{
    INSERT INTO sales (product_code, qty, price) VALUES (?, ?, ?)
  }) or die $dbh->errstr;
  while (<>) {
      chomp;
      my ($product_code, $qty, $price) = split /,/;
      $sth->execute($product_code, $qty, $price) or die $dbh->errstr;
  }
  $dbh->commit or die $dbh->errstr;

See L</execute> and L</bind_param> for more details.

The C<q{...}> style quoting used in this example avoids clashing with
quotes that may be used in the SQL statement. Use the double-quote like
C<qq{...}> operator if you want to interpolate variables into the string.
See L<perlop/"Quote and Quote-like Operators"> for more details.

See also the L</bind_column> method, which is used to associate Perl
variables with the output columns of a C<SELECT> statement.

=head1 THE DBI PACKAGE AND CLASS

In this section, we cover the DBI class methods, utility functions,
and the dynamic attributes associated with generic DBI handles.

=head2 DBI Constants

The following SQL standard type constants can be imported individually
or, by importing the special C<:sql_types> tag, all together:

  SQL_CHAR SQL_NUMERIC SQL_DECIMAL SQL_INTEGER SQL_SMALLINT
  SQL_FLOAT SQL_REAL SQL_DOUBLE SQL_VARCHAR
  SQL_DATE SQL_TIME SQL_TIMESTAMP
  SQL_LONGVARCHAR SQL_BINARY SQL_VARBINARY SQL_LONGVARBINARY
  SQL_BIGINT SQL_TINYINT
  SQL_WCHAR SQL_WVARCHAR SQL_WLONGVARCHAR
  SQL_BIT
  SQL_ALL_TYPES

See the L</type_info>, L</type_info_all>, and L</bind_param> methods
for possible uses.

=head2 DBI Class Methods

The following methods are provided by the DBI class:

=over 4

=item C<connect>

  $dbh = DBI->connect($data_source, $username, $password)
            or die $DBI::errstr;
  $dbh = DBI->connect($data_source, $username, $password, \%attr)
            or die $DBI::errstr;

Establishes a database connection, or session, to the requested C<$data_source>.
Returns a database handle object if the connection succeeds. Use
C<$dbh->E<gt>C<disconnect> to terminate the connection.

If the connect fails (see below), it returns C<undef> and sets both C<$DBI::err>
and C<$DBI::errstr>. (It does I<not> set C<$!>, etc.) You should generally
test the return status of C<connect> and C<print $DBI::errstr> if it has failed.

Multiple simultaneous connections to multiple databases through multiple
drivers can be made via the DBI. Simply make one C<connect> call for each
database and keep a copy of each returned database handle.

The C<$data_source> value should begin with "C<dbi:>I<driver_name>C<:>". The
I<driver_name> specifies the driver that will be used to make the
connection. (Letter case is significant.)

As a convenience, if the C<$data_source> parameter is undefined or empty, the
DBI will substitute the value of the environment variable C<DBI_DSN>.
If just the I<driver_name> part is empty (i.e., the C<$data_source> prefix is "C<dbi::>"),
the environment variable C<DBI_DRIVER> is used. If neither variable is set,
then C<connect> dies.

Examples of C<$data_source> values are:

  dbi:DriverName:database_name
  dbi:DriverName:database_name@hostname:port
  dbi:DriverName:database=database_name;host=hostname;port=port

There is I<no standard> for the text following the driver name. Each
driver is free to use whatever syntax it wants. The only requirement the
DBI makes is that all the information is supplied in a single string.
You must consult the documentation for the drivers you are using for a
description of the syntax they require. (Where a driver author needs
to define a syntax for the C<$data_source>, it is recommended that
they follow the ODBC style, shown in the last example above.)

If the environment variable C<DBI_AUTOPROXY> is defined (and the driver in
C<$data_source> is not "C<Proxy>") then the connect request will
automatically be changed to:

  dbi:Proxy:$ENV{DBI_AUTOPROXY};dsn=$data_source

and passed to the DBD::Proxy module. C<DBI_AUTOPROXY> is typically set as
"C<hostname=...;port=...>". See the DBD::Proxy documentation for more details.

If C<$username> or C<$password> are undefined (rather than just empty),
then the DBI will substitute the values of the C<DBI_USER> and C<DBI_PASS>
environment variables, respectively.  The DBI will warn if the
environment variables are not defined.  However, the everyday use of 
these environment 
variables is not recommended for security reasons. The mechanism is
primarily intended to simplify testing.

C<DBI->E<gt>C<connect> automatically installs the driver if it has not been
installed yet. Driver installation either returns a valid driver
handle, or it I<dies> with an error message that includes the string
"C<install_driver>" and the underlying problem. So C<DBI->E<gt>C<connect> 
will die
on a driver installation failure and will only return C<undef> on a
connect failure, in which case C<$DBI::errstr> will hold the error message.

The C<$data_source> argument (with the "C<dbi:...:>" prefix removed) and the
C<$username> and C<$password> arguments are then passed to the driver for
processing. The DBI does not define any interpretation for the
contents of these fields.  The driver is free to interpret the
C<$data_source>, C<$username>, and C<$password> fields in any way, and supply
whatever defaults are appropriate for the engine being accessed.
(Oracle, for example, uses the ORACLE_SID and TWO_TASK environment 
variables if no C<$data_source> is specified.)

The C<AutoCommit> and C<PrintError> attributes for each connection default to
"on". (See L</AutoCommit> and L</PrintError> for more information.)
However, it is strongly recommended that you explicitly define C<AutoCommit>
rather than rely on the default. Future versions of
the DBI may issue a warning if C<AutoCommit> is not explicitly defined.

The C<\%attr> parameter can be used to alter the default settings of 
C<PrintError>, C<RaiseError>, C<AutoCommit>, and other attributes. For example:

  $dbh = DBI->connect($data_source, $user, $pass, {
	PrintError => 0,
	AutoCommit => 0
  });

You can also define connection attribute values within the C<$data_source>
parameter. For example:

  dbi:DriverName(PrintError=>0,Taint=>1):...

Individual attributes values specified in this way take precedence over
any conflicting values specified via the C<\%attr> parameter to C<connect>.

The C<dbi_connect_method> attribute can be used to specify which driver
method should be called to establish the connection. The only useful
values are 'connect', 'connect_cached', or some specialized case like
'Apache::DBI::connect' (which is automatically the default when running
within Apache).

Where possible, each session (C<$dbh>) is independent from the transactions
in other sessions. This is useful when you need to hold cursors open
across transactions--for example, if you use one session for your long lifespan
cursors (typically read-only) and another for your short update
transactions.

For compatibility with old DBI scripts, the driver can be specified by
passing its name as the fourth argument to C<connect> (instead of C<\%attr>):

  $dbh = DBI->connect($data_source, $user, $pass, $driver);

In this "old-style" form of C<connect>, the C<$data_source> should not start
with "C<dbi:driver_name:>". (If it does, the embedded driver_name
will be ignored). Also note that in this older form of C<connect>, 
the C<$dbh->E<gt>C<{AutoCommit}> attribute is I<undefined>, the
C<$dbh->E<gt>C<{PrintError}> attribute is off, and the old C<DBI_DBNAME>
environment variable is
checked if C<DBI_DSN> is not defined. Beware that this "old-style" 
C<connect> will be withdrawn in a future version of DBI.

=item C<connect_cached> I<NEW>

  $dbh = DBI->connect_cached($data_source, $username, $password)
            or die $DBI::errstr;
  $dbh = DBI->connect_cached($data_source, $username, $password, \%attr)
            or die $DBI::errstr;

C<connect_cached> is like L</connect>, except that the database handle
returned is also
stored in a hash associated with the given parameters. If another call
is made to C<connect_cached> with the same parameter values, then the
corresponding cached C<$dbh> will be returned if it is still valid.
The cached database handle is replaced with a new connection if it
has been disconnected or if the C<ping> method fails.

Note that the behavior of this method differs in several respects from the
behavior of presistent connections implemented by Apache::DBI.

Caching can be useful in some applications, but it can also cause
problems and should be used with care. The exact behavior of this
method is liable to change, so if you intend to use it in any production
applications you should discuss your needs on the I<dbi-users> mailing list.

The cache can be accessed (and cleared) via the L</CachedKids> attribute.


=item C<available_drivers>

  @ary = DBI->available_drivers;
  @ary = DBI->available_drivers($quiet);

Returns a list of all available drivers by searching for C<DBD::*> modules
through the directories in C<@INC>. By default, a warning is given if
some drivers are hidden by others of the same name in earlier
directories. Passing a true value for C<$quiet> will inhibit the warning.


=item C<data_sources>

  @ary = DBI->data_sources($driver);
  @ary = DBI->data_sources($driver, \%attr);

Returns a list of all data sources (databases) available via the named
driver.  If C<$driver> is empty or C<undef>, then the value of the
C<DBI_DRIVER> environment variable is used.

The driver will be loaded if it hasn't been already. Note that if the
driver loading fails then it I<dies> with an error message that
includes the string "C<install_driver>" and the underlying problem.

Data sources are returned in a form suitable for passing to the
L</connect> method (that is, they will include the "C<dbi:$driver:>" prefix).

Note that many drivers have no way of knowing what data sources might
be available for it. These drivers return an empty or incomplete list
or may require driver-specific attributes, such as a connected database
handle, to be supplied.


=item C<trace>

  DBI->trace($trace_level)
  DBI->trace($trace_level, $trace_filename)

DBI trace information can be enabled for all handles using the C<trace>
DBI class method. To enable trace information for a specific handle, use
the similar C<$h->E<gt>C<trace> method described elsewhere.

Trace levels are as follows:

  0 - Trace disabled.
  1 - Trace DBI method calls returning with results or errors.
  2 - Trace method entry with parameters and returning with results.
  3 - As above, adding some high-level information from the driver
      and some internal information from the DBI.
  4 - As above, adding more detailed information from the driver.
      Also includes DBI mutex information when using threaded Perl.
  5 and above - As above but with more and more obscure information.

Trace level 1 is best for a simple overview of what's happening.
Trace level 2 is a good choice for general purpose tracing.  Levels 3
and above (up to 9) are best reserved for investigating a
specific problem, when you need to see "inside" the driver and DBI.

The trace output is detailed and typically very useful. Much of the
trace output is formatted using the L</neat> function, so strings
in the trace output may be edited and truncated.

Initially trace output is written to C<STDERR>.  If C<$trace_filename> is
specified and can be opened in append mode then all trace
output (including that from other handles) is redirected to that file.
A warning is generated is the file can't be opened.
Further calls to C<trace> without a C<$trace_filename> do not alter where
the trace output is sent. If C<$trace_filename> is undefined, then
trace output is sent to C<STDERR> and the previous trace file is closed.
The C<trace> method returns the I<previous> tracelevel.

See also the C<$h->E<gt>C<trace> and C<$h->E<gt>C<trace_msg> methods and the
L</DEBUGGING> section
for information about the C<DBI_TRACE> environment variable.


=back


=head2 DBI Utility Functions

In addition to the methods listed in the previous section,
the DBI package also provides these utility functions:

=over 4

=item C<neat>

  $str = DBI::neat($value, $maxlen);

Return a string containing a neat (and tidy) representation of the
supplied value.

Strings will be quoted, although internal quotes will I<not> be escaped.
Values known to be numeric will be unquoted. Undefined (NULL) values
will be shown as C<undef> (without quotes). Unprintable characters will
be replaced by dot (.).

For result strings longer than C<$maxlen> the result string will be
truncated to C<$maxlen-4> and "C<...'>" will be appended.  If C<$maxlen> is 0
or C<undef>, it defaults to C<$DBI::neat_maxlen> which, in turn, defaults to 400.

This function is designed to format values for human consumption.
It is used internally by the DBI for L</trace> output. It should
typically I<not> be used for formatting values for database use.
(See also L</quote>.)

=item C<neat_list>

  $str = DBI::neat_list(\@listref, $maxlen, $field_sep);

Calls C<DBI::neat> on each element of the list and returns a string
containing the results joined with C<$field_sep>. C<$field_sep> defaults
to C<", ">.

=item C<looks_like_number>

  @bool = DBI::looks_like_number(@array);

Returns true for each element that looks like a number.
Returns false for each element that does not look like a number.
Returns C<undef> for each element that is undefined or empty.

=back


=head2 DBI Dynamic Attributes

Dynamic attributes are always associated with the I<last handle used>
(that handle is represented by C<$h> in the descriptions below).

Where an attribute is equivalent to a method call, then refer to
the method call for all related documentation.

Warning: these attributes are provided as a convenience but they
do have limitations. Specifically, they have a short lifespan:
because they are associated with
the last handle used, they should only be used I<immediately> after
calling the method that "sets" them.
If in any doubt, use the corresponding method call.

=over 4

=item C<$DBI::err>

Equivalent to C<$h->E<gt>C<err>.

=item C<$DBI::errstr>

Equivalent to C<$h->E<gt>C<errstr>.

=item C<$DBI::state>

Equivalent to C<$h->E<gt>C<state>.

=item C<$DBI::rows>

Equivalent to C<$h->E<gt>C<rows>. Please refer to the documentation 
for the L</rows> method.

=item C<$DBI::lasth>

Returns the DBI object handle used for the most recent DBI method call.
If the last DBI method call was a DESTROY then $DBI::lasth will return
the handle of the parent of the destroyed handle, if there is one.

=back


=head1 METHODS COMMON TO ALL HANDLES

The following methods can be used by all types of DBI handles.

=over 4

=item C<err>

  $rv = $h->err;

Returns the I<native> database engine error code from the last driver
method called. The code is typically an integer but you should not
assume that.

The DBI resets $h->err to undef before most DBI method calls, so the
value only has a short lifespan. Also, most drivers share the same
error variables across all their handles, so calling a method on
one handle will typically reset the error on all the other handles
that are children of that driver.

If you need to test for individual errors I<and> have your program be
portable to different database engines, then you'll need to determine
what the corresponding error codes are for all those engines and test for
all of them.

=item C<errstr>

  $str = $h->errstr;

Returns the native database engine error message from the last driver
method called. This has the same lifespan issues as the L</err> method
described above.

=item C<state>

  $str = $h->state;

Returns an error code in the standard SQLSTATE five character format.
Note that the specific success code C<00000> is translated to 'C<>'
(false). If the driver does not support SQLSTATE (and most don't),
then state will return C<S1000> (General Error) for all errors.

The driver is free to return any value via C<state>, e.g., warning
codes, even if it has not declared an error by returning a true value
via the L</err> method described above.

=item C<trace>

  $h->trace($trace_level);
  $h->trace($trace_level, $trace_filename);

DBI trace information can be enabled for a specific handle (and any
future children of that handle) by setting the trace level using the
C<trace> method.

Trace level 1 is best for a simple overview of what's happening.
Trace level 2 is a good choice for general purpose tracing.  Levels 3
and above (up to 9) are best reserved for investigating a
specific problem, when you need to see "inside" the driver and DBI.
Set C<$trace_level> to 0 to disable the trace.

The trace output is detailed and typically very useful. Much of the
trace output is formatted using the L</neat> function, so strings
in the trace output may be edited and truncated.

Initially, trace output is written to C<STDERR>.  If C<$trace_filename> is
specified, then the file is opened in append mode and I<all> trace
output (including that from other handles) is redirected to that file.
Further calls to trace without a C<$trace_filename> do not alter where
the trace output is sent. If C<$trace_filename> is undefined, then
trace output is sent to C<STDERR> and the previous trace file is closed.

See also the C<DBI->E<gt>C<trace> method and L</DEBUGGING> for information
about the C<DBI_TRACE> environment variable.


=item C<trace_msg>

  $h->trace_msg($message_text);
  $h->trace_msg($message_text, $min_level);

Writes C<$message_text> to the trace file if trace is enabled for C<$h> or
for the DBI as a whole. Can also be called as C<DBI->E<gt>C<trace_msg($msg)>.
See L</trace>.

If C<$min_level> is defined, then the message is output only if the trace
level is equal to or greater than that level. C<$min_level> defaults to 1.


=item C<func>

  $h->func(@func_arguments, $func_name);

The C<func> method can be used to call private non-standard and
non-portable methods implemented by the driver. Note that the function
name is given as the last argument.

This method is not directly related to calling stored procedures.
Calling stored procedures is currently not defined by the DBI.
Some drivers, such as DBD::Oracle, support it in non-portable ways.
See driver documentation for more details.

=back


=head1 ATTRIBUTES COMMON TO ALL HANDLES

These attributes are common to all types of DBI handles.

Some attributes are inherited by child handles. That is, the value
of an inherited attribute in a newly created statement handle is the
same as the value in the parent database handle. Changes to attributes
in the new statement handle do not affect the parent database handle
and changes to the database handle do not affect existing statement
handles, only future ones.

Attempting to set or get the value of an unknown attribute is fatal,
except for private driver specific attributes (which all have names
starting with a lowercase letter).

Example:

  $h->{AttributeName} = ...;	# set/write
  ... = $h->{AttributeName};	# get/read

=over 4

=item C<Warn> (boolean, inherited)

Enables useful warnings for certain bad practices. Enabled by default. Some
emulation layers, especially those for Perl 4 interfaces, disable warnings.
Since warnings are generated using the Perl C<warn> function, they can be
intercepted using the Perl C<$SIG{__WARN__}> hook.

=item C<Active> (boolean, read-only)

True if the handle object is "active". This is rarely used in
applications. The exact meaning of active is somewhat vague at the
moment. For a database handle it typically means that the handle is
connected to a database (C<$dbh->E<gt>C<disconnect> sets C<Active> off).  For
a statement handle it typically means that the handle is a C<SELECT>
that may have more data to fetch. (Fetching all the data or calling C<$sth->E<gt>C<finish>
sets C<Active> off.)

=item C<Kids> (integer, read-only)

For a driver handle, C<Kids> is the number of currently existing database
handles that were created from that driver handle.  For a database
handle, C<Kids> is the number of currently existing statement handles that
were created from that database handle.

=item C<ActiveKids> (integer, read-only)

Like C<Kids>, but only counting those that are C<Active> (as above).

=item C<CachedKids> (hash ref)

For a database handle, returns a reference to the cache (hash) of
statement handles created by the L</prepare_cached> method.  For a
driver handle, returns a reference to the cache (hash) of
database handles created by the L</connect_cached> method.

=item C<CompatMode> (boolean, inherited)

Used by emulation layers (such as Oraperl) to enable compatible behavior
in the underlying driver (e.g., DBD::Oracle) for this handle. Not normally
set by application code.

=item C<InactiveDestroy> (boolean)

This attribute can be used to disable the I<database engine> related
effect of DESTROYing a handle (which would normally close a prepared
statement or disconnect from the database etc).

For a database handle, this attribute does not disable an I<explicit>
call to the disconnect method, only the implicit call from DESTROY.

This attribute is specifically designed for use in Unix applications
that "fork" child processes. Either the parent or the child process,
but not both, should set C<InactiveDestroy> on all their shared handles.
Note that some databases, including Oracle, don't support passing a
database connection across a fork.

=item C<PrintError> (boolean, inherited)

This attribute can be used to force errors to generate warnings (using
C<warn>) in addition to returning error codes in the normal way.  When set
"on", any method which results in an error occuring will cause the DBI to
effectively do a C<warn("$class $method failed: $DBI::errstr")> where C<$class>
is the driver class and C<$method> is the name of the method which failed. E.g.,

  DBD::Oracle::db prepare failed: ... error text here ...

By default, C<DBI->E<gt>C<connect> sets C<PrintError> "on".

If desired, the warnings can be caught and processed using a C<$SIG{__WARN__}>
handler or modules like CGI::Carp and CGI::ErrorWrap.

=item C<RaiseError> (boolean, inherited)

This attribute can be used to force errors to raise exceptions rather
than simply return error codes in the normal way. It is "off" by default.
When set "on", any method which results in an error will cause
the DBI to effectively do a C<die("$class $method failed: $DBI::errstr")>,
where C<$class> is the driver class and C<$method> is the name of the method
that failed. E.g.,

  DBD::Oracle::db prepare failed: ... error text here ...

If you turn C<RaiseError> on then you'd normally turn C<PrintError> off.
If C<PrintError> is also on, then the C<PrintError> is done first (naturally).

Typically C<RaiseError> is used in conjunction with C<eval { ... }>
to catch the exception that's been thrown and followed by an
C<if ($@) { ... }> block to handle the caught exception. In that eval
block the $DBI::lasth variable can be useful for diagnosis and reporting.
For example, $DBI::lasth->{Type} and $DBI::lasth->{Statement}.

If you want to temporarily turn C<RaiseError> off (inside a library function
that is likely to fail, for example), the recommended way is like this:

  {
    local $h->{RaiseError};  # localize and turn off for this block
    ...
  }

The original value will automatically and reliably be restored by Perl,
regardless of how the block is exited.
The same logic applies to other attributes, including C<PrintError>.

Sadly, this doesn't work for Perl versions up to and including 5.004_04.
Even more sadly, for Perl 5.5 and 5.6.0 it does work but leaks memory!
For backwards compatibility, you could just use C<eval { ... }> instead.


=item C<ShowErrorStatement> (boolean, inherited) I<NEW>

This attribute can be used to cause the relevant Statement text to be
appended to the error messages generated by the C<RaiseError> and
C<PrintError> attributes. Only applies to errors on statement handles
plus the prepare() and do() database handle methods.
(The exact format of the appended text is subject to change.)


=item C<ChopBlanks> (boolean, inherited)

This attribute can be used to control the trimming of trailing space
characters from fixed width character (CHAR) fields. No other field
types are affected, even where field values have trailing spaces.

The default is false (although it is possible that the default may change).
Applications that need specific behavior should set the attribute as
needed. Emulation interfaces should set the attribute to match the
behavior of the interface they are emulating.

Drivers are not required to support this attribute, but any driver which
does not support it must arrange to return C<undef> as the attribute value.


=item C<LongReadLen> (unsigned integer, inherited)

This attribute may be used to control the maximum length of long fields
("blob", "memo", etc.) which the driver will read from the
database automatically when it fetches each row of data.  The
C<LongReadLen> attribute only relates to fetching and reading long values; it
is not involved in inserting or updating them.

A value of 0 means not to automatically fetch any long data. (C<fetch>
should return C<undef> for long fields when C<LongReadLen> is 0.)

The default is typically 0 (zero) bytes but may vary between drivers.
Applications fetching long fields should set this value to slightly
larger than the longest long field value to be fetched.

Some databases return some long types encoded as pairs of hex digits.
For these types, C<LongReadLen> relates to the underlying data length and not the
doubled-up length of the encoded string.

Changing the value of C<LongReadLen> for a statement handle after it
has been C<prepare>'d will typically have no effect, so it's common to
set C<LongReadLen> on the C<$dbh> before calling C<prepare>.

Note that the value used here has a direct effect on the memory used
by the application, so don't be too generous.

See L</LongTruncOk> for more information on truncation behavior.

=item C<LongTruncOk> (boolean, inherited)

This attribute may be used to control the effect of fetching a long
field value which has been truncated (typically because it's longer
than the value of the C<LongReadLen> attribute).

By default, C<LongTruncOk> is false and so fetching a long value that
needs to be truncated will cause the fetch to fail.
(Applications should always be sure to
check for errors after a fetch loop in case an error, such as a divide
by zero or long field truncation, caused the fetch to terminate
prematurely.)

If a fetch fails due to a long field truncation when C<LongTruncOk> is
false, many drivers will allow you to continue fetching further rows.

See also L</LongReadLen>.

=item C<Taint> (boolean, inherited)

If this attribute is set to a true value I<and> Perl is running in
taint mode (e.g., started with the C<-T> option), then all data
fetched from the database is tainted, and the arguments to most DBI
method calls are checked for being tainted. I<This may change.>

The attribute defaults to off, even if Perl is in taint mode.
See L<perlsec> for more about taint mode.  If Perl is not
running in taint mode, this attribute has no effect.

When fetching data that you trust you can turn off the Taint attribute,
for that statement handle, for the duration of the fetch loop.

Currently only fetched data is tainted. It is possible that the results
of other DBI method calls, and the value of fetched attributes, may
also be tainted in future versions. That change may well break your
applications unless you take great care now. If you use DBI Taint mode,
please report your experience and any suggestions for changes.


=item C<private_*>

The DBI provides a way to store extra information in a DBI handle as
"private" attributes. The DBI will allow you to store and retreive any
attribute which has a name starting with "C<private_>". It is strongly
recommended that you use just I<one> private attribute (e.g., use a
hash ref) and give it a long and unambiguous name that includes the
module or application name that the attribute relates to (e.g.,
"C<private_YourFullModuleName_thingy>").

=back


=head1 DBI DATABASE HANDLE OBJECTS

This section covers the methods and attributes associated with
database handles.

=head2 Database Handle Methods

The following methods are specified for DBI database handles:

=over 4

=item C<do>

  $rc  = $dbh->do($statement)           or die $dbh->errstr;
  $rc  = $dbh->do($statement, \%attr)   or die $dbh->errstr;
  $rv  = $dbh->do($statement, \%attr, @bind_values) or ...

Prepare and execute a single statement. Returns the number of rows
affected or C<undef> on error. A return value of C<-1> means the 
number of rows is not known or is not available.

This method is typically most useful for I<non>-C<SELECT> statements that
either cannot be prepared in advance (due to a limitation of the
driver) or do not need to be executed repeatedly. It should not
be used for C<SELECT> statements because it does not return a statement
handle (so you can't fetch any data).

The default C<do> method is logically similar to:

  sub do {
      my($dbh, $statement, $attr, @bind_values) = @_;
      my $sth = $dbh->prepare($statement, $attr) or return undef;
      $sth->execute(@bind_values) or return undef;
      my $rows = $sth->rows;
      ($rows == 0) ? "0E0" : $rows; # always return true if no error
  }

For example:

  my $rows_deleted = $dbh->do(q{
      DELETE FROM table
      WHERE status = ?
  }, undef, 'DONE') or die $dbh->errstr;

Using placeholders and C<@bind_values> with the C<do> method can be
useful because it avoids the need to correctly quote any variables
in the C<$statement>. But if you'll be executing the statement many
times then it's more efficient to C<prepare> it once and call
C<execute> many times instead.

The C<q{...}> style quoting used in this example avoids clashing with
quotes that may be used in the SQL statement. Use the double-quote-like
C<qq{...}> operator if you want to interpolate variables into the string.
See L<perlop/"Quote and Quote-like Operators"> for more details.

=item C<selectrow_array>

  @row_ary = $dbh->selectrow_array($statement);
  @row_ary = $dbh->selectrow_array($statement, \%attr);
  @row_ary = $dbh->selectrow_array($statement, \%attr, @bind_values);

This utility method combines L</prepare>, L</execute> and
L</fetchrow_array> into a single call. If called in a list context, it
returns the first row of data from the statement. If called in a scalar
context, it returns the first field of the first row. The C<$statement>
parameter can be a previously prepared statement handle, in which case
the C<prepare> is skipped.

If any method fails, and L</RaiseError> is not set, C<selectrow_array>
will return an empty list.

In a scalar context, C<selectrow_array> returns the value of the first
field. An C<undef> is returned if there are no matching rows or an error
occurred. Since that C<undef> can't be distinguished from an C<undef> returned
because the first field value was NULL, calling C<selectrow_array> in
a scalar context should be used with caution.


=item C<selectall_arrayref>

  $ary_ref = $dbh->selectall_arrayref($statement);
  $ary_ref = $dbh->selectall_arrayref($statement, \%attr);
  $ary_ref = $dbh->selectall_arrayref($statement, \%attr, @bind_values);

This utility method combines L</prepare>, L</execute> and
L</fetchall_arrayref> into a single call. It returns a reference to an
array containing a reference to an array for each row of data fetched.

The C<$statement> parameter can be a previously prepared statement handle,
in which case the C<prepare> is skipped. This is recommended if the
statement is going to be executed many times.

If L</RaiseError> is not set and any method except C<fetchall_arrayref>
fails then C<selectall_arrayref> will return C<undef>; if
C<fetchall_arrayref> fails then it will return with whatever data it
has been fetched thus far. $DBI::err should be checked to catch that.


=item C<selectall_hashref>

  $ary_ref = $dbh->selectall_hashref($statement);
  $ary_ref = $dbh->selectall_hashref($statement, \%attr);
  $ary_ref = $dbh->selectall_hashref($statement, \%attr, @bind_values);

This utility method combines L</prepare>, L</execute> and
L</fetchrow_hashref> into a single call. It returns a reference to an
array containing, for each row of data fetched, a reference to a hash
containing field name and value pairs for that row.

The C<$statement> parameter can be a previously prepared statement handle,
in which case the C<prepare> is skipped. This is recommended if the
statement is going to be executed many times.

If any method except C<fetchrow_hashref> fails, and L</RaiseError> is not set,
C<selectall_hashref> will return C<undef>.  If C<fetchrow_hashref> fails and
L</RaiseError> is not set, then it will return with whatever data it
has fetched thus far. $DBI::err should be checked to catch that.


=item C<selectcol_arrayref>

  $ary_ref = $dbh->selectcol_arrayref($statement);
  $ary_ref = $dbh->selectcol_arrayref($statement, \%attr);
  $ary_ref = $dbh->selectcol_arrayref($statement, \%attr, @bind_values);

This utility method combines L</prepare>, L</execute>, and fetching one
column from all the rows, into a single call. It returns a reference to
an array containing the values of the first column from each row.

The C<$statement> parameter can be a previously prepared statement handle,
in which case the C<prepare> is skipped. This is recommended if the
statement is going to be executed many times.

If any method except C<fetch> fails, and L</RaiseError> is not set,
C<selectcol_arrayref> will return C<undef>.  If C<fetch> fails and
L</RaiseError> is not set, then it will return with whatever data it
has fetched thus far. $DBI::err should be checked to catch that.


=item C<prepare>

  $sth = $dbh->prepare($statement)          or die $dbh->errstr;
  $sth = $dbh->prepare($statement, \%attr)  or die $dbh->errstr;

Prepares a single statement for later execution by the database
engine and returns a reference to a statement handle object.

The returned statement handle can be used to get attributes of the
statement and invoke the L</execute> method. See L</Statement Handle Methods>.

Drivers for engines without the concept of preparing a
statement will typically just store the statement in the returned
handle and process it when C<$sth->E<gt>C<execute> is called. Such drivers are
unlikely to give much useful information about the
statement, such as C<$sth->E<gt>C<{NUM_OF_FIELDS}>, until after C<$sth->E<gt>C<execute>
has been called. Portable applications should take this into account.

In general, DBI drivers do not parse the contents of the statement
(other than simply counting any L</Placeholders>). The statement is
passed directly to the database engine, sometimes known as pass-thru
mode. This has advantages and disadvantages. On the plus side, you can
access all the functionality of the engine being used. On the downside,
you're limited if you're using a simple engine, and you need to take extra care if
writing applications intended to be portable between engines.

Portable applications should not assume that a new statement can be
prepared and/or executed while still fetching results from a previous
statement.

Some command-line SQL tools use statement terminators, like a semicolon,
to indicate the end of a statement. Such terminators should not normally
be used with the DBI.


=item C<prepare_cached>

  $sth = $dbh->prepare_cached($statement)
  $sth = $dbh->prepare_cached($statement, \%attr)
  $sth = $dbh->prepare_cached($statement, \%attr, $allow_active)

Like L</prepare> except that the statement handle returned will be
stored in a hash associated with the C<$dbh>. If another call is made to
C<prepare_cached> with the same C<$statement> and C<%attr> values, then the
corresponding cached C<$sth> will be returned without contacting the
database server.

This caching can be useful in some applications, but it can also cause
problems and should be used with care. If the cached C<$sth> being
returned is active (i.e., is a C<SELECT> that may still have data to be
fetched) then a warning will be generated and C<finish> will be called
for you.  The warning can be suppressed by setting C<$allow_active> to
true.  The cache can be accessed (and cleared) via the L</CachedKids>
attribute.

Here's an example of one possible use of C<prepare_cached>:

  while ( ($field, $value) = each %search_fields ) {
      push @sql,   "$field = ?";
      push @values, $value;
  }
  $qualifier = "";
  $qualifier = "where ".join(" and ", @sql) if @sql;
  $sth = $dbh->prepare_cached("SELECT * FROM table $qualifier");
  $sth->execute(@values);


=item C<commit>

  $rc  = $dbh->commit     or die $dbh->errstr;

Commit (make permanent) the most recent series of database changes
if the database supports transactions and AutoCommit is off.

If C<AutoCommit> is on, then calling
C<commit> will issue a "commit ineffective with AutoCommit" warning.

See also L</Transactions> in the L</FURTHER INFORMATION> section below.

=item C<rollback>

  $rc  = $dbh->rollback   or die $dbh->errstr;

Rollback (undo) the most recent series of uncommitted database
changes if the database supports transactions and AutoCommit is off.

If C<AutoCommit> is on, then calling
C<rollback> will issue a "rollback ineffective with AutoCommit" warning.

See also L</Transactions> in the L</FURTHER INFORMATION> section below.


=item C<disconnect>

  $rc = $dbh->disconnect  or warn $dbh->errstr;

Disconnects the database from the database handle. C<disconnect> is typically only used
before exiting the program. The handle is of little use after disconnecting.

The transaction behavior of the C<disconnect> method is, sadly,
undefined.  Some database systems (such as Oracle and Ingres) will
automatically commit any outstanding changes, but others (such as
Informix) will rollback any outstanding changes.  Applications not
using C<AutoCommit> should explicitly call C<commit> or C<rollback> before
calling C<disconnect>.

The database is automatically disconnected by the C<DESTROY> method if
still connected when there are no longer any references to the handle.
The C<DESTROY> method for each driver should implicitly call C<rollback> to
undo any uncommitted changes. This is vital behavior to ensure that
incomplete transactions don't get committed simply because Perl calls
C<DESTROY> on every object before exiting. Also, do not rely on the order
of object destruction during "global destruction", as it is undefined.

Generally, if you want your changes to be commited or rolled back when
you disconnect, then you should explicitly call L</commit> or L</rollback>
before disconnecting.

If you disconnect from a database while you still have active statement
handles, you will get a warning. The statement handles should either be
cleared (destroyed) before disconnecting, or the C<finish> method 
should be called on
each one.


=item C<ping>

  $rc = $dbh->ping;

Attempts to determine, in a reasonably efficient way, if the database
server is still running and the connection to it is still working.
Individual drivers should implement this function in the most suitable
manner for their database engine.

The current I<default> implementation always returns true without
actually doing anything. Actually, it returns "C<0 but true>" which is
true but zero. That way you can tell if the return value is genuine or
just the default. Drivers should override this method with one that
does the right thing for their type of database.

Few applications would have direct use for this method. See the specialized
Apache::DBI module for one example usage.


=item C<table_info> I<NEW>

B<Warning:> This method is experimental and may change.

  $sth = $dbh->table_info;
  $sth = $dbh->table_info( \%attr );

Returns an active statement handle that can be used to fetch
information about tables and views that exist in the database.

The following attributes (all or separate) may be used as selection criteria:

  %attr = (
       TABLE_CAT   => $CatVal    # String value of the catalog name
     , TABLE_SCHEM => $SchVal    # String value of the schema name
     , TABLE_NAME  => $TblVal    # String value of the table name
     , TABLE_TYPE  => $TypVal    # String value of the table type(s)
  );

Note: The support for the selection criteria is driver specific. If the
driver doesn't support one or more then them then you'll get back more
than you asked for and can do the filtering yourself.

The arguments TABLE_CAT, TABLE_SCHEM and TABLE_NAME may accept search
patterns according to the database/driver, for example:

  $sth = $dbh->table_info( { TABLE_NAME  => '%TAB%'} );

The value of TABLE_TYPE is a comma-separated list of one or more types
of tables to be returned in the result set. Each value may optionally be
quoted, e.g.:

  $sth = $dbh->table_info( { TABLE_TYPE  => "TABLE" } );
  $sth = $dbh->table_info( { TABLE_TYPE  => "'TABLE', 'VIEW'" } );

In addition the following special cases may also be supported by some drivers:

=over 4

=item *
If the value of TABLE_CAT is '%' and TABLE_SCHEM and TABLE_NAME name
are empty strings, the result set contains a list of catalog names.
For example:

  $sth = $dbh->table_info({ TABLE_CAT=>'%', TABLE_SCHEM=>'', TABLE_NAME=>'' });

=item *
If the value of TABLE_SCHEM is '%' and TABLE_CAT and TABLE_NAME are
empty strings, the result set contains a list of schema names.

=item *
If the value of TABLE_TYPE is '%' and TABLE_CAT, TABLE_SCHEM, and
TABLE_NAME are all empty strings, the result set contains a list of
table types.

=back

The statement handle returned has at least the following fields in the
order show below. Other fields, after these, may also be present.

B<TABLE_CAT>: Table catalog identifier. This field is NULL (C<undef>) if not
applicable to the data source, which is usually the case. This field
is empty if not applicable to the table.

B<TABLE_SCHEM>: The name of the schema containing the TABLE_NAME value.
This field is NULL (C<undef>) if not applicable to data source, and
empty if not applicable to the table.

B<TABLE_NAME>: Name of the table (or view, synonym, etc).

B<TABLE_TYPE>: One of the following: "TABLE", "VIEW", "SYSTEM TABLE",
"GLOBAL TEMPORARY", "LOCAL TEMPORARY", "ALIAS", "SYNONYM" or a type
identifier that is specific to the data
source.

B<REMARKS>: A description of the table. May be NULL (C<undef>).

Note that C<table_info> might not return records for all tables.
Applications can use any valid table regardless of whether it's
returned by C<table_info>.  See also L</tables>.

For more detailed information about the fields and their meanings,
you can refer to:

  http://msdn.microsoft.com/library/psdk/dasdk/odch6wqb.htm

If that URL ceases to work then use the MSDN search facility at:

  http://search.microsoft.com/us/dev/

and search for C<SQLTables returns> using the exact phrase option.
The link you want will probably just be called C<SQLTables> and will
be part of the Data Access SDK.

See also page 306 of the (very large) SQL/CLI specification:

  http://www.jtc1sc32.org/sc32/jtc1sc32.nsf/Attachments/DF86E81BE70151D58525699800643F56/$FILE/32N0595T.PDF

=item C<tables> I<NEW>

B<Warning:> This method is experimental and may change.

  @names = $dbh->tables;
  @names = $dbh->tables( \%attr );

Returns a list of table and view names, possibly including a schema prefix.
This list should include all
tables that can be used in a C<SELECT> statement without further
qualification.

Note that C<table_info> might not return records for all tables.
Applications can use any valid table regardless of whether it's
returned by tables.  See also L</table_info>.


=item C<type_info_all> I<NEW>

B<Warning:> This method is experimental and may change.

  $type_info_all = $dbh->type_info_all;

Returns a reference to an array which holds information about each data
type variant supported by the database and driver. The array and its
contents should be treated as read-only.

The first item is a reference to an 'index' hash of C<Name =>E<gt> C<Index> pairs.
The items following that are references to arrays, one per supported data
type variant. The leading index hash defines the names and order of the
fields within the arrays that follow it.
For example:

  $type_info_all = [
    {   TYPE_NAME         => 0,
	DATA_TYPE         => 1,
	COLUMN_SIZE       => 2,     # was PRECISION originally
	LITERAL_PREFIX    => 3,
	LITERAL_SUFFIX    => 4,
	CREATE_PARAMS     => 5,
	NULLABLE          => 6,
	CASE_SENSITIVE    => 7,
	SEARCHABLE        => 8,
	UNSIGNED_ATTRIBUTE=> 9,
	FIXED_PREC_SCALE  => 10,    # was MONEY originally
	AUTO_UNIQUE_VALUE => 11,    # was AUTO_INCREMENT originally
	LOCAL_TYPE_NAME   => 12,
	MINIMUM_SCALE     => 13,
	MAXIMUM_SCALE     => 14,
	NUM_PREC_RADIX    => 15,
    },
    [ 'VARCHAR', SQL_VARCHAR,
	undef, "'","'", undef,0, 1,1,0,0,0,undef,1,255, undef
    ],
    [ 'INTEGER', SQL_INTEGER,
	undef,  "", "", undef,0, 0,1,0,0,0,undef,0,  0, 10
    ],
  ];

Note that more than one row may have the same value in the C<DATA_TYPE>
field if there are different ways to spell the type name and/or there
are variants of the type with different attributes (e.g., with and
without C<AUTO_UNIQUE_VALUE> set, with and without C<UNSIGNED_ATTRIBUTE>, etc).

The rows are ordered by C<DATA_TYPE> first and then by how closely each
type maps to the corresponding ODBC SQL data type, closest first.

The meaning of the fields is described in the documentation for
the L</type_info> method. The index values shown above (e.g.,
C<NULLABLE =>E<gt> C<6>) are for illustration only. Drivers may define the
fields with a different order.

This method is not normally used directly. The L</type_info> method
provides a more useful interface to the data.

Even though an 'index' hash is provided, all the field names in the
index hash defined above will always have the index values defined
above.  This is defined behaviour so that you don't need to rely on the
index hash, which is handy because the lettercase of the keys is not
defined. It is usually uppercase, as show here, but drivers are free to
return names with any lettercase. Drivers are also free to return extra
driver-specific columns of information - though it's recommended that
they start at column index 50 to leave room for expansion of the
DBI/ODBC specification.


=item C<type_info> I<NEW>

B<Warning:> This method is experimental and may change.

  @type_info = $dbh->type_info($data_type);

Returns a list of hash references holding information about one or more
variants of C<$data_type>. The list is ordered by C<DATA_TYPE> first and
then by how closely each type maps to the corresponding ODBC SQL data
type, closest first.  If called in a scalar context then only the first
(best) element is returned.

If C<$data_type> is undefined or C<SQL_ALL_TYPES>, then the list will
contain hashes for all data type variants supported by the database and driver.

If C<$data_type> is an array reference then C<type_info> returns the
information for the I<first> type in the array that has any matches.

The keys of the hash follow the same letter case conventions as the
rest of the DBI (see L</Naming Conventions and Name Space>). The
following items should exist:

=over 4

=item TYPE_NAME (string)

Data type name for use in CREATE TABLE statements etc.

=item DATA_TYPE (integer)

SQL data type number.

=item COLUMN_SIZE (integer)

For numeric types, this is either the total number of digits (if the
NUM_PREC_RADIX value is 10) or the total number of bits allowed in the
column (if NUM_PREC_RADIX is 2).

For string types, this is the maximum size of the string in bytes.

For date and interval types, this is the maximum number of characters
needed to display the value.

=item LITERAL_PREFIX (string)

Characters used to prefix a literal. A typical prefix is "C<'>" for characters,
or possibly "C<0x>" for binary values passed as hexadecimal.  NULL (C<undef>) is
returned for data types for which this is not applicable.


=item LITERAL_SUFFIX (string)

Characters used to suffix a literal. Typically "C<'>" for characters.
NULL (C<undef>) is returned for data types where this is not applicable.

=item CREATE_PARAMS (string)

Parameter names for data type definition. For example, C<CREATE_PARAMS> for a
C<DECIMAL> would be "C<precision,scale>" if the DECIMAL type should be
declared as C<DECIMAL(>I<precision,scale>C<)> where I<precision> and I<scale>
are integer values.  For a C<VARCHAR> it would be "C<max length>".
NULL (C<undef>) is returned for data types for which this is not applicable.

=item NULLABLE (integer)

Indicates whether the data type accepts a NULL value:
C<0> or an empty string = no, C<1> = yes, C<2> = unknown.

=item CASE_SENSITIVE (boolean)

Indicates whether the data type is case sensitive in collations and
comparisons.

=item SEARCHABLE (integer)

Indicates how the data type can be used in a WHERE clause, as
follows:

  0 - Cannot be used in a WHERE clause
  1 - Only with a LIKE predicate
  2 - All comparison operators except LIKE
  3 - Can be used in a WHERE clause with any comparison operator

=item UNSIGNED_ATTRIBUTE (boolean)

Indicates whether the data type is unsigned.  NULL (C<undef>) is returned
for data types for which this is not applicable.

=item FIXED_PREC_SCALE (boolean)

Indicates whether the data type always has the same precision and scale
(such as a money type).  NULL (C<undef>) is returned for data types 
for which
this is not applicable.

=item AUTO_UNIQUE_VALUE (boolean)

Indicates whether a column of this data type is automatically set to a
unique value whenever a new row is inserted.  NULL (C<undef>) is returned
for data types for which this is not applicable.

=item LOCAL_TYPE_NAME (string)

Localized version of the C<TYPE_NAME> for use in dialog with users.
NULL (C<undef>) is returned if a localized name is not available (in which
case C<TYPE_NAME> should be used).

=item MINIMUM_SCALE (integer)

The minimum scale of the data type. If a data type has a fixed scale,
then C<MAXIMUM_SCALE> holds the same value.  NULL (C<undef>) is returned for
data types for which this is not applicable.

=item MAXIMUM_SCALE (integer)

The maximum scale of the data type. If a data type has a fixed scale,
then C<MINIMUM_SCALE> holds the same value.  NULL (C<undef>) is returned for
data types for which this is not applicable.

=item SQL_DATA_TYPE (integer)

This column is the same as the C<DATA_TYPE> column, except for interval
and datetime data types.  For interval and datetime data types, the
C<SQL_DATA_TYPE> field will return C<SQL_INTERVAL> or C<SQL_DATETIME>, and the
C<SQL_DATETIME_SUB> field below will return the subcode for the specific
interval or datetime data type. If this field is NULL, then the driver
does not support or report on interval or date subtypes.

=item SQL_DATETIME_SUB (integer)

For interval or datetime data types, where the C<SQL_DATA_TYPE> field
above is C<SQL_INTERVAL> or C<SQL_DATETIME>, this field will hold the subcode
for the specific interval or datetime data type. Otherwise it will be
NULL (C<undef>).

=item NUM_PREC_RADIX (integer)

The radix value of the data type. For approximate numeric types, 
C<NUM_PREC_RADIX>
contains the value 2 and C<COLUMN_SIZE> holds the number of bits. For
exact numeric types, C<NUM_PREC_RADIX> contains the value 10 and C<COLUMN_SIZE> holds
the number of decimal digits. NULL (C<undef>) is returned either for data types
for which this is not applicable or if the driver cannot report this information.

=item INTERVAL_PRECISION (integer)

The interval leading precision for interval types. NULL is returned
either for data types for which this is not applicable or if the driver
cannot report this information.

=back

For example, to find the type name for the fields in a select statement
you can do:

  @names = map { scalar $dbh->type_info($_)->{TYPE_NAME} } @{ $sth->{TYPE} }

Since DBI and ODBC drivers vary in how they map their types into the
ISO standard types you may need to search for more than one type.
Here's an example looking for a usable type to store a date:

  $my_date_type = $dbh->type_info( [ SQL_DATE, SQL_TIMESTAMP ] );

Similarly, to more reliably find a type to store small integers, you could
use a list starting with C<SQL_SMALLINT>, C<SQL_INTEGER>, C<SQL_DECIMAL>, etc.

For more detailed information about these fields and their meanings, you
can refer to:

  http://msdn.microsoft.com/library/psdk/dasdk/odch6yy7.htm

If that URL ceases to work then use the MSDN search facility at 
 
    http://search.microsoft.com/us/dev/
 
and search the MSDN Library for C<SQLGetTypeInfo returns> using the exact phrase option.
The link you want will probably just be called C<SQLGetTypeInfo> (there
may be more than one). 

The individual data types are currently described here:

  http://msdn.microsoft.com/library/psdk/dasdk/odap8fcj.htm

If that URL ceases to work, or to get more general information, use the
MSDN search facility as described above and search for C<SQL Data Types>.

=item C<quote>

  $sql = $dbh->quote($value);
  $sql = $dbh->quote($value, $data_type);

Quote a string literal for use as a literal value in an SQL statement,
by escaping any special characters (such as quotation marks)
contained within the string and adding the required type of outer
quotation marks.

  $sql = sprintf "SELECT foo FROM bar WHERE baz = %s",
                $dbh->quote("Don't");

For most database types, quote would return C<'Don''t'> (including the
outer quotation marks).

An undefined C<$value> value will be returned as the string C<NULL> (without
quotation marks) to match how NULLs are represented in SQL.

If C<$data_type> is supplied, it is used to try to determine the required
quoting behavior by using the information returned by L</type_info>.
As a special case, the standard numeric types are optimized to return
C<$value> without calling C<type_info>.

Quote will probably I<not> be able to deal with all possible input
(such as binary data or data containing newlines), and is not related in
any way with escaping or quoting shell meta-characters. There is no
need to quote values being used with L</"Placeholders and Bind Values">.

=back


=head2 Database Handle Attributes

This section describes attributes specific to database handles.

Changes to these database handle attributes do not affect any other
existing or future database handles.

Attempting to set or get the value of an unknown attribute is fatal,
except for private driver-specific attributes (which all have names
starting with a lowercase letter).

Example:

  $h->{AutoCommit} = ...;	# set/write
  ... = $h->{AutoCommit};	# get/read

=over 4

=item C<AutoCommit>  (boolean)

If true, then database changes cannot be rolled-back (undone).  If false,
then database changes automatically occur within a "transaction", which
must either be committed or rolled back using the C<commit> or C<rollback>
methods.

Drivers should always default to C<AutoCommit> mode (an unfortunate
choice largely forced on the DBI by ODBC and JDBC conventions.)

Attempting to set C<AutoCommit> to an unsupported value is a fatal error.
This is an important feature of the DBI. Applications that need
full transaction behavior can set C<$dbh->E<gt>C<{AutoCommit} = 0> (or
set C<AutoCommit> to 0 via L</connect>) 
without having to check that the value was assigned successfully.

For the purposes of this description, we can divide databases into three
categories:

  Databases which don't support transactions at all.
  Databases in which a transaction is always active.
  Databases in which a transaction must be explicitly started (C<'BEGIN WORK'>).

B<* Databases which don't support transactions at all>

For these databases, attempting to turn C<AutoCommit> off is a fatal error.
C<commit> and C<rollback> both issue warnings about being ineffective while
C<AutoCommit> is in effect.

B<* Databases in which a transaction is always active>

These are typically mainstream commercial relational databases with
"ANSI standard" transaction behavior.
If C<AutoCommit> is off, then changes to the database won't have any
lasting effect unless L</commit> is called (but see also
L</disconnect>). If L</rollback> is called then any changes since the
last commit are undone.

If C<AutoCommit> is on, then the effect is the same as if the DBI 
called C<commit> automatically after every successful database
operation. In other words, calling C<commit> or C<rollback> explicitly while
C<AutoCommit> is on would be ineffective because the changes would
have already been commited.

Changing C<AutoCommit> from off to on should issue a L</commit> in most drivers.

Changing C<AutoCommit> from on to off should have no immediate effect.

For databases which don't support a specific auto-commit mode, the
driver has to commit each statement automatically using an explicit
C<COMMIT> after it completes successfully (and roll it back using an
explicit C<ROLLBACK> if it fails).  The error information reported to the
application will correspond to the statement which was executed, unless
it succeeded and the commit or rollback failed.

B<* Databases in which a transaction must be explicitly started>

For these databases, the intention is to have them act like databases in
which a transaction is always active (as described above).

To do this, the DBI driver will automatically begin a transaction when
C<AutoCommit> is turned off (from the default "on" state) and will
automatically begin another transaction after a L</commit> or L</rollback>.
In this way, the application does not have to treat these databases as a
special case.

See L</commit>, L</disconnect> and L</Transactions> for other important
notes about transactions.


=item C<Driver>  (handle)

Holds the handle of the parent driver. The only recommended use for this
is to find the name of the driver using:

  $dbh->{Driver}->{Name}


=item C<Name>  (string)

Holds the "name" of the database. Usually (and recommended to be) the
same as the "C<dbi:DriverName:...>" string used to connect to the database,
but with the leading "C<dbi:DriverName:>" removed.


=item C<Statement>  (string, read-only)

Returns the statement string passed to the most recent L</prepare> method
called in this database handle, even if that method failed. This is especially
useful where C<RaiseError> is enabled and the exception handler checks $@
and sees that a 'prepare' method call failed.


=item C<RowCacheSize>  (integer)

A hint to the driver indicating the size of the local row cache that the
application would like the driver to use for future C<SELECT> statements.
If a row cache is not implemented, then setting C<RowCacheSize> is ignored
and getting the value returns C<undef>.

Some C<RowCacheSize> values have special meaning, as follows:

  0 - Automatically determine a reasonable cache size for each C<SELECT>
  1 - Disable the local row cache
 >1 - Cache this many rows
 <0 - Cache as many rows that will fit into this much memory for each C<SELECT>.

Note that large cache sizes may require a very large amount of memory
(I<cached rows * maximum size of row>). Also, a large cache will cause
a longer delay not only for the first fetch, but also whenever the 
cache needs refilling.

See also the L</RowsInCache> statement handle attribute.

=back


=head1 DBI STATEMENT HANDLE OBJECTS

This section lists the methods and attributes associated with DBI
statement handles.

=head2 Statement Handle Methods

The DBI defines the following methods for use on DBI statement handles:

=over 4

=item C<bind_param>

  $rc = $sth->bind_param($p_num, $bind_value)  or die $sth->errstr;
  $rv = $sth->bind_param($p_num, $bind_value, \%attr)     or ...
  $rv = $sth->bind_param($p_num, $bind_value, $bind_type) or ...

The C<bind_param> method can be used to bind a value
with a placeholder embedded in the prepared statement. Placeholders
are indicated with question mark character (C<?>). For example:

  $dbh->{RaiseError} = 1;        # save having to check each method call
  $sth = $dbh->prepare("SELECT name, age FROM people WHERE name LIKE ?");
  $sth->bind_param(1, "John%");  # placeholders are numbered from 1
  $sth->execute;
  DBI::dump_results($sth);

Note that the C<?> is not enclosed in quotation marks, even when the
placeholder represents a string.  Some drivers also allow placeholders
like C<:>I<name> and C<:>I<n> (e.g., C<:1>, C<:2>, and so on)
in addition to C<?>, but their use
is not portable.  Undefined bind values or C<undef> can be used to
indicate null values.

Some drivers do not support placeholders.

With most drivers, placeholders can't be used for any element of a
statement that would prevent the database server from validating the
statement and creating a query execution plan for it. For example:

  "SELECT name, age FROM ?"         # wrong (will probably fail)
  "SELECT name, ?   FROM people"    # wrong (but may not 'fail')

Also, placeholders can only represent single scalar values.
For example, the following
statement won't work as expected for more than one value:

  "SELECT name, age FROM people WHERE name IN (?)"    # wrong

B<Data Types for Placeholders>

The C<\%attr> parameter can be used to hint at the data type the
placeholder should have. Typically, the driver is only interested in
knowing if the placeholder should be bound as a number or a string.

  $sth->bind_param(1, $value, { TYPE => SQL_INTEGER });

As a short-cut for this common case, the data type can be passed
directly, in place of the C<\%attr> hash reference. This example is
equivalent to the one above:

  $sth->bind_param(1, $value, SQL_INTEGER);

The C<TYPE> value indicates the standard (non-driver-specific) type for
this parameter. To specify the driver-specific type, the driver may
support a driver-specific attribute, such as C<{ ora_type =>E<gt>C< 97 }>.  The
data type for a placeholder cannot be changed after the first
C<bind_param> call. However, it can be left unspecified, in which case it
defaults to the previous value.

The SQL_INTEGER and other related constants can be imported using

  use DBI qw(:sql_types);

See L</"DBI Constants"> for more information.

Perl only has string and number scalar data types. All database types
that aren't numbers are bound as strings and must be in a format the
database will understand.

As an alternative to specifying the data type in the C<bind_param> call,
you can let the driver pass the value as the default type (C<VARCHAR>).
You can then use an SQL function to convert the type within the statement.
For example:

  INSERT INTO price(code, price) VALUES (?, CONVERT(MONEY,?))

The C<CONVERT> function used here is just an example. The actual function
and syntax will vary between different databases and is non-portable.

See also L</"Placeholders and Bind Values"> for more information.


=item C<bind_param_inout>

  $rc = $sth->bind_param_inout($p_num, \$bind_value, $max_len)  or die $sth->errstr;
  $rv = $sth->bind_param_inout($p_num, \$bind_value, $max_len, \%attr)     or ...
  $rv = $sth->bind_param_inout($p_num, \$bind_value, $max_len, $bind_type) or ...

This method acts like L</bind_param>, but also enables values to be
updated by the statement. The statement is typically
a call to a stored procedure. The C<$bind_value> must be passed as a
reference to the actual value to be used.

Note that unlike L</bind_param>, the C<$bind_value> variable is not
read when C<bind_param_inout> is called. Instead, the value in the
variable is read at the time L</execute> is called.

The additional C<$max_len> parameter specifies the minimum amount of
memory to allocate to C<$bind_value> for the new value. If the value
returned from the database is too
big to fit, then the execution should fail. If unsure what value to use,
pick a generous length, i.e., a length larger than the longest value that would ever be
returned.  The only cost of using a larger value than needed is wasted memory.

It is expected that few drivers will support this method. The only
driver currently known to do so is DBD::Oracle (DBD::ODBC may support
it in a future release). Therefore it should not be used for database
independent applications.

Undefined values or C<undef> are used to indicate null values.
See also L</"Placeholders and Bind Values"> for more information.


=item C<execute>

  $rv = $sth->execute                or die $sth->errstr;
  $rv = $sth->execute(@bind_values)  or die $sth->errstr;

Perform whatever processing is necessary to execute the prepared
statement.  An C<undef> is returned if an error occurs.  A successful
C<execute> always returns true regardless of the number of rows affected,
even if it's zero (see below). It is always important to check the
return status of C<execute> (and most other DBI methods) for errors.

For a I<non>-C<SELECT> statement, C<execute> returns the number of rows
affected, if known. If no rows were affected, then C<execute> returns
"C<0E0>", which Perl will treat as 0 but will regard as true. Note that it
is I<not> an error for no rows to be affected by a statement. If the
number of rows affected is not known, then C<execute> returns -1.

For C<SELECT> statements, execute simply "starts" the query within the
database engine. Use one of the fetch methods to retreive the data after
calling C<execute>.  The C<execute> method does I<not> return the number of
rows that will be returned by the query (because most databases can't
tell in advance), it simply returns a true value.

If any arguments are given, then C<execute> will effectively call
L</bind_param> for each value before executing the statement.
Values bound in this way are usually treated as C<SQL_VARCHAR> types
unless the driver can determine the correct type (which is rare), or
unless
C<bind_param> (or C<bind_param_inout>) has already been used to specify the
type.


=item C<fetchrow_arrayref>

  $ary_ref = $sth->fetchrow_arrayref;
  $ary_ref = $sth->fetch;    # alias

Fetches the next row of data and returns a reference to an array
holding the field values.  Null fields are returned as C<undef>
values in the array.
This is the fastest way to fetch data, particularly if used with
C<$sth->E<gt>C<bind_columns>.

If there are no more rows or if an error occurs, then C<fetchrow_arrayref>
returns an C<undef>. You should check C<$sth->E<gt>C<err> afterwards (or use the
C<RaiseError> attribute) to discover if the C<undef> returned was due to an
error.

Note that the same array reference is returned for each fetch, so don't
store the reference and then use it after a later fetch.  Also, the
elements of the array are also reused for each row, so take care if you
want to take a reference to an element. See also L</bind_columns>.

=item C<fetchrow_array>

 @ary = $sth->fetchrow_array;

An alternative to C<fetchrow_arrayref>. Fetches the next row of data
and returns it as a list containing the field values.  Null fields
are returned as C<undef> values in the list.

If there are no more rows or if an error occurs, then C<fetchrow_array>
returns an empty list. You should check C<$sth->E<gt>C<err> afterwards (or use
the C<RaiseError> attribute) to discover if the empty list returned was
due to an error.

In a scalar context, C<fetchrow_array> returns the value of the first
field. An C<undef> is returned if there are no more rows or if an error
occurred. Since that C<undef> can't be distinguished from an C<undef> returned
because the first field value was NULL, you should exercise some
caution if you use C<fetchrow_array> in a scalar context.

=item C<fetchrow_hashref>

 $hash_ref = $sth->fetchrow_hashref;
 $hash_ref = $sth->fetchrow_hashref($name);

An alternative to C<fetchrow_arrayref>. Fetches the next row of data
and returns it as a reference to a hash containing field name and field
value pairs.  Null fields are returned as C<undef> values in the hash.

If there are no more rows or if an error occurs, then C<fetchrow_hashref>
returns an C<undef>. You should check C<$sth->E<gt>C<err> afterwards (or use the
C<RaiseError> attribute) to discover if the C<undef> returned was due to an
error.

The optional C<$name> parameter specifies the name of the statement handle
attribute. For historical reasons it defaults to "C<NAME>", however using either
"C<NAME_lc>" or "C<NAME_uc>" is recomended for portability.

The keys of the hash are the same names returned by C<$sth->E<gt>C<{$name}>. If
more than one field has the same name, there will only be one entry in
the returned hash for those fields.

Because of the extra work C<fetchrow_hashref> and Perl have to perform, it
is not as efficient as C<fetchrow_arrayref> or C<fetchrow_array>.

Currently, a new hash reference is returned for each row.  I<This will
change> in the future to return the same hash ref each time, so don't
rely on the current behaviour.


=item C<fetchall_arrayref>

  $tbl_ary_ref = $sth->fetchall_arrayref;
  $tbl_ary_ref = $sth->fetchall_arrayref( $slice_array_ref );
  $tbl_ary_ref = $sth->fetchall_arrayref( $slice_hash_ref  );

The C<fetchall_arrayref> method can be used to fetch all the data to be
returned from a prepared and executed statement handle. It returns a
reference to an array that contains one reference per row.

If there are no rows to return, C<fetchall_arrayref> returns a reference
to an empty array. If an error occurs, C<fetchall_arrayref> returns the
data fetched thus far, which may be none.  You should check C<$sth->E<gt>C<err>
afterwards (or use the C<RaiseError> attribute) to discover if the data is
complete or was truncated due to an error.

When passed an array reference, C<fetchall_arrayref> uses L</fetchrow_arrayref>
to fetch each row as an array ref. If the parameter array is not empty
then it is used as a slice to select individual columns by index number.

With no parameters, C<fetchall_arrayref> acts as if passed an empty array ref.

When passed a hash reference, C<fetchall_arrayref> uses L</fetchrow_hashref>
to fetch each row as a hash reference. If the parameter hash is empty then
fetchrow_hashref is simply called in a tight loop and the keys in the hashes
have whatever name lettercase is returned by default from fetchrow_hashref.

If the parameter hash is not empty, then it is used as a slice to
select individual columns by name. The names should be lower case
regardless of the letter case in C<$sth->E<gt>C<{NAME}>.  The values of
the hash should be set to 1.

For example, to fetch just the first column of every row:

  $tbl_ary_ref = $sth->fetchall_arrayref([0]);

To fetch the second to last and last column of every row:

  $tbl_ary_ref = $sth->fetchall_arrayref([-2,-1]);

To fetch all fields of every row as a hash ref:

  $tbl_ary_ref = $sth->fetchall_arrayref({});

To fetch only the fields called "foo" and "bar" of every row as a hash ref:

  $tbl_ary_ref = $sth->fetchall_arrayref({ foo=>1, bar=>1 });

The first two examples return a reference to an array of array refs. The last
returns a reference to an array of hash refs.


=item C<fetchall_hashref>

  $tbl_ary_ref = $sth->fetchall_hashref;

The C<fetchall_hashref> method can be used to fetch all the data to be
returned from a prepared and executed statement handle. It returns a
reference to an array that contains one hash of field name and value
pairs per row.

If there are no rows to return, C<fetchall_hashref> returns a reference
to an empty array. If an error occurs, C<fetchall_hashref> returns the
data fetched thus far, which may be none.  You should check C<$sth->E<gt>C<err>
afterwards (or use the C<RaiseError> attribute) to discover if the data is
complete or was truncated due to an error.


=item C<finish>

  $rc  = $sth->finish;

Indicates that no more data will be fetched from this statement handle
before it is either executed again or destroyed.  The C<finish> method
is rarely needed, but can sometimes be helpful in very specific
situations to allow the server to free up resources (such as sort
buffers).

When all the data has been fetched from a C<SELECT> statement, the driver
should automatically call C<finish> for you. So you should not normally
need to call it explicitly.

Consider a query like:

  SELECT foo FROM table WHERE bar=? ORDER BY foo

where you want to select just the first (smallest) "foo" value from a
very large table. When executed, the database server will have to use
temporary buffer space to store the sorted rows. If, after executing
the handle and selecting one row, the handle won't be re-executed for
some time and won't be destroyed, the C<finish> method can be used to tell
the server that the buffer space can be freed.

Calling C<finish> resets the L</Active> attribute for the statement.  It
may also make some statement handle attributes (such as C<NAME> and C<TYPE>)
unavailable if they have not already been accessed (and thus cached).

The C<finish> method does not affect the transaction status of the
database connection.  It has nothing to do with transactions. It's mostly an
internal "housekeeping" method that is rarely needed. There's no need
to call C<finish> if you're about to destroy or re-execute the statement
handle.  See also L</disconnect> and the L</Active> attribute.

The C<finish> method should have been called C<cancel_select>.


=item C<rows>

  $rv = $sth->rows;

Returns the number of rows affected by the last row affecting command,
or -1 if the number of rows is not known or not available.

Generally, you can only rely on a row count after a I<non>-C<SELECT>
C<execute> (for some specific operations like C<UPDATE> and C<DELETE>), or
after fetching all the rows of a C<SELECT> statement.

For C<SELECT> statements, it is generally not possible to know how many
rows will be returned except by fetching them all.  Some drivers will
return the number of rows the application has fetched so far, but
others may return -1 until all rows have been fetched.  So use of the
C<rows> method or C<$DBI::rows> with C<SELECT> statements is not
recommended.

One alternative method to get a row count for a C<SELECT> is to execute a
"SELECT COUNT(*) FROM ..." SQL statement with the same "..." as your
query and then fetch the row count from that.


=item C<bind_col>

  $rc = $sth->bind_col($column_number, \$var_to_bind);

Binds an output column (field) of a C<SELECT> statement to a Perl variable.
See C<bind_columns> below for an example.  Note that column numbers count
up from 1.

Whenever a row is fetched from the database, the corresponding Perl
variable is automatically updated. There is no need to fetch and assign
the values manually.  The binding is performed at a very low level
using Perl aliasing so there is no extra copying taking place.  This
makes using bound variables very efficient.

For maximum portability between drivers, C<bind_col> should be called after
C<execute>. This restriction may be removed in a later version of the DBI.

You do not need to bind output columns in order to fetch data, but it
can be useful for some applications which need either maximum performance
or greater clarity of code.  The L</bind_param> method
performs a similar but opposite function for input variables.

=item C<bind_columns>

  $rc = $sth->bind_columns(@list_of_refs_to_vars_to_bind);

Calls L</bind_col> for each column of the C<SELECT> statement.
The C<bind_columns> method will die if the number of references does not
match the number of fields.

For maximum portability between drivers, C<bind_columns> should be called
after C<execute>.

For example:

  $dbh->{RaiseError} = 1; # do this, or check every call for errors
  $sth = $dbh->prepare(q{ SELECT region, sales FROM sales_by_region });
  $sth->execute;
  my ($region, $sales);

  # Bind Perl variables to columns:
  $rv = $sth->bind_columns(\$region, \$sales);

  # you can also use Perl's \(...) syntax (see perlref docs):
  #     $sth->bind_columns(\($region, $sales));

  # Column binding is the most efficient way to fetch data
  while ($sth->fetch) {
      print "$region: $sales\n";
  }

For compatibility with old scripts, the first parameter will be
ignored if it is C<undef> or a hash reference.

Here's a more fancy example that binds columns to the values I<inside>
a hash (thanks to H.Merijn Brand):

  $sth->execute;
  my %row;
  $sth->bind_columns( \( @row{ @{$sth->{NAME_lc} } } ));
  while ($sth->fetch) {
      print "$row{region}: $row{sales}\n";
  }


=item C<dump_results>

  $rows = $sth->dump_results($maxlen, $lsep, $fsep, $fh);

Fetches all the rows from C<$sth>, calls C<DBI::neat_list> for each row, and
prints the results to C<$fh> (defaults to C<STDOUT>) separated by C<$lsep>
(default C<"\n">). C<$fsep> defaults to C<", "> and C<$maxlen> defaults to 35.

This method is designed as a handy utility for prototyping and
testing queries. Since it uses L</neat_list> to
format and edit the string for reading by humans, it is not recomended
for data transfer applications.

=back


=head2 Statement Handle Attributes

This section describes attributes specific to statement handles. Most
of these attributes are read-only.

Changes to these statement handle attributes do not affect any other
existing or future statement handles.

Attempting to set or get the value of an unknown attribute is fatal,
except for private driver specific attributes (which all have names
starting with a lowercase letter).

Example:

  ... = $h->{NUM_OF_FIELDS};	# get/read

Note that some drivers cannot provide valid values for some or all of
these attributes until after C<$sth->E<gt>C<execute> has been called.

See also L</finish> to learn more about the effect it 
may have on some attributes.

=over 4

=item C<NUM_OF_FIELDS>  (integer, read-only)

Number of fields (columns) the prepared statement will return. Non-C<SELECT>
statements will have C<NUM_OF_FIELDS == 0>.


=item C<NUM_OF_PARAMS>  (integer, read-only)

The number of parameters (placeholders) in the prepared statement.
See SUBSTITUTION VARIABLES below for more details.


=item C<NAME>  (array-ref, read-only)

Returns a reference to an array of field names for each column. The
names may contain spaces but should not be truncated or have any
trailing space. Note that the names have the letter case (upper, lower
or mixed) as returned by the driver being used. Portable applications
should use L</NAME_lc> or L</NAME_uc>.

  print "First column name: $sth->{NAME}->[0]\n";

=item C<NAME_lc>  (array-ref, read-only)

Like L</NAME> but always returns lowercase names.

=item C<NAME_uc>  (array-ref, read-only)

Like L</NAME> but always returns uppercase names.

=item C<TYPE>  (array-ref, read-only)

Returns a reference to an array of integer values for each
column. The value indicates the data type of the corresponding column.

The values correspond to the international standards (ANSI X3.135
and ISO/IEC 9075) which, in general terms, means ODBC. Driver-specific
types that don't exactly match standard types should generally return
the same values as an ODBC driver supplied by the makers of the
database. That might include private type numbers in ranges the vendor
has officially registered with the ISO working group:

  ftp://jerry.ece.umassd.edu/isowg3/dbl/SQL_Registry

Where there's no vendor-supplied ODBC driver to be compatible with, the
DBI driver can use type numbers in the range that is now officially reserved
for use by the DBI: -9999 to -9000.

All possible values for C<TYPE> should have at least one entry in the
output of the C<type_info_all> method (see L</type_info_all>).

=item C<PRECISION>  (array-ref, read-only)

Returns a reference to an array of integer values for each
column.  For non-numeric columns, the value generally refers to either
the maximum length or the defined length of the column.  For numeric
columns, the value refers to the maximum number of significant digits
used by the data type (without considering a sign character or decimal
point).  Note that for floating point types (REAL, FLOAT, DOUBLE), the
"display size" can be up to 7 characters greater than the precision.
(for the sign + decimal point + the letter E + a sign + 2 or 3 digits).

=item C<SCALE>  (array-ref, read-only)

Returns a reference to an array of integer values for each column.
NULL (C<undef>) values indicate columns where scale is not applicable.

=item C<NULLABLE>  (array-ref, read-only)

Returns a reference to an array indicating the possibility of each
column returning a null.  Possible values are C<0>
(or an empty string) = no, C<1> = yes, C<2> = unknown.

  print "First column may return NULL\n" if $sth->{NULLABLE}->[0];


=item C<CursorName>  (string, read-only)

Returns the name of the cursor associated with the statement handle, if
available. If not available or if the database driver does not support the
C<"where current of ..."> SQL syntax, then it returns C<undef>.


=item C<Statement>  (string, read-only)

Returns the statement string passed to the L</prepare> method.


=item C<RowsInCache>  (integer, read-only)

If the driver supports a local row cache for C<SELECT> statements, then
this attribute holds the number of un-fetched rows in the cache. If the
driver doesn't, then it returns C<undef>. Note that some drivers pre-fetch
rows on execute, whereas others wait till the first fetch.

See also the L</RowCacheSize> database handle attribute.

=back


=head1 FURTHER INFORMATION

=head2 Transactions

Transactions are a fundamental part of any robust database system. They
protect against errors and database corruption by ensuring that sets of
related changes to the database take place in atomic (indivisible,
all-or-nothing) units.

This section applies to databases that support transactions and where
C<AutoCommit> is off.  See L</AutoCommit> for details of using C<AutoCommit>
with various types of databases.

The recommended way to implement robust transactions in Perl
applications is to use C<RaiseError> and S<C<eval { ... }>>
(which is very fast, unlike S<C<eval "...">>). For example:

  $dbh->{AutoCommit} = 0;  # enable transactions, if possible
  $dbh->{RaiseError} = 1;
  eval {
      foo(...)        # do lots of work here
      bar(...)        # including inserts
      baz(...)        # and updates
      $dbh->commit;   # commit the changes if we get this far
  };
  if ($@) {
      warn "Transaction aborted because $@";
      $dbh->rollback; # undo the incomplete changes
      # add other application on-error-clean-up code here
  }

If the C<RaiseError> attribute is not set, then DBI calls would need to be
manually checked for errors, typically like this:

  $h->method(@args) or die $h->errstr;

With C<RaiseError> set, the DBI will automatically C<die> if any DBI method
call on that handle (or a child handle) fails, so you don't have to
test the return value of each method call. See L</RaiseError> for more
details.

A major advantage of the C<eval> approach is that the transaction will be
properly rolled back if I<any> code (not just DBI calls) in the inner
application dies for any reason. The major advantage of using the
C<$h->E<gt>C<{RaiseError}> attribute is that all DBI calls will be checked
automatically. Both techniques are strongly recommended.

After calling C<commit> or C<rollback> many drivers will not let you
fetch from a previously active C<SELECT> statement handle that's a child
of the same database handle. A typical way round this is to connect the
the database twice and use one connection for C<SELECT> statements.


=head2 Handling BLOB / LONG / Memo Fields

Many databases support "blob" (binary large objects), "long", or similar
datatypes for holding very long strings or large amounts of binary
data in a single field. Some databases support variable length long
values over 2,000,000,000 bytes in length.

Since values of that size can't usually be held in memory, and because
databases can't usually know in advance the length of the longest long
that will be returned from a C<SELECT> statement (unlike other data
types), some special handling is required.

In this situation, the value of the C<$h->E<gt>C<{LongReadLen}> attribute is used
to determine how much buffer space to allocate when fetching such
fields.  The C<$h->E<gt>C<{LongTruncOk}> attribute is used to determine how to
behave if a fetched value can't fit into the buffer.

When trying to insert long or binary values, placeholders should be used
since there are often limits on the maximum size of an C<INSERT>
statement and the L</quote> method generally can't cope with binary
data.  See L</Placeholders and Bind Values>.


=head2 Simple Examples

Here's a complete example program to select and fetch some data:

  my $dbh = DBI->connect("dbi:DriverName:db_name", $user, $password)
      or die "Can't connect to $data_source: $DBI::errstr";

  my $sth = $dbh->prepare( q{
          SELECT name, phone
          FROM mytelbook
  }) or die "Can't prepare statement: $DBI::errstr";

  my $rc = $sth->execute
      or die "Can't execute statement: $DBI::errstr";

  print "Query will return $sth->{NUM_OF_FIELDS} fields.\n\n";
  print "Field names: @{ $sth->{NAME} }\n";

  while (($name, $phone) = $sth->fetchrow_array) {
      print "$name: $phone\n";
  }
  # check for problems which may have terminated the fetch early
  die $sth->errstr if $sth->err;

  $dbh->disconnect;

Here's a complete example program to insert some data from a file.
(This example uses C<RaiseError> to avoid needing to check each call).

  my $dbh = DBI->connect("dbi:DriverName:db_name", $user, $password, {
      RaiseError => 1, AutoCommit => 0
  });

  my $sth = $dbh->prepare( q{
      INSERT INTO table (name, phone) VALUES (?, ?)
  });

  open FH, "<phone.csv" or die "Unable to open phone.csv: $!";
  while (<FH>) {
      chomp;
      my ($name, $phone) = split /,/;
      $sth->execute($name, $phone);
  }
  close FH;

  $dbh->commit;
  $dbh->disconnect;

Here's how to convert fetched NULLs (undefined values) into empty strings:

  while($row = $sth->fetchrow_arrayref) {
    # this is a fast and simple way to deal with nulls:
    foreach (@$row) { $_ = '' unless defined }
    print "@$row\n";
  }

The C<q{...}> style quoting used in these examples avoids clashing with
quotes that may be used in the SQL statement. Use the double-quote like
C<qq{...}> operator if you want to interpolate variables into the string.
See L<perlop/"Quote and Quote-like Operators"> for more details.


=head2 Threads and Thread Safety

Perl versions 5.004_50 and later include optional experimental support
for multiple threads on many platforms.  If the DBI is built using a
Perl that has threads enabled then it will use a per-driver mutex to
ensure that only one thread is with a driver at any one time.
Please note that support for threads in Perl is still experimental and
is known to have some significant problems. It's use is not recommended.


=head2 Signal Handling and Canceling Operations

The first thing to say is that signal handling in Perl is currently
I<not> safe. There is always a small risk of Perl crashing and/or
core dumping when, or after, handling a signal.  (The risk was reduced
with 5.004_04 but is still present.)

The two most common uses of signals in relation to the DBI are for
canceling operations when the user types Ctrl-C (interrupt), and for
implementing a timeout using C<alarm()> and C<$SIG{ALRM}>.

To assist in implementing these operations, the DBI provides a C<cancel>
method for statement handles. The C<cancel> method should abort the current
operation and is designed to be called from a signal handler.

However, it must be stressed that: a) few drivers implement this at
the moment (the DBI provides a default method that just returns C<undef>);
and b) even if implemented, there is still a possibility that the statement
handle, and possibly the parent database handle, will not be usable
afterwards.

If C<cancel> returns true, then it has successfully
invoked the database engine's own cancel function.  If it returns false,
then C<cancel> failed. If it returns C<undef>, then the database
engine does not have cancel implemented.


=head1 DEBUGGING

In addition to the L</trace> method, you can enable the same trace
information by setting the C<DBI_TRACE> environment variable before
starting Perl.

On Unix-like systems using a Bourne-like shell, you can do this easily
on the command line:

  DBI_TRACE=2 perl your_test_script.pl

If C<DBI_TRACE> is set to a non-numeric value, then it is assumed to
be a file name and the trace level will be set to 2 with all trace
output appended to that file. If the name begins with a number
followed by an equal sign (C<=>), then the number and the equal sign are
stripped off from the name, and the number is used to set the trace
level. For example:

  DBI_TRACE=1=dbitrace.log perl your_test_script.pl

See also the L</trace> method.

It can sometimes be handy to compare trace files from two different
runs of the same script. However using a tool like C<diff> doesn't work
well because the trace file is full of object addresses that may
differ each run. Here's a handy little command to strip those out:

 perl -pe 's/\b0x[\da-f]{6,}/0xNNNN/gi; s/\b[\da-f]{6,}/<long number>/gi'


=head1 WARNING AND ERROR MESSAGES

=head2 Fatal Errors

=over 4

=item Can't call method "prepare" without a package or object reference

The C<$dbh> handle you're using to call C<prepare> is probably undefined because
the preceding C<connect> failed. You should always check the return status of
DBI methods, or use the L</RaiseError> attribute.

=item Can't call method "execute" without a package or object reference

The C<$sth> handle you're using to call C<execute> is probably undefined because
the preceeding C<prepare> failed. You should always check the return status of
DBI methods, or use the L</RaiseError> attribute.

=item DBI/DBD internal version mismatch

The DBD driver module was built with a different version of DBI than
the one currently being used.  You should rebuild the DBD module under
the current version of DBI.

(Some rare platforms require "static linking". On those platforms, there
may be an old DBI or DBD driver version actually embedded in the Perl
executable being used.)

=item DBD driver has not implemented the AutoCommit attribute

The DBD driver implementation is incomplete. Consult the author.

=item Can't [sg]et %s->{%s}: unrecognised attribute

You attempted to set or get an unknown attribute of a handle.  Make
sure you have spelled the attribute name correctly; case is significant
(e.g., "Autocommit" is not the same as "AutoCommit").

=back

=head2 Warnings

=over 4

=item Database handle destroyed without explicit disconnect

A C<$dbh> handle went out of scope or the program ended before the handle
was disconnected from the database.  You should always explicitly call
C<disconnect> when you are finished using a database handle. If using
transactions then you should also explicitly call C<commit> or C<rollback>
before C<disconnect>.

=item DBI Handle cleared whilst still holding %d cached kids!

Most probably due to a DBI bug. Possibly a DBD driver bug. Please report it.

=item DBI Handle cleared whilst still active!

Most probably due to a DBI bug. Possibly a DBD driver bug. Please report it.

=item DBI Handle has uncleared implementors data

Most probably a DBD driver bug. Please report it.

=item DBI Handle has %d uncleared child handles

Most probably due to a DBI bug. Possibly a DBD driver bug. Please report it.

=back

=head1 SEE ALSO

=head2 Driver and Database Documentation

Refer to the documentation for the DBD driver that you are using.

Refer to the SQL Language Reference Manual for the database engine that you are using.

=head2 Books and Journals

 Programming the Perl DBI, by Alligator Descartes and Tim Bunce.

 Programming Perl 2nd Ed. by Larry Wall, Tom Christiansen & Randal Schwartz.

 Learning Perl by Randal Schwartz.

 Dr Dobb's Journal, November 1996.

 The Perl Journal, April 1997.

=head2 Manual Pages

L<perl(1)>, L<perlmod(1)>, L<perlbook(1)>

=head2 Mailing List

The I<dbi-users> mailing list is the primary means of communication among
users of the DBI and its related modules. For details send email to:

 dbi-users-help@perl.org

There are typically between 700 and 900 messages per month.  You have
to subscribe in order to be able to post. However you can opt for a
'post-only' subscription.

Mailing list archives are held at:

 http://www.xray.mpe.mpg.de/mailing-lists/dbi/
 http://groups.yahoo.com/group/dbi-users
 http://www.bitmechanic.com/mail-archives/dbi-users/
 http://marc.theaimsgroup.com/?l=perl-dbi&r=1&w=2
 http://www.mail-archive.com/dbi-users%40perl.org/
 http://www.mail-archive.com/dbi-users%40perl.org/

=head2 Assorted Related WWW Links

The DBI "Home Page":

 http://dbi.perl.org/

Other DBI related links:

 http://tegan.deltanet.com/~phlip/DBUIdoc.html
 http://dc.pm.org/perl_db.html
 http://wdvl.com/Authoring/DB/Intro/toc.html
 http://www.hotwired.com/webmonkey/backend/tutorials/tutorial1.html
 http://bumppo.net/lists/macperl/1999/06/msg00197.html

Other database related links:

 http://www.jcc.com/sql_stnd.html
 http://cuiwww.unige.ch/OSG/info/FreeDB/FreeDB.home.html

Commercial and Data Warehouse Links

 http://www.dwinfocenter.org
 http://www.datawarehouse.com
 http://www.datamining.org
 http://www.olapcouncil.org
 http://www.idwa.org
 http://www.knowledgecenters.org/dwcenter.asp

Recommended Perl Programming Links

 http://language.perl.com/style/

=head2 FAQ

Please also read the DBI FAQ which is installed as a DBI::FAQ module. 
You can use I<perldoc> to read it by executing the C<perldoc DBI::FAQ> command.

=head1 AUTHORS

DBI by Tim Bunce.  This pod text by Tim Bunce, J. Douglas Dunlop,
Jonathan Leffler and others.  Perl by Larry Wall and the
C<perl5-porters>.

=head1 COPYRIGHT

The DBI module is Copyright (c) 1994-2000 Tim Bunce. England.
All rights reserved.

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the Perl README file.

=head1 ACKNOWLEDGEMENTS

I would like to acknowledge the valuable contributions of the many
people I have worked with on the DBI project, especially in the early
years (1992-1994). In no particular order: Kevin Stock, Buzz Moschetti,
Kurt Andersen, Ted Lemon, William Hails, Garth Kennedy, Michael Peppler,
Neil S. Briscoe, Jeff Urlwin, David J. Hughes, Jeff Stander,
Forrest D Whitcher, Larry Wall, Jeff Fried, Roy Johnson, Paul Hudson,
Georg Rehfeld, Steve Sizemore, Ron Pool, Jon Meek, Tom Christiansen,
Steve Baumgarten, Randal Schwartz, and a whole lot more.

Then, of course, there are the poor souls who have struggled through
untold and undocumented obstacles to actually implement DBI drivers.
Among their ranks are Jochen Wiedmann, Alligator Descartes, Jonathan
Leffler, Jeff Urlwin, Michael Peppler, Henrik Tougaard, Edwin Pratomo,
Davide Migliavacca, Jan Pazdziora, Peter Haworth, Edmund Mergl, Steve
Williams, Thomas Lowery, and Phlip Plumlee. Without them, the DBI would
not be the practical reality it is today.  I'm also especially grateful
to Alligator Descartes for starting work on the "Programming the Perl
DBI" book and letting me jump on board.

=head1 TRANSLATIONS

A German translation of this manual (possibly slightly out of date) is
available, thanks to O'Reilly, at:

  http://www.oreilly.de/catalog/perldbiger/

Some other translations:

 http://cronopio.net/perl/                              - Spanish
 http://member.nifty.ne.jp/hippo2000/dbimemo.htm        - Japanese


=head1 SUPPORT / WARRANTY

The DBI is free software. IT COMES WITHOUT WARRANTY OF ANY KIND.

Commercial support for Perl and the DBI, DBD::Oracle and
Oraperl modules can be arranged via The Perl Clinic.
For more details visit:

  http://www.perlclinic.com

=head1 TRAINING

References to DBI related training resources. No recommendation implied.

  http://www.treepax.co.uk/
  http://www.keller.com/dbweb/

=head1 FREQUENTLY ASKED QUESTIONS

See the DBI FAQ for a more comprehensive list of FAQs. Use the
C<perldoc DBI::FAQ> command to read it.

=head2 How fast is the DBI?

To measure the speed of the DBI and DBD::Oracle code, I modified
DBD::Oracle so you can set an attribute that will cause the
same row to be fetched from the row cache over and over again (without
involving Oracle code but exercising *all* the DBI and DBD::Oracle code
in the code path for a fetch).

The results (on my lightly loaded old Sparc 10) fetching 50000 rows using:

	1 while $csr->fetch;

were:
	one field:   5300 fetches per cpu second (approx)
	ten fields:  4000 fetches per cpu second (approx)

Obviously results will vary between platforms (newer faster platforms
can reach around 50000 fetches per second), but it does give a feel for
the maximum performance: fast.  By way of comparison, using the code:

	1 while @row = $csr->fetchrow_array;

(C<fetchrow_array> is roughly the same as C<ora_fetch>) gives:

	one field:   3100 fetches per cpu second (approx)
	ten fields:  1000 fetches per cpu second (approx)

Notice the slowdown and the more dramatic impact of extra fields.
(The fields were all one char long. The impact would be even bigger for
longer strings.)

Changing that slightly to represent actually doing something in Perl
with the fetched data:

    while(@row = $csr->fetchrow_array) {
        $hash{++$i} = [ @row ];
    }

gives:	ten fields:  500 fetches per cpu second (approx)

That simple addition has *halved* the performance.

I therefore conclude that DBI and DBD::Oracle overheads are small
compared with Perl language overheads (and probably database overheads).

So, if you think the DBI or your driver is slow, try replacing your
fetch loop with just:

	1 while $csr->fetch;

and time that. If that helps then point the finger at your own code. If
that doesn't help much then point the finger at the database, the
platform, the network etc. But think carefully before pointing it at
the DBI or your driver.

(Having said all that, if anyone can show me how to make the DBI or
drivers even more efficient, I'm all ears.)


=head2 Why doesn't my CGI script work right?

Read the information in the references below.  Please do I<not> post
CGI related questions to the I<dbi-users> mailing list (or to me).

 http://www.perl.com/cgi-bin/pace/pub/doc/FAQs/cgi/perl-cgi-faq.html
 http://www3.pair.com/webthing/docs/cgi/faqs/cgifaq.shtml
 http://www-genome.wi.mit.edu/WWW/faqs/www-security-faq.html
 http://www.boutell.com/faq/
 http://www.perl.com/perl/faq/

General problems and good ideas:

 Use the CGI::ErrorWrap module.
 Remember that many env vars won't be set for CGI scripts.

=head2 How can I maintain a WWW connection to a database?

For information on the Apache httpd server and the C<mod_perl> module see

  http://perl.apache.org/

=head2 What about ODBC?

A DBD::ODBC module is available.

=head2 Does the DBI have a year 2000 problem?

No. The DBI has no knowledge or understanding of dates at all.

Individual drivers (DBD::*) may have some date handling code but are
unlikely to have year 2000 related problems within their code. However,
your application code which I<uses> the DBI and DBD drivers may have
year 2000 related problems if it has not been designed and written well.

See also the "Does Perl have a year 2000 problem?" section of the Perl FAQ:

  http://www.perl.com/CPAN/doc/FAQs/FAQ/PerlFAQ.html

=head1 OTHER RELATED WORK AND PERL MODULES

=over 4

=item Apache::DBI by E.Mergl@bawue.de

To be used with the Apache daemon together with an embedded Perl
interpreter like C<mod_perl>. Establishes a database connection which
remains open for the lifetime of the HTTP daemon. This way the CGI
connect and disconnect for every database access becomes superfluous.

=item JDBC Server by Stuart 'Zen' Bishop zen@bf.rmit.edu.au

The server is written in Perl. The client classes that talk to it are 
of course in Java. Thus, a Java applet or application will be able to 
comunicate via the JDBC API with any database that has a DBI driver installed.
The URL used is in the form C<jdbc:dbi://host.domain.etc:999/Driver/DBName>.
It seems to be very similar to some commercial products, such as jdbcKona.

=item Remote Proxy DBD support

As of DBI 1.02, a complete implementation of a DBD::Proxy driver and the
DBI::ProxyServer are part of the DBI distribution.

=item SQL Parser

	Hugo van der Sanden <hv@crypt.compulink.co.uk>
	Stephen Zander <stephen.zander@mckesson.com>

Based on the O'Reilly lex/yacc book examples and C<byacc>.

See also the SQL::Statement module, a very simple SQL parser and engine,
base of the DBD::CSV driver.

=back

=cut
