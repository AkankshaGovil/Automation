# makepm.PL -- Builds Rsync.pm from Rsync.in.  -*- perl -*-
sub find_rsync {
   my @dirs = map { s/^$/./; $_ } split (':', $ENV{PATH});
   push (@dirs, qw(/usr/local/bin /usr/bin /opt/bin));
   for my $dir (@dirs) {
      next unless -d $dir;
      next unless -x "$dir/rsync";
      return "$dir/rsync";
   }
}
$|=1;
$RSYNC=&find_rsync;
print <<'EOT';

File::Rsync needs to know the path to the rsync binary.  This path is encoded
in the installed module as the default path to rsync (it can be overridden
at runtime).  Please enter the full path to the rsync program or just hit
Enter if the guess is correct.  (If you always want the module to depend on
the $PATH environment variable at runtime, just set the path to 'rsync'
(this is not recommended)).

EOT
$RSYNC||='/usr/local/bin/rsync';
# The following lines are commented so that rsync path is not prompted 
# START CHANGE
# print "Path to rsync [$RSYNC]: ";
# my $ans=<STDIN>;
# chomp $ans;
# $RSYNC=$ans || $RSYNC;
$RSYNC='/usr/local/bin/rsync';
# END CHANGE

$RSYNC=~s/\\/\\\\/g;
$RSYNC=~s/\'/\\\'/g;
open IN,'Config.in' or die "Cannot read Config.in: $!\n";
open OUT,'>Config.pm' or die "Cannot write Config.pm: $!\n";
while (<IN>) {
   if (/^(\s*rsync_path\s*=>)/) { print OUT "$1 '$RSYNC',\n" }
   else                                  { print OUT }
}
close IN;
close OUT;
