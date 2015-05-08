package NexTone::decrypt ;

require 5.002 ;
require DynaLoader;
use strict;
use warnings;
use vars qw(@ISA $VERSION);
@ISA = qw(DynaLoader);
$VERSION = "1.04" ;

bootstrap NexTone::decrypt ;
1;
__END__

=head1 NAME

NexTone::decrypt - NexTone decrypt source module

=head1 SYNOPSIS

    use NexTone::decrypt ;

=head1 DESCRIPTION

This package is copyrighted by NexTone Communications Inc.

It is used for decrypting NexTones encrypted spurce modules
during runtime. Any usage of this module as a whole or partially
is restricted according to the following LICENSE

=head1 WARNING


=head1 AUTHOR

Paritosh  Tyagi 

=head1 DATE

22 October, 2002

=cut
