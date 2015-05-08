#!../../perl

$|=1;
my $calls = 0;


# =================================================
# Example code for sub classing the DBI.
#
# Note that the extra ::db and ::st classes must be set up
# as sub classes of the corresponding DBI classes.
#
# This whole mechanism is new and experimental - it may change!

package MyDBI;
@ISA = qw(DBI);

# inherit connect etc


package MyDBI::db;
@ISA = qw(DBI::db);

sub prepare {
    my($dbh, @args) = @_;
    ++$calls;
    my $sth = $dbh->SUPER::prepare(@args);
    return $sth;
}


package MyDBI::st;
@ISA = qw(DBI::st);

sub fetch {
    my($sth, @args) = @_;
    ++$calls;
    my $row = $sth->SUPER::fetch(@args);
    $row->[1] = lc($row->[1]) if $row; # modify data as an example
    return $row;
}


# =================================================
package main;

print "1..$tests\n";

sub ok ($$$) {
    my($n, $got, $want) = @_;
    ++$t;
    die "sequence error, expected $n but actually $t"
	if $n and $n != $t;
    return print "ok $t\n" if $got eq $want;
    warn "Test $n: wanted '$want', got '$got'\n";
    print "not ok $t\n";
}


# =================================================
package main;

use DBI;

# tell the DBI that MyDBI is a new 'root class'
MyDBI->init_rootclass;
ok(0, 1, 1);

$dbh = MyDBI->connect("dbi:Sponge:foo","","");
ok(0, ref $dbh, 'MyDBI::db');
#$dbh->trace(3);

$sth = $dbh->prepare("foo",
	# data for DBD::Sponge to return via fetch
	{ rows => [ [ 42, "AAA", 9 ], [ 43, "BB", 8 ], ] }
);
ok(0, $calls, 1);
ok(0, ref $sth, 'MyDBI::st');

my $row = $sth->fetch;
ok(0, $calls, 2);
ok(0, $row->[1], "aaa");


BEGIN { $tests = 6 }
