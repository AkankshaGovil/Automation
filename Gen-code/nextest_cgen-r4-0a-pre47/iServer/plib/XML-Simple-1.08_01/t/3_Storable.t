# $Id: 3_Storable.t,v 1.1 2002/04/09 15:16:07 sridhar Exp $

use strict;

use File::Spec;

eval { require Storable; };
unless($INC{'Storable.pm'}) {
  print STDERR "no Storable.pm...";
  print "1..0\n";
  exit 0;
}

# Initialise filenames and check they're there

my $SrcFile   = File::Spec->catfile('t', 'desertnet.src');
my $XMLFile   = File::Spec->catfile('t', 'desertnet.xml');
my $CacheFile = File::Spec->catfile('t', 'desertnet.stor');

unless(-e $SrcFile) {
  print STDERR "test data missing...";
  print "1..0\n";
  exit 0;
}

print "1..19\n";

my $t = 1;

##############################################################################
#                   S U P P O R T   R O U T I N E S
##############################################################################

##############################################################################
# Print out 'n ok' or 'n not ok' as expected by test harness.
# First arg is test number (n).  If only one following arg, it is interpreted
# as true/false value.  If two args, equality = true.
#

sub ok {
  my($n, $x, $y) = @_;
  die "Sequence error got $n expected $t" if($n != $t);
  $x = 0 if(@_ > 2  and  $x ne $y);
  print(($x ? '' : 'not '), 'ok ', $t++, "\n");
}


##############################################################################
# Take two scalar values (may be references) and compare them (recursively
# if necessary) returning 1 if same, 0 if different.
#

sub DataCompare {
  my($x, $y) = @_;

  my($i);

  if(!ref($x)) {
    return(1) if($x eq $y);
    print STDERR "$t:DataCompare: $x != $y\n";
    return(0);
  }

  if(ref($x) eq 'ARRAY') {
    unless(ref($y) eq 'ARRAY') {
      print STDERR "$t:DataCompare: expected arrayref, got: $y\n";
      return(0);
    }
    if(scalar(@$x) != scalar(@$y)) {
      print STDERR "$t:DataCompare: expected ", scalar(@$x),
                   " element(s), got: ", scalar(@$y), "\n";
      return(0);
    }
    for($i = 0; $i < scalar(@$x); $i++) {
      DataCompare($x->[$i], $y->[$i]) || return(0);
    }
    return(1);
  }

  if(ref($x) eq 'HASH') {
    unless(ref($y) eq 'HASH') {
      print STDERR "$t:DataCompare: expected hashref, got: $y\n";
      return(0);
    }
    if(scalar(keys(%$x)) != scalar(keys(%$y))) {
      print STDERR "$t:DataCompare: expected ", scalar(keys(%$x)),
                   " key(s) (", join(', ', keys(%$x)),
		   "), got: ",  scalar(keys(%$y)), " (", join(', ', keys(%$y)),
		   ")\n";
      return(0);
    }
    foreach $i (keys(%$x)) {
      unless(exists($y->{$i})) {
	print STDERR "$t:DataCompare: missing hash key - {$i}\n";
	return(0);
      }
      DataCompare($x->{$i}, $y->{$i}) || return(0);
    }
    return(1);
  }

  print STDERR "Don't know how to compare: " . ref($x) . "\n";
  return(0);
}


##############################################################################
# Copy a file
#

sub CopyFile {
  my($Src, $Dst) = @_;
  
  open(IN, $Src) || return(undef);
  local($/) = undef;
  my $Data = <IN>;
  close(IN);

  open(OUT, ">$Dst") || return(undef);
  print OUT $Data;
  close(OUT);

  return(1);
}


##############################################################################
# Wait until the current time is greater than the supplied value
#

sub PassTime {
  my($Target) = @_;

  while(time <= $Target) {
    sleep 1;
  }
}


##############################################################################
#                      T E S T   R O U T I N E S
##############################################################################

use XML::Simple;

# Initialise test data

