#!perl -w

use lib qw(blib/arch blib/lib);	# needed since -T ignores PERL5LIB
use DBI qw(:sql_types);
use Config;
use Cwd;
$|=1;

my $haveFileSpec = eval { require File::Spec };

print "1..$tests\n";

require VMS::Filespec if $^O eq 'VMS';

sub ok ($$;$) {
    my($n, $ok, $msg) = @_;
	$msg = ($msg) ? " ($msg)" : "";
    ++$t;
    die "sequence error, expected $n but actually $t at line ".(caller)[2]."\n"
		if $n and $n != $t;
    ($ok) ? print "ok $t\n" : print "not ok $t\n";
    warn "# failed test $t at line ".(caller)[2]."$msg\n" unless $ok;
    return $ok;
}
	

my $trace_file = "dbitrace.log";
unlink $trace_file;
ok(0, !-e $trace_file);
my $orig_trace_level = DBI->trace;
DBI->trace(3,$trace_file);		# enable trace before first driver load

my $r;
my $dbh = DBI->connect('dbi:ExampleP(AutoCommit=>1 ,Taint = 1):', undef, undef);
die "Unable to connect to ExampleP driver: $DBI::errstr" unless $dbh;

if (0) {
DBI->trace(9,undef);
warn DBI::dump_handle($dbh,"dump_handle",1);
warn my $foo=$dbh->{Driver};
warn $foo=$dbh->{Driver};
die DBI::dump_handle($dbh,"dump_handle",1);
}

my $dbh2 = DBI->connect('dbi:ExampleP:', '', '');
ok(0, $dbh ne $dbh2);
my $dbh3 = DBI->connect_cached('dbi:ExampleP:', '', '');
my $dbh4 = DBI->connect_cached('dbi:ExampleP:', '', '');
ok(0, $dbh3 eq $dbh4);
my $dbh5 = DBI->connect_cached('dbi:ExampleP:', '', '', { foo=>1 });
ok(0, $dbh5 ne $dbh4);

$dbh->{AutoCommit} = 1;
$dbh->{PrintError} = 0;
ok(0, $dbh->{Taint}      == 1);
ok(0, $dbh->{AutoCommit} == 1);
ok(0, $dbh->{PrintError} == 0);
#$dbh->trace(2);

ok(0, $dbh->quote("quote's") eq "'quote''s'");
ok(0, $dbh->quote("42", SQL_VARCHAR) eq "'42'");
ok(0, $dbh->quote("42", SQL_INTEGER) eq "42");
ok(0, $dbh->quote(undef)     eq "NULL");

eval { $dbh->commit('dummy') };
ok(0, $@ =~ m/DBI commit: invalid number of parameters: handle \+ 1/);

DBI->trace(0, undef);
if ($^O =~ /cygwin/i) { # cygwin has buffer flushing bug
	ok(0, 1);
} else {
	ok(0,  -s $trace_file > 1024, "trace file size = " . -s $trace_file);
}
unlink $trace_file;
ok(0, !-e $trace_file);

# internal hack to assist debugging using DBI_TRACE env var. See DBI.pm.
DBI->trace(@DBI::dbi_debug) if @DBI::dbi_debug;

ok(0, $dbh->ping);
my $cursor_e = $dbh->prepare("select unknown_field_name from ?");
ok(0, !defined $cursor_e);
ok(0, $DBI::err);
ok(0, $DBI::errstr =~ m/Unknown field names: unknown_field_name/);
ok(0, $DBI::err    == $dbh->err);
ok(0, $DBI::errstr eq $dbh->errstr);

ok(0, $dbh->errstr eq $dbh->func('errstr'));

foreach(17..19) { ok(0, 1) }	# soak up to next round number

my $std_sql = "select mode,size,name from ?";
my $csr_a = $dbh->prepare($std_sql);
ok(0, ref $csr_a);
my $csr_b = $dbh->prepare($std_sql);
ok(0, ref $csr_b);

ok(0, $csr_a != $csr_b);
ok(0, $csr_a->{NUM_OF_FIELDS} == 3);
ok(0, $csr_a->{'Database'}->{'Driver'}->{'Name'} eq 'ExampleP');
ok(0, $csr_a->{'Database'} eq $dbh);

ok(0, "@{$csr_b->{NAME_lc}}" eq "mode size name");	# before NAME
ok(0, "@{$csr_b->{NAME_uc}}" eq "MODE SIZE NAME");
ok(0, "@{$csr_b->{NAME}}"    eq "mode size name");

# get a dir always readable on all platforms
my $dir = getcwd() || cwd();
$dir = VMS::Filespec::unixify($dir) if $^O eq 'VMS';
# untaint $dir
$dir =~ m/(.*)/; $dir = $1|| die;

my($col0, $col1, $col2, $rows);
my(@row_a, @row_b);

#$csr_a->trace(2);
ok(0, $csr_a->bind_columns(undef, \($col0, $col1, $col2)) );
ok(0, $csr_a->execute( $dir ));
ok(0, $csr_a->{Taint} = 1);

@row_a = $csr_a->fetchrow_array;
ok(0, @row_a);

