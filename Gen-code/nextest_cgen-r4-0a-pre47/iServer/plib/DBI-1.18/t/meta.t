#!../perl

$|=1;
print "1..$tests\n";

sub ok ($$;$) {
    my($n, $got, $want) = @_;
    ++$t;
    die "sequence error, expected $n but actually $t"
	if $n and $n != $t;
    return print "ok $t\n" if @_<3 && $got;
    return print "ok $t\n" if $got eq $want;
    warn "Test $n: wanted '$want', got '$got'\n";
    print "not ok $t\n";
}

use DBI qw(:sql_types);

$dbh = DBI->connect("dbi:ExampleP:.","","");
die "Unable to connect to ExampleP driver: $DBI::errstr" unless $dbh;
ok(0, $dbh);
#$dbh->trace(3);

#use Data::Dumper;
#print Dumper($dbh->type_info_all);
#print Dumper($dbh->type_info);
#print Dumper($dbh->type_info(DBI::SQL_INTEGER));

my @ti = $dbh->type_info;
ok(0, @ti>0);
ok(0, $dbh->type_info(SQL_INTEGER)->{DATA_TYPE}, SQL_INTEGER);
ok(0, $dbh->type_info(SQL_INTEGER)->{TYPE_NAME}, 'INTEGER');

ok(0, $dbh->type_info(SQL_VARCHAR)->{DATA_TYPE}, SQL_VARCHAR);
ok(0, $dbh->type_info(SQL_VARCHAR)->{TYPE_NAME}, 'VARCHAR');


BEGIN { $tests = 6 }
