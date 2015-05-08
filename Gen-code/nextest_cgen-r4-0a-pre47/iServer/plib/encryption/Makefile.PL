use  ExtUtils::MakeMaker;

sub MY::install {
	package MY;
	my $inherited = shift->SUPER::install(@_);
	$inherited =~ s/(install ::.*)\n/$1 install_encrypt\n/;
	$inherited = $inherited.$self->{NOECHO}.q{
install_encrypt :: .DUMMY
	(cp encr ../../nars-agent/)
	(cp decr ../../nars-agent/)

.DUMMY ::
};
		
	$inherited;
}

WriteMakefile(
	NAME      => 'NexTone::decrypt',
	VERSION_FROM   => 'decrypt.pm',

	# The line below disables both the dynamic link test and the
	# test for DEBUGGING.
	# It is only enabled here to allow the decrypt test harness
	# to run without having to build statically.
	DEFINE	  => "-DBYPASS",
);
