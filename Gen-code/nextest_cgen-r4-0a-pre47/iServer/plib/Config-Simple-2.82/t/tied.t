use strict;
use Test;
use Config::Simple;
use File::Spec;

ok(1);

my $filename = File::Spec->catfile("t", "sample.cfg");
tie my %Config, "Config::Simple::Tie", $filename, O_RDONLY or die $!;

ok(tied(%Config));

ok($Config{'module.name'}, "Config::Simple");

while ( my ($name, $value) = each %Config) {
    ok($Config{$name}, $value);
}

$Config{"module.name"} = "Config::Simple";

delete $Config{"module.name"};

my $worked = $Config{"module.name"} ? 0 : 1;
ok($worked);

$Config{"module.name"} = "Config::Simple";

ok($Config{'module.name'}, "Config::Simple");

$Config{"module.name"} = "Nice module";
ok($Config{"module.name"}, "Nice module");

$Config{"module.name"} = "Config::Simple";

$Config{"module.version"} = "2.4";
ok($Config{'module.version'}, "2.4");

ok(scalar keys %Config, 9);


ok($Config{"author.email"}, 'sherzodr@cpan.org');
ok($Config{'author.url'}, 'http://www.ultracgis.com');
BEGIN {
	
	plan test=>19;
	$^W = 0;

};

