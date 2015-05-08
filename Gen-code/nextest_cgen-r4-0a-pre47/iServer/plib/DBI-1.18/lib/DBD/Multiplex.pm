
my $mplx_trace = 0;

# vim:ts=4
{
    package DBD::Multiplex;

    require DBI;
    require Carp;

    @EXPORT = ();
    $VERSION = substr(q$Revision: 1.1.2.1 $, 9,-1) -1;

#   $Id: Multiplex.pm,v 1.1.2.1 2002/12/18 19:35:09 santosh Exp $
#
#   Copyright (c) 1999, Tim Bunce & Thomas Kishel
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.

    $drh = undef;       # holds driver handle once initialised
    $err = 0;           # The $DBI::err value

    sub driver {
        return $drh if $drh;
        my($class, $attr) = @_;
        $class .= "::dr";
        ($drh) = DBI::_new_drh($class, {
            'Name' => 'Multiplex',
            'Version' => $VERSION,
            'Attribution' => 'DBD Multiplex by Tim Bunce & Thomas Kishel',
	});
        return $drh;
    }


    # functions for calling a method for each child handle

    sub mplx_do_calls {		# 'bottom-level' support function to do the calls
        my ($method, $handles, $wantarray, $opts) = (shift,shift,shift,shift);
	my $errh = $opts->{errh};	# handle to record error on
	my $stop = $opts->{stop_on}||'';# stop on first 'err' or 'ok' else do all
	my $trace= $opts->{trace};
        my @results;
	my @errors;	# sparse array paralleling $results[0..n]
	warn "mplx_do_calls $method for (@$handles)" if $trace;
	foreach my $h (@$handles) {        # child handle
	    push @results, ($wantarray) ? [        $h->$method(@_) ]
				        : [ scalar $h->$method(@_) ];
	    if (my $err = $h->err) {
		my $errstr = $h->errstr;
		$errors[ @results-1 ] = [ $err, $errstr ];
		DBI::set_err($errh, $err, $errstr) if $errh;
		warn "mplx_do_calls $h->$method ERROR: $err, $errstr" if $trace;
		last if $stop eq 'err';
	    }
	    else {
		warn "mplx_do_calls $h->$method OK: @{ $results[-1] }" if $trace;
		last if $stop eq 'ok';
	    }
	}
	return (\@results, \@errors);
    }

    #	XXX need variants/flags for
    #	- call-each-till-first-success-then-stop
    #	- call-all-return-first-success-else-error
    #	- call-all-return-most-common-result (eg three-way-voting etc)
    #	In some cases the default behaviour may depend on the method being called.
    #	In others the application may want to override that for specific cases
    #	(using \%attrib)
    sub mplx_method_all {
	my $method  = shift;
	my $ph      = shift;
	my $handles = $ph->{mplx_h} || die;

	my %opts = ( errh => $ph, stop_on => 'err', trace => $mplx_trace );

        my ($results, $errors)
	    = mplx_do_calls($method, $handles, wantarray, \%opts, @_);

	my $result = $results->[0];	# pick results to return
	return $result->[0] unless wantarray;
	return @$result;
    }

}



{   package DBD::Multiplex::dr; # ====== DRIVER ======
    $imp_data_size = 0;

    sub connect { my ($drh, $dsn, $user, $auth, $attr) = @_;

	# XXX parse $dsn and make multiple connects
	# need to define good syntax
	my $dbh1 = DBI->connect($dsn, $user, $auth, $attr);
	return DBI::set_err($drh, $DBI::err, $DBI::errstr) unless $dbh1;

	my ($this) = DBI::_new_dbh($drh, {
	    Name => $dsn,
	    User => $user,
	    mplx_h => [ $dbh1 ],
	});

	return $this;
    }

    sub disconnect_all { }
    sub DESTROY { }
}



{   package DBD::Multiplex::db; # ====== DATABASE ======
    $imp_data_size = 0;

    use strict;

    sub prepare {
	my $dbh = shift;
	my ($statement, $attribs) = @_;

	my $handles = $dbh->{mplx_h} || die;
	my %opts = ( errh => $dbh );
        my ($results, $errors)
	    = DBD::Multiplex::mplx_do_calls('prepare', $handles, wantarray, \%opts, @_);
	return if @$errors;

	my ($outer, $sth) = DBI::_new_sth($dbh, {
	    'Statement'   => $statement,
	    mplx_h => [ map { $_->[0] } @$results ],
	});
	$outer;
    }

    # XXX replace this with dynamic info from updated DBI
    # XXX needs expanding manually in the short term
    use subs qw(FETCH STORE DESTROY);

    sub AUTOLOAD {
	my $method = $DBD::Multiplex::db::AUTOLOAD;
	$method =~ s/^DBD::Multiplex::db:://;
	warn "db AUTOLOAD $method(@_)" if $mplx_trace;

	my @results = (wantarray)
		? (       DBD::Multiplex::mplx_method_all($method, @_))
		: (scalar DBD::Multiplex::mplx_method_all($method, @_));

	return $results[0] unless wantarray;
	return @results;
    }

}


{   package DBD::Multiplex::st; # ====== STATEMENT ======
    $imp_data_size = 0;
    use strict;

    # XXX replace this with dynamic info from updated DBI
    # XXX needs expanding manually in the short term
    use subs qw(execute fetch fetchrow_arrayref finish STORE FETCH DESTROY);

    sub AUTOLOAD {
	my $method = $DBD::Multiplex::st::AUTOLOAD;
	$method =~ s/^DBD::Multiplex::st:://;
	warn "st AUTOLOAD $method(@_)" if $mplx_trace;

	my @results = (wantarray)
		? (       DBD::Multiplex::mplx_method_all($method, @_))
		: (scalar DBD::Multiplex::mplx_method_all($method, @_));

	return $results[0] unless wantarray;
	return @results;
    }

}

1;
__END__

=head1 NAME

DBD::Multiplex - A DBI driver multiplexer

=head1 SYNOPSIS

  use DBI;

  $dbh = DBI->connect("dbi:Multiplex:dsn", $user, $passwd);

  # See the DBI module documentation for full details

=head1 DESCRIPTION

To be written

=cut
