# -*- perl -*-

use strict;

my $script = <<'SCRIPT';
~startperl~ -w

use strict;

use DBI::Shell;
shell(@ARGV);
exit(0);

__END__

=head1 NAME

dbish	- Interactive command shell for the Perl DBI

=head1 SYNOPSIS

  dbish <options> dsn [user [password]]

=head1 DESCRIPTION

This tool is a command wrapper for the DBI::Shell perl module.
See L<DBI::Shell(3)> for full details.

=head1 SEE ALSO

L<DBI::Shell(3)>, L<DBI(3)>

=cut
SCRIPT

use Config;

my $file = __FILE__; $file =~ s/\.PL$//;

$script =~ s/\~(\w+)\~/$Config{$1}/eg;
if (!(open(FILE, ">$file"))  ||
    !(print FILE $script)  ||
    !(close(FILE))) {
    die "Error while writing $file: $!\n";
}
print "Extracted $file from ",__FILE__," with variable substitutions.\n";

# end