# check bind_columns
ok(0, $row_a[0] eq $col0);
ok(0, $row_a[1] eq $col1);
ok(0, $row_a[2] eq $col2);

# Check Taint attribute works. This requires this test to be run
# manually with the -T flag: "perl -T -Mblib t/examp.t"
sub is_tainted {
	my $foo;
    return ! eval { ($foo=join('',@_)), kill 0; 1; };
}
if (is_tainted($^X)) {
    print "Taint attribute test enabled\n";
    ok(0, is_tainted($row_a[0]) );
    ok(0, is_tainted($row_a[1]) );
    ok(0, is_tainted($row_a[2]) );
	# check simple method call values
    ok(0, 1);
	# check simple attribute values
    ok(0, 1); # is_tainted($dbh->{AutoCommit}) );
	# check nested attribute values (where a ref is returned)
    #ok(0, is_tainted($csr_a->{NAME}->[0]) );
	# check checking for tainted values
	eval { $dbh->prepare($^X); 1; };
	ok(0, $@ =~ /Insecure dependency/, $@);
	eval { $csr_a->execute($^X); 1; };
	ok(0, $@ =~ /Insecure dependency/, $@);
undef $@;
}
else {
    print "Taint attribute tests skipped\n";
    ok(0,1) while (1..7);
}
$csr_a->{Taint} = 0;

ok(0, $csr_b->bind_param(1, $dir));
ok(0, $csr_b->execute());
@row_b = @{ $csr_b->fetchrow_arrayref };
ok(0, @row_b);

ok(0, "@row_a" eq "@row_b");
@row_b = $csr_b->fetchrow_array;
ok(0, "@row_a" ne "@row_b");

ok(0, $csr_a->finish);
ok(0, $csr_b->finish);

#$csr_b->trace(4);
ok(0, $csr_b->execute());
my $row_b = $csr_b->fetchrow_hashref('NAME_uc');
#$csr_b->trace(0);
ok(0, $row_b);
ok(0, $row_b->{MODE} == $row_a[0]);
ok(0, $row_b->{SIZE} == $row_a[1]);
ok(0, $row_b->{NAME} eq $row_a[2]);

$csr_a = undef;	# force destruction of this cursor now
ok(0, 1);

ok(0, $csr_b->execute());
$r = $csr_b->fetchall_arrayref;
ok(0, $r);
ok(0, @$r);
ok(0, $r->[0]->[0] == $row_a[0]);
ok(0, $r->[0]->[1] == $row_a[1]);
ok(0, $r->[0]->[2] eq $row_a[2]);

ok(0, $csr_b->execute());
$r = $csr_b->fetchall_arrayref([2,1]);
ok(0, $r && @$r);
ok(0, $r->[0]->[1] == $row_a[1]);
ok(0, $r->[0]->[0] eq $row_a[2]);

ok(0, $csr_b->execute());
#$csr_b->trace(9);
$r = $csr_b->fetchall_arrayref({ Size=>1, NAME=>1});
ok(0, $r && @$r);
ok(0, $r->[0]->{Size} == $row_a[1]);
ok(0, $r->[0]->{NAME} eq $row_a[2]);

#$csr_b->trace(4);
ok(0, $csr_b->execute());
$r = $csr_b->fetchall_arrayref({});
ok(0, $r);
ok(0, keys %{$r->[0]} == 3);
ok(0, "@{$r->[0]}{qw(mode size name)}" eq "@row_a", "'@{$r->[0]}{qw(mode size name)}' ne '@row_a'");
#$csr_b->trace(0);

# use Data::Dumper; warn Dumper([\@row_a, $r]);

$rows = $csr_b->rows;
ok(0, $rows > 0, "row count $rows");
ok(0, $rows == @$r, "$rows vs ".@$r);

# ---

@row_b = $dbh->selectrow_array($std_sql, undef, $dir);
ok(0, @row_b == 3);
ok(0, "@row_b" eq "@row_a");

$r = $dbh->selectrow_hashref($std_sql, undef, $dir);
ok(0, keys %$r == 3);
ok(0, $r->{mode} eq $row_a[0]);
ok(0, $r->{size} eq $row_a[1]);
ok(0, $r->{name} eq $row_a[2]);

$r = $dbh->selectall_arrayref($std_sql, undef, $dir);
ok(0, $r);
ok(0, @{$r->[0]} == 3);
ok(0, "@{$r->[0]}" eq "@row_a");
ok(0, @$r == $rows);

$r = $dbh->selectall_hashref($std_sql, undef, $dir);
ok(0, $r);
ok(0, keys %{$r->[0]} == 3);
ok(0, "@{$r->[0]}{qw(mode size name)}" eq "@row_a");
ok(0, @$r == $rows);

$r = $dbh->selectcol_arrayref($std_sql, undef, $dir);
ok(0, $r);
ok(0, @$r == $rows);
ok(0, $r->[0] eq $row_b[0]);

# ---

my $csr_c;
$csr_c = $dbh->prepare("select unknown_field_name1 from ?");
ok(0, !defined $csr_c);
ok(0, $DBI::errstr =~ m/Unknown field names: unknown_field_name1/);

