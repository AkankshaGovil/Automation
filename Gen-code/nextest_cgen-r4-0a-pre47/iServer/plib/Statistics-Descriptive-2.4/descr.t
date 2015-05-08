
require 'Descriptive.pm';

print "1..9\n";

$testct = 1;

# test #1

$stat = Statistics::Descriptive::Full->new();
@result = $stat->least_squares_fit();
print ( (@result? 'not ': '' ) . 'ok ' . $testct++ . "\n" );

# test #2
# data are y = 2*x - 1

$stat->add_data( 1, 3, 5, 7 );
@result = $stat->least_squares_fit();
$ok = ( $result[0] == -1 ) && ( $result[1] == 2 );
print ( ($ok? '': 'not ' ) . 'ok ' . $testct++ . "\n" );

# test #3
# test error condition on harmonic mean : one element zero
$stat = Statistics::Descriptive::Full->new();
$stat->add_data( 1.1, 2.9, 4.9, 0.0 );
$result = $stat->harmonic_mean();
$ok = ! defined( $result );
print ( ($ok? '': 'not ' ) . 'ok ' . $testct++ . "\n" );

# test #4
# test error condition on harmonic mean : sum of elements zero
$stat = Statistics::Descriptive::Full->new();
$stat->add_data( 1.0, -1.0 );
$result = $stat->harmonic_mean();
$ok = ! defined( $result );
print ( ($ok? '': 'not ' ) . 'ok ' . $testct++ . "\n" );

# test #5
# test error condition on harmonic mean : sum of elements near zero
$stat = Statistics::Descriptive::Full->new();
$savetol = $Statistics::Descriptive::Tolerance;
$Statistics::Descriptive::Tolerance = 0.1;
$stat->add_data( 1.01, -1.0 );
$result = $stat->harmonic_mean();
$ok = ! defined( $result );
print ( ($ok? '': 'not ' ) . 'ok ' . $testct++ . "\n" );

$Statistics::Descriptive::Tolerance = $savetol;

# test #6
# test normal function of harmonic mean
$stat = Statistics::Descriptive::Full->new();
$stat->add_data( 1,2,3 );
$result = $stat->harmonic_mean();
$ok = defined( $result ) && abs( $result - 1.6363 ) < 0.001;
print ( ($ok? '': 'not ' ) . 'ok ' . $testct++ . "\n" );

# test #7
# test stringification of hash keys in frequency distribution
$stat = Statistics::Descriptive::Full->new();
$stat->add_data(0.1,
                0.15,
                0.16,
               1/3);
%f = $stat->frequency_distribution(2);

$ok = ($f{0.216666666666667} == 3) &&
      ($f{0.333333333333333} == 1);
print ( ($ok? '': 'not ' ) . 'ok ' . $testct++ . "\n" );

# test #8 and #9
# Test the percentile function and caching
$stat = Statistics::Descriptive::Full->new();
$stat->add_data(-5,-2,4,7,7,18);
##Check algorithm
if ( $stat->percentile(50) == 4 ) {
  print "ok 8\n";
}
else {
  print "not ok 8";
}
if ($stat->percentile(25) == -2) {
  print "ok 9\n";
}
else {
  print "not ok 9";
}
