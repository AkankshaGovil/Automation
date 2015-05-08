
use DBI;
$|=1;

print "1..$tests\n";

{   package DBD::Test;

    $drh = undef;	# holds driver handle once initialised

    sub driver{
	return $drh if $drh;
	print "ok 1\n";		# just getting here is enough!
	my($class, $attr) = @_;
	$class .= "::dr";
	($drh) = DBI::_new_drh($class, {
		'Name' => 'Test',
		'Version' => '$Revision: 1.1.2.1 $',
	    },
	    77	# 'implementors data'
	    );
	print "ok 2\n";		# just getting here is enough!
	$drh;
    }
}

{   package DBD::Test::dr;
    $imp_data_size = 0;
    $imp_data_size = 0;	# avoid typo warning

    sub disconnect_all { undef }
    sub DESTROY { undef }
}

$INC{'DBD/Test.pm'} = 'dummy';	# fool require in install_driver()

# Note that install_driver should *not* normally be called directly.
# This test does so only because it's a test of install_driver!
$drh = DBI->install_driver('Test');
($drh) ? print "ok 3\n" : print "not ok 3\n";

(DBI::_get_imp_data($drh) == 77) ? print "ok 4\n" : print "not ok 4\n";

foreach (5..9) { print "ok $_\n"; }

DBI::set_err($drh, "99", "foo");
($DBI::err == 99)        ? print "ok 10\n" : print "not ok 10\n";
($DBI::errstr eq "foo")  ? print "ok 11\n" : print "not ok 11\n";

DBI::set_err($drh, 0, "00000");
($DBI::state eq "")      ? print "ok 12\n" : print "not ok 12\n";

DBI::set_err($drh, 1, "test error");
($DBI::state eq "S1000") ? print "ok 13\n" : print "not ok 13\n";

DBI::set_err($drh, 1, "test error", "IM999");
($DBI::state eq "IM999") ? print "ok 14\n" : print "not ok 14\n";

eval { $DBI::rows = 1 };
($@ =~ m/Can't modify/)  ? print "ok 15\n" : print "not ok 15\n";


BEGIN { $tests = 15 }
exit 0;