my $Expected  = {
          'server' => {
                        'sahara' => {
                                      'osversion' => '2.6',
                                      'osname' => 'solaris',
                                      'address' => [
                                                     '10.0.0.101',
                                                     '10.0.1.101'
                                                   ]
                                    },
                        'gobi' => {
                                    'osversion' => '6.5',
                                    'osname' => 'irix',
                                    'address' => '10.0.0.102'
                                  },
                        'kalahari' => {
                                        'osversion' => '2.0.34',
                                        'osname' => 'linux',
                                        'address' => [
                                                       '10.0.0.103',
                                                       '10.0.1.103'
                                                     ]
                                      }
                      }
        };

ok(1, CopyFile($SrcFile, $XMLFile));  # Start with known source file
unlink($CacheFile);                   # Ensure there are ...
ok(2, ! -e $CacheFile);               # ... no cache files lying around

my $opt = XMLin($XMLFile);
ok(3, DataCompare($opt, $Expected));  # Got what we expected
ok(4, ! -e $CacheFile);               # And no cache file was created
PassTime(time());                     # Ensure cache file will be newer

$opt = XMLin($XMLFile, cache => 'storable');
ok(5, DataCompare($opt, $Expected));  # Got what we expected again
ok(6, -e $CacheFile);                 # But this time a cache file was created
my $t0 = (stat($CacheFile))[9];       # Remember cache timestamp
PassTime($t0);

$opt = XMLin($XMLFile, cache => 'storable');
ok(7, DataCompare($opt, $Expected));  # Got what we expected from the cache
my $t1 = (stat($CacheFile))[9];       # Check cache timestamp
ok(8, $t0, $t1);                      # has not changed

PassTime(time());
$t0 = time();
open(FILE, ">>$XMLFile");             # Touch the XML file
print FILE "\n";
close(FILE);
$opt = XMLin($XMLFile, cache => 'storable');
ok(9, DataCompare($opt, $Expected));  # Got what we expected
my $t2 = (stat($CacheFile))[9];       # Check cache timestamp
ok(10, $t1 != $t2);                   # has changed

unlink($XMLFile);
ok(11, ! -e $XMLFile);                # Original XML file is gone
open(FILE, ">$XMLFile");              # Re-create it (empty)
close(FILE);
PassTime((stat($XMLFile))[9]);        # But ensure cache file is newer
unlink($CacheFile);                   # Seems to be rqd for test on Win32
Storable::nstore($Expected, $CacheFile);
$opt = XMLin($XMLFile, cache => 'storable');
ok(12, DataCompare($opt, $Expected)); # Got what we expected from the cache
ok(13, ! -s $XMLFile);                # even though the XML file is empty
$t2 = (stat($CacheFile))[9];
PassTime($t2);
open(FILE, ">$XMLFile") ||            # Write some new data to the XML file
  die "open(>$XMLFile): $!\n";
print FILE qq(<opt one="1" two="2"></opt>\n);
close(FILE);

$opt = XMLin($XMLFile);               # Parse with no caching
ok(14, DataCompare($opt, { one => 1, two => 2})); # Got what we expected
$t0 = (stat($CacheFile))[9];          # And timestamp on cache file
my $s0 = (-s $CacheFile);
ok(15, $t0 == $t2);                   # has not changed

                                      # Parse again with caching enabled
$opt = XMLin($XMLFile, cache => 'storable');
                                      # Came through the cache
ok(16, DataCompare($opt, { one => 1, two => 2}));
$t1 = (stat($CacheFile))[9];          # which has been updated
my $s1 = (-s $CacheFile);
ok(17, ($t0 != $t1) || ($s0 != $s1)); # Content changes but date may not on Win32

ok(18, CopyFile($SrcFile, $XMLFile)); # Put back the original file
PassTime($t1);
$opt = XMLin($XMLFile, cache => 'storable');
ok(19, DataCompare($opt, $Expected)); # Got what we expected

# Clean up and go

unlink($CacheFile);
unlink($XMLFile);
exit(0);