print "RaiseError & PrintError & ShowErrorStatement\n";
$dbh->{RaiseError} = 1;
$dbh->{ShowErrorStatement} = 1;
ok(0, $dbh->{RaiseError});
ok(0, $dbh->{ShowErrorStatement});
my $error_sql = "select unknown_field_name2 from ?";
ok(0, ! eval { $csr_c = $dbh->prepare($error_sql); 1; });
#print "$@\n";
ok(0, $@ =~ m/Unknown field names: unknown_field_name2/, $@);
ok(0, $@ =~ m/\Q$error_sql/, $@); # ShowErrorStatement
$dbh->{RaiseError} = 0;
ok(0, !$dbh->{RaiseError});
$dbh->{ShowErrorStatement} = 0;
ok(0, !$dbh->{ShowErrorStatement});

{
  my @warn;
  local($SIG{__WARN__}) = sub { push @warn, @_ };
  $dbh->{PrintError} = 1;
  ok(0, $dbh->{PrintError});
  ok(0, ! $dbh->prepare("select unknown_field_name3 from ?"));
  ok(0, "@warn" =~ m/Unknown field names: unknown_field_name3/);
  $dbh->{PrintError} = 0;
  ok(0, !$dbh->{PrintError});
}

print "dump_results\n";
ok(0, $csr_a = $dbh->prepare($std_sql));
if ($haveFileSpec && length(File::Spec->rootdir))
{
  ok(0, $csr_a->execute(File::Spec->rootdir));
} else {
  ok(0, $csr_a->execute('/'));
}
my $dump_dir = ($ENV{TMP} || $ENV{TEMP} || $ENV{TMPDIR} 
               || $ENV{'SYS$SCRATCH'} || '/tmp');
my $dump_file = ($haveFileSpec)
    ? File::Spec->catfile($dump_dir, 'dumpcsr.tst')
    : "$dump_dir/dumpcsr.tst";
($dump_file) = ($dump_file =~ m/^(.*)$/);	# untaint
if (open(DUMP_RESULTS, ">$dump_file")) {
	ok(0, $csr_a->dump_results("10", "\n", ",\t", \*DUMP_RESULTS));
	close(DUMP_RESULTS);
	ok(0, -s $dump_file > 0);
} else {
	warn "# dump_results test skipped: unable to open $dump_file: $!\n";
	ok(0, 1);
	ok(0, 1);
}
unlink $dump_file;


print "table_info\n";
# First generate a list of all subdirectories
$dir = $haveFileSpec ? File::Spec->curdir() : ".";
ok(0, opendir(DIR, $dir));
my(%dirs, %unexpected, %missing);
while (defined(my $file = readdir(DIR))) {
    $dirs{$file} = 1 if -d $file;
}
closedir(DIR);
my $sth = $dbh->table_info();
ok(0, $sth);
%unexpected = %dirs;
%missing = ();
while (my $ref = $sth->fetchrow_hashref()) {
    if (exists($unexpected{$ref->{'TABLE_NAME'}})) {
	delete $unexpected{$ref->{'TABLE_NAME'}};
    } else {
	$missing{$ref->{'TABLE_NAME'}} = 1;
    }
}
ok(0, keys %unexpected == 0)
    or print "Unexpected directories: ", join(",", keys %unexpected), "\n";
ok(0, keys %missing == 0)
    or print "Missing directories: ", join(",", keys %missing), "\n";

# Test the tables method
%unexpected = %dirs;
%missing = ();
foreach my $table ($dbh->tables()) {
    if (exists($unexpected{$table})) {
	delete $unexpected{$table};
    } else {
	$missing{$table} = 1;
    }
}
ok(0, keys %unexpected == 0)
    or print "Unexpected directories: ", join(",", keys %unexpected), "\n";
ok(0, keys %missing == 0)
    or print "Missing directories: ", join(",", keys %missing), "\n";


for (my $i = 0;  $i < 300;  $i += 100) {
	print "Testing the fake directories ($i).\n";
    ok(0, $csr_a = $dbh->prepare("SELECT name, mode FROM long_list_$i"));
    ok(0, $csr_a->execute(), $DBI::errstr);
    my $ary = $csr_a->fetchall_arrayref;
    ok(0, @$ary == $i, @$ary." rows instead of $i");
    if ($i) {
	my @n1 = map { $_->[0] } @$ary;
	my @n2 = reverse map { "file$_" } 1..$i;
	ok(0, "@n1" eq "@n2", "'@n1' ne '@n2'");
    }
    else {
	ok(0,1);
    }
}

DBI->disconnect_all;	# doesn't do anything yet
ok(0, 1);

# Test the $dbh->func method
print "Testing \$dbh->func().\n";
my %tables = map { $_ =~ /lib/ ? ($_, 1) : () } $dbh->tables();
my $ok = 1;
foreach my $t ($dbh->func('lib', 'examplep_tables')) {
    defined(delete $tables{$t}) or print "Unexpected table: $t\n";
}
ok(0, (%tables == 0));

BEGIN { $tests = 127; }
