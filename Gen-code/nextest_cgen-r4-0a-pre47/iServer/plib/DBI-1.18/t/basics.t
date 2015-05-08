#!../../perl -w

$|=1;

print "1..$tests\n";

sub ok ($$) {
    my($n, $ok) = @_;
    ++$t;
    die "sequence error, expected $n but actually $t"
		if $n and $n != $t;
	#warn "test $t done.\n";
    ($ok) ? print "ok $t\n" : print "not ok $t\n";
}


use DBI qw(:sql_types :utils);
ok(0, 1);
ok(0, 1);

$switch = DBI->internal;
ok(0, ref $switch eq 'DBI::dr');

@drivers = DBI->available_drivers(); # at least 'ExampleP' should be installed
ok(0, @drivers);
ok(0, "@drivers" =~ m/ExampleP/i);	# ignore case for VMS & Win32

$switch->debug(0);
$switch->{DebugDispatch} = 0;	# handled by Switch
$switch->{Warn} = 1;			# handled by DBI core

ok(0, 1);
ok(7, $switch->{'Attribution'} =~ m/DBI.*? by Tim Bunce/);
ok(8, $switch->{'Version'} > 0);

eval { $switch->{FooBarUnknown} = 1 };
ok(9,  $@ =~ /Can't set/);

eval { $_=$switch->{BarFooUnknown} };
ok(10, $@ =~ /Can't get/);

ok(11, $switch->{private_test1} = 1);
ok(12, $switch->{private_test1} == 1);

ok(13, !defined $switch->{CachedKids});
ok(14, $switch->{CachedKids} = { });
ok(15, ref $switch->{CachedKids} eq 'HASH');
ok(16, ref $switch->{CachedKids} eq 'HASH');

ok(0, $switch->{Kids} == 0);
ok(0, $switch->{ActiveKids} == 0);
ok(0, $switch->{Active});

$switch->trace_msg("Test \$h->trace_msg text.\n", 1);
DBI->trace_msg("Test DBI->trace_msg text.\n", 1);

ok(0, SQL_VARCHAR == 12);
ok(0, SQL_ALL_TYPES == 0);
ok(0, neat(1+1) eq "2");
ok(0, neat("2") eq "'2'");
ok(0, neat(undef) eq "undef");
ok(0, neat_list([1+1, "2", undef, "foobarbaz"], 8, "|") eq "2|'2'|undef|'foo...'");

my @is_num = looks_like_number(undef, "", "foo", 1, ".");
ok(0, !defined $is_num[0]);	# undef -> undef
ok(0, !defined $is_num[1]);	# "" -> undef (eg "don't know")
ok(0,  defined $is_num[2]);	# "foo" -> defined false
ok(0,         !$is_num[2]);	# "foo" -> defined false
ok(0,          $is_num[3]); # 1 -> true
ok(0,         !$is_num[4]); # "." -> false

BEGIN { $tests = 31 }
exit 0;
