
# vim:ts=2:sw=2:ai:aw:nu
{
    package DBD::ADO;

    require DBI;
    require Carp;
		use strict;
		use vars qw($err $errstr $state $drh $VERSION @EXPORT);

    @EXPORT = ();
    $VERSION = substr(q!Revision: 2.0 !, 9,-1) +0;

#   $Id: ADO.pm,v 1.1.2.1 2002/12/18 19:35:09 santosh Exp $
#
#   Copyright (c) 1999, Phlip & Tim Bunce
#   Copyright (c) 2001, Thomas "A." Lowery
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.

    $drh = undef;       # holds driver handle once initialised
    $err = 0;           # The $DBI::err value
		$errstr = "";
		$state = "";

    sub driver{
        return $drh if $drh;
        my($class, $attr) = @_;
        $class .= "::dr";
        ($drh) = DBI::_new_drh($class, {
            'Name' => 'ADO',
            'Version' => $VERSION,
            'Attribution' => 'DBD ADO for Win32 by Phlip & Tim Bunce',
						'Err' => \$DBD::ADO::err,
						'Errstr' => \$DBD::ADO::errstr,
						'State' => \$DBD::ADO::state,
	});
        return $drh;
    }


  	sub errors {
			my $Conn = shift;
			my $err_ary = [];

			my $lastError = Win32::OLE->LastError;
			push @$err_ary, "\nLasterror:\t " . ($lastError+0) . ": $lastError" 
				if $lastError;

			return unless ref $Conn;
			my $Errors = $Conn->Errors();
			# return unless defined $Errors;
			if($Errors && $Errors->{Count}) {
				my $err;
				foreach $err (Win32::OLE::in($Errors)) {
	    		next if $err->{Number} == 0; # Skip warnings
	    		push(@$err_ary, 
						"\tDescription:\t$err->{Description}",
	    			"\tHelpContext:\t$err->{HelpContext}",
						"\tHelpFile:   \t$err->{HelpFile}",
	    			"\tNativeError:\t$err->{NativeError}",
						"\tNumber:     \t$err->{Number}", 
						"\tSource:     \t$err->{Source}",
						"\tSQLState:   \t$err->{SQLState}");
				}
			#return join "\n", @$err_ary;
			}
		$Errors->Clear if $Errors;
		$Conn->Errors->Clear();
		return ($err_ary? join "\n", @$err_ary : undef);
    }

}


# ADO.pm lexically scoped constants
my $ado_consts;
my $VT_I4_BYREF;
my $ado_sptype;
my %connect_options;



{   package DBD::ADO::dr; # ====== DRIVER ======

	use strict;
	use vars qw($imp_data_size);
	$imp_data_size = 0;

	sub DBPROPVAL_TC_ALL { return  8 };
	sub DBPROPVAL_TC_DDL_IGNORE { return 4 };
	sub DBPROPVAL_TC_DDL_COMMIT { return 2 };
	sub DBPROPVAL_TC_DML{ return 1 };
	sub DBPROPVAL_TC_NONE{ return 0 };

    sub connect { 
			my ($drh, $dsn, $user, $auth) = @_;

  	require Win32::OLE;

	unless ($ado_consts) {
	    require Win32::OLE::Const;
	    my $name = "Microsoft ActiveX Data Objects 2\\.\\d+ Library";
	    $ado_consts = Win32::OLE::Const->Load($name)
		or die "Unable to load Win32::OLE::Const ``$name'' ".Win32::OLE->LastError;
	    require Win32::OLE::Variant;
	    $VT_I4_BYREF = Win32::OLE::Variant::VT_I4()
			 | Win32::OLE::Variant::VT_BYREF();

	}

	local $Win32::OLE::Warn = 0;
	my $conn = Win32::OLE->new('ADODB.Connection');
	my $lastError = Win32::OLE->LastError;
	return DBI::set_err($drh, 1,
		"Can't create 'ADODB.Connection': $lastError")
	    if $lastError;

	my $this = DBI::_new_dbh($drh, {
	    	Name => $dsn,
	    	User => $user,
				AutoCommit => 1,
				Warn => 0,
				LongReadLen => 0,
				LongTruncOk => 0,
			}, 
			{ 
				ado_conn => undef,
				Attributes => undef,
				CommandTimeout => undef,
				ConnectionString => undef,
				ConnectionTimeout => undef,
				CursorLocation => undef,
				DefaultDatabase => undef,
				IsolationLevel => undef,
				Mode => undef,
				Provider => undef,
				State => undef,
				Version => undef,
			});

		$this->STORE( ado_conn => $conn );
	$drh->trace_msg( "->ADO Connection: " . ref $this->FETCH('ado_conn') . 
		" Connection: " . ref $conn . "\n", 1);
	##  ODBC rule - Null is not the same as an empty password...
	$auth = '' if !defined $auth;

	my (@cdsn,$cdsn);
	# Run thru the dsn extracting connection options.
	if( $dsn =~ /;/ ) {
	for my $s (split( /;/, $dsn)) {
		if ($s =~ m/^(.*?)=(.*)$/s){
			my ($c, $v) = ($1, $2);
				# Only include the options defined.
					if( $conn->{$c} ) {
						$this->STORE($c, $v);
						$drh->trace_msg("->> Storing $c $v\n", 1);
						next;
					} 
			}
		push(@cdsn, $s );
		}
	} else {
		if($dsn =~ m/^(.*?)=(.*)$/s) {
			$this->STORE( "ConnectionString", $dsn );
		} else {
			$this->STORE( "ConnectionString", "DSN=$dsn" );
			push(@cdsn, $dsn);
		}
	}

	$cdsn = join( ";", @cdsn );
	$drh->trace_msg("->> Open ADO connection using $cdsn\n", 1);
	$conn->Open ($cdsn, $user, $auth);
	$lastError = DBD::ADO::errors($conn);
	return DBI::set_err( $drh, 1, 
		  "Can't connect to '$dsn': $lastError")
	    if $lastError;

	# Remember, Tom, or-ing them works much better.
	my $att =
		$ado_consts->{adXactCommitRetaining} |
		$ado_consts->{adXactAbortRetaining};

	$conn->{Attributes} = $att;
	$lastError = DBD::ADO::errors($conn);
	return DBI::set_err( $conn, 1, 
			"Failed setting CommitRetaining: $lastError")
		if $lastError;

	# Determine if the provider supports transaction.
	my $auto = 0;
	eval {
		$auto = $conn->Properties->{qq{Transaction DDL}}->{Value};
	if ( $auto eq &DBPROPVAL_TC_ALL ) {
		$this->{ado_provider_support_auto_commit} = $auto;
		$this->{ado_provider_auto_commit_comments} = 
			qq{Transactions can contain DDL and DML statements in any order.};
	} elsif ( $auto eq &DBPROPVAL_TC_DDL_COMMIT ) {
		$this->{ado_provider_support_auto_commit} = $auto;
		$this->{ado_provider_auto_commit_comments} = 
			qq{Transactions can contain DML statements.  DDL statements within a transaction cause the transaction to be committed.};
	} elsif ( $auto eq &DBPROPVAL_TC_DDL_IGNORE )  {
		$this->{ado_provider_support_auto_commit} = $auto;
		$this->{ado_provider_auto_commit_comments} = 
			qq{Transactions can only contain DML statements.  DDL statements within a transaction are ignored.};
	} elsif ( $auto eq &DBPROPVAL_TC_DML )  {
		$this->{ado_provider_support_auto_commit} = $auto;
		$this->{ado_provider_auto_commit_comments} = 
			qq{Transactions can only contain Data Manipulation (DML) statements.  DDL statements within a trnsaction cause an error.};
	} else {
		$this->{ado_provider_support_auto_commit} = $auto;
		$this->{ado_provider_auto_commit_comments} = 
			qq{Transactions are not supported.};
	}
	};
	if ($@) {
		warn "No transactions";
		$this->{ado_provider_support_auto_commit} = 0;
		$this->{ado_provider_auto_commit_comments} = 
			qq{Transactions are not supported.};
		$auto = 0;
		$lastError = DBD::ADO::errors($conn);
	}

	$drh->trace_msg( "->> Transaction support: $auto " .
		$this->{ado_provider_auto_commit_comments} . "\n",1);
	# If transaction are not supported, why execute.
 	if ($auto) {
		$conn->BeginTrans;
		$lastError = DBD::ADO::errors($conn);
		return DBI::set_err( $this, 1, 
			"Begin Transaction Failed: $lastError")
		if $lastError;
	}

	return $this;
	}

    sub disconnect_all { }
    sub DESTROY {
			my $self = shift;
			my $conn = $self->{ado_conn};
			my $auto = $self->{AutoCommit};
			if (defined $conn) {
				$conn->CommitTrans if $auto 
					and $self->{ado_provider_support_auto_commit};
				$conn->RollbackTrans unless $auto 
					and not $self->{ado_provider_support_auto_commit};
			my $lastError = DBD::ADO::errors($conn);
			return DBI::set_err( $self, 1, "Failed to Destory: $lastError") 
				if $lastError;
			}
		}
} # ====== DRIVER ======


# names of adSchemaProviderTypes fields
my $ado_info = [qw{
	TYPE_NAME DATA_TYPE COLUMN_SIZE LITERAL_PREFIX
	LITERAL_SUFFIX CREATE_PARAMS IS_NULLABLE CASE_SENSITIVE
	SEARCHABLE UNSIGNED_ATTRIBUTE FIXED_PREC_SCALE AUTO_UNIQUE_VALUE
	LOCAL_TYPE_NAME MINIMUM_SCALE MAXIMUM_SCALE GUID TYPELIB
	VERSION IS_LONG BEST_MATCH IS_FIXEDLENGTH
}];
# check IS_NULLABLE => NULLABLE (only difference with DBI/ISO field names)
# Information returned from the provider about the schema.  The column names
# are different then the DBI spec.
my $ado_schematables = [
	qw{ TABLE_CAT TABLE_SCHEM TABLE_NAME TABLE_TYPE REMARKS 
		TABLE_GUID TABLE_PROPID DATE_CREATED DATE_MODIFIED
	} ];

my $ado_dbi_schematables = [
	qw{ TABLE_CAT TABLE_SCHEM TABLE_NAME TABLE_TYPE REMARKS } 
	];

my $sch_dbi_to_ado = {
	TABLE_CAT     => 'TABLE_CATALOG',
	TABLE_SCHEM   => 'TABLE_SCHEMA',
	TABLE_NAME    => 'TABLE_NAME',
	TABLE_TYPE    => 'TABLE_TYPE',
	REMARKS       => 'DESCRIPTION',
	TABLE_GUID    => 'TABLE_GUID',
	TABLE_PROPID  => 'TABLE_PROPID',
	DATE_CREATED  => 'DATE_CREATED',
	DATE_MODIFIED => 'DATE_MODIFIED', 
	};
	
my @myType;
my $ado_type;

{   package DBD::ADO::db; # ====== DATABASE ======
    $imp_data_size = 0;

		use Carp;
    use strict;

		# Rollback to the database.
		sub rollback {
			my($dbh) = @_;

			return carp "Rollback ineffective when AutoCommit is on\n"
				if $dbh->{AutoCommit} and $dbh->FETCH('Warn');
			return carp $dbh->{ado_provider_auto_commit_comments} 
				unless $dbh->{ado_provider_support_auto_commit};
			if (exists $dbh->{ado_conn} and defined $dbh->{ado_conn} and
				$dbh->{ado_conn}->{State} & $ado_consts->{adStateOpen}) {
					$dbh->{ado_conn}->RollbackTrans;
					my $lastError = DBD::ADO::errors($dbh->{ado_conn});
					return DBI::set_err( $dbh, 1,
						"Failed to Rollback Trans: $lastError") 
					if $lastError;
			}
		}

		sub disconnect {
			my ($dbh) = @_;
			my $conn = $dbh->{ado_conn};
			local $Win32::OLE::Warn = 0;
			$dbh->trace_msg( "<- State: (" . $conn->State  . ")\n");
			if ($conn->State & $ado_consts->{adStateOpen}) {
				my $auto = $dbh->{AutoCommit};
				# Change the connection attribute so Commit/Rollback
				# does not start another transaction.
				$conn->{Attributes} = 0;
				my $lastError = DBD::ADO::errors($conn);
				return DBI::set_err( $conn, 1, 
						"Failed setting CommitRetaining: $lastError") #-2147168242
				if $lastError and $lastError !~ m/-2147168242/;

				$dbh->trace_msg( "<- modified connection Attributes " . $conn->{Attributes} . "\n");
				$dbh->trace_msg( "<- AutoCommit -> $auto  Provider Support -> $dbh->{ado_provider_support_auto_commit} Comments -> $dbh->{ado_provider_auto_commit_comments}\n");
		$dbh->{ado_provider_auto_commit_comments} = 
				$conn->CommitTrans if $auto and
					$dbh->{ado_provider_support_auto_commit};

				$conn->RollbackTrans unless $auto and
					not $dbh->{ado_provider_support_auto_commit};

				$lastError = DBD::ADO::errors($conn);
				return DBI::set_err( $dbh, 1, "Failed to disconnect: $lastError") 
					if $lastError and $lastError !~ m/-2147168242/;

				$conn->Close;
				$conn = undef;
				$dbh->{ado_conn} = undef;
			}

			$dbh->STORE( ado_conn => undef );
		}

		# Commit to the database.
		sub commit {
			my($dbh) = @_;

			return warn "Commit ineffective when AutoCommit is on\n"
				if $dbh->{AutoCommit} and $dbh->FETCH('Warn');
			return carp $dbh->{ado_provider_auto_commit_comments} 
				unless $dbh->{ado_provider_support_auto_commit};
			if (exists $dbh->{ado_conn} and defined $dbh->{ado_conn} and
				$dbh->{ado_conn}->{State} == $ado_consts->{adStateOpen}) {

				$dbh->{ado_conn}->CommitTrans;
				my $lastError = DBD::ADO::errors($dbh->{ado_conn});
				return DBI::set_err( $dbh, 1, "Failed to CommitTrans: $lastError") 
					if $lastError;
			}
		}

    sub prepare {
			my($dbh, $statement, $attribs) = @_;
			my $conn = $dbh->FETCH("ado_conn");


			$dbh->trace_msg( "-> create a new statement handler\n");

			my $comm = Win32::OLE->new('ADODB.Command');
			my $lastError = Win32::OLE->LastError;
			return DBI::set_err($dbh, 1,
				"Can't create 'object ADODB.Command': $lastError")
	    if $lastError;

			$comm->{ActiveConnection} = $conn;
			$lastError = DBD::ADO::errors($conn);
			return DBI::set_err($dbh, 1,
				"Unable to set ActiveConnection 'ADODB.Command': $lastError")
	    if $lastError;

			$comm->{CommandText} = $statement;
			$lastError = DBD::ADO::errors($conn);
			return DBI::set_err($dbh, 1,
				"Unable to set CommandText 'ADODB.Command': $lastError")
	    if $lastError;

			my $ct = $attribs->{CommandType}? $attribs->{CommandType}: "adCmdText";
			$comm->{CommandType} = $ado_consts->{$ct};
			$lastError = DBD::ADO::errors($conn);
			return DBI::set_err($dbh, 1,
				"Unable to set command type 'ADODB.Command': $lastError")
	    if $lastError;



			my ($outer, $sth) = DBI::_new_sth($dbh, {
	    		'Statement'   => $statement, 
					LongReadLen		=> 0,
					LongTruncOk		=> 0,
					CursorName		=> undef,
					RowsInCache		=> 0,
					ado_comm			=> undef,
					ado_conn 			=> undef,
					ado_dbh				=> undef,
					ado_params		=> [],
					ado_rowset		=> undef,
					ado_refresh		=> 1,
					ado_current_row_count => 0,
			});

#			Determine if Refresh is supported.  If the call returns
#			an error, then Parameters->Refresh is not supported.
			$comm->Parameters->Refresh() if $sth->FETCH(q{ado_refresh});
			$lastError = DBD::ADO::errors($conn);
			if (!$sth->FETCH(q{ado_refresh}) or $lastError) {
				$sth->STORE( 'NUM_OF_PARAMS', _params($statement));
				$sth->STORE( 'ado_refresh' => 0 );
				my $params = $sth->FETCH( 'NUM_OF_PARAMS');
				if ($params > 0) {
					for ( 0 .. ($params - 1)) {
						my $parm = 
							$comm->CreateParameter("$_",
								$ado_consts->{adVarChar},
								$ado_consts->{adParamInput},
								1,
								"");
						my $lastError = DBD::ADO::errors($conn);
						return DBI::set_err( $sth, 1, 
		  				"Unable to CreateParameter: $lastError")
	    			if $lastError;

						$comm->Parameters->Append($parm);
						$lastError = DBD::ADO::errors($conn);
						return DBI::set_err( $sth, 1, 
		  				"Append parameter failed : $lastError")
	    			if $lastError;
					}
				}
			} else {
				$sth->STORE( 'NUM_OF_PARAMS' => $comm->Parameters->Count );
				$sth->STORE( 'ado_refresh' => 1 );
#				Describe the Parameters.
				if ($comm->Parameters->Count) {
				my $cnt = 0;
				while ($cnt < $comm->Parameters->Count) {
					my $x = $comm->Parameters->Item($cnt);
					$dbh->trace_msg( "-> prepare: Name: " .
						$x->{Name} .
						" Type: " .
						$x->{Type} .
						" Direction: " .
						$x->{Direction} .
						" Size: " .
						$x->{Size} .
						"\n");
						$cnt++;
				}
				} else {
						$sth->trace_msg("-> prepare: statement contains no parameters\n");
				}
#				Only preparing a statement if it contains parameters.
				$comm->{Prepared} = 1;
				$lastError = DBD::ADO::errors($conn);
				return DBI::set_err($dbh, 1,
					"Unable to set prepared 'ADODB.Command': $lastError")
	    	if $lastError;
			}

			$sth->STORE( 'CursorName' => undef );
			$sth->STORE( 'RowsInCache' => 0 );

			$sth->STORE( 'ado_params', [] );
			$sth->STORE( ado_conn => $conn );
			$sth->STORE( ado_comm => $comm );
			$sth->STORE( ado_dbh  => $dbh );

		return $outer;
    }
		#
		# Creates a Statement handle from a row set.
		#
    sub _rs_sth_prepare {
			my($dbh, $rs, $attribs) = @_;

			my $conn = $dbh->FETCH("ado_conn");
			my $rows;

			my ($outer, $sth) = DBI::_new_sth($dbh, {
	    		'Statement'   => $attribs,
					LongReadLen		=> 0,
					LongTruncOk		=> 0,
					CursorName		=> undef,
					RowsInCache		=> 0,
					ado_comm			=> undef,
					ado_conn 			=> undef,
					ado_rowset		=> $rs,
					ado_fields		=> undef,
					ado_dbh				=> undef,
					ado_params		=> [],
			});

			my $ado_fields = [ Win32::OLE::in($rs->Fields) ];
			$sth->STORE(ado_fields => $ado_fields);
			my $NUM_OF_FIELDS = @$ado_fields;

			$sth->STORE(Active => 1);
			$sth->STORE(NUM_OF_FIELDS => $NUM_OF_FIELDS);
			$sth->STORE( 'ado_params' => [] );
			$sth->STORE(NAME => [ map { $_->Name } @$ado_fields ]);
			$sth->STORE(ado_dbh => $dbh);
			$sth->STORE(ado_conn => $conn);
			$sth->STORE(ado_comm => $conn);

		return $outer;
    }


# Determine the number of parameters, if Refresh fails.
sub _params
{ 
	my $sql = shift;
	use Text::ParseWords;
	$^W = 0;
	$sql =~ s/\n/ /;
		my $rtn = join( " ", grep { m/\?/ } 
			grep { ! m/^['"].*\?/ } &quotewords('\s+', 1, $sql));
	return ($rtn =~ tr /?//);
}

	# Get information from the current provider.
	sub GetTypeInfo {
		my($dbh, $attribs) = @_;
		my $sth;
		my $lastError;

	# If the type information is previously obtained, use it.
	unless (defined $ado_type) {
		$ado_type = &_determine_type_support;
	}

	my $sponge = DBI->connect("dbi:Sponge:","","",{ RaiseError => 1 });
	$sth = $sponge->prepare("adSchemaProviderTypes", {
		rows=>   [ @$ado_type ] , NAME=> $ado_info,
	});
	$sth;
    }

use Win32::OLE::Variant;

	sub table_info {
		my($dbh, $attribs) = @_;
		my @tp;
		
		my @criteria = (undef); # ADO needs at least one element in the criteria array!
		for (my $i=0; $i<@$ado_dbi_schematables; $i++) {
			my $field = $ado_dbi_schematables->[$i];
			if (exists $attribs->{$field}) {
				$criteria[$i] = $attribs->{$field};
			}
		}
		
		my $field_names = $attribs->{ADO_Columns} ?
			$ado_schematables : $ado_dbi_schematables;

		my $oRec = $dbh->{ado_conn}->OpenSchema($ado_consts->{adSchemaTables}, \@criteria);
		if (exists $attribs->{Filter}) {
			$oRec->{Filter} = $attribs->{Filter};
		}
		
		while(! $oRec->{EOF}) {
			my @out = map { $oRec->Fields($_)->{Value} } 
				map { $sch_dbi_to_ado->{$_} } @$field_names;
			# Jan Dubois jand@activestate.com addition to handle changes
			# in Win32::OLE return of Variant types of data.
			foreach ( @out ) {
				$_ = $_->As(VT_BSTR) 
					if (defined $_) && (UNIVERSAL::isa($_, 'Win32::OLE::Variant'));
			}
			if ($attribs->{Trim_Catalog}) {
				$out[0] =~ s/^(.*\\)//;  # removes leading 
				$out[0] =~ s/(\..*)$//;  # removes file extension 
			}
			push( @tp, \@out );
			$oRec->MoveNext;
		}
		$oRec->Close;
		$oRec = undef;

		my $statement = "adSchemaTables";
		my $sponge = DBI->connect("dbi:Sponge:","","",{ RaiseError => 1 });
		my $sth = $sponge->prepare($statement,
			{ rows=> \@tp, NAME=> $field_names });
		$sth;
	}

  	sub type_info_all {
		my ($dbh) = @_;
		my $names = {
          TYPE_NAME		=> 0,
          DATA_TYPE		=> 1,
          COLUMN_SIZE		=> 2,
          LITERAL_PREFIX	=> 3,
          LITERAL_SUFFIX	=> 4,
          CREATE_PARAMS		=> 5,
          NULLABLE		=> 6,
          CASE_SENSITIVE	=> 7,
          SEARCHABLE		=> 8,
          UNSIGNED_ATTRIBUTE	=> 9,
          FIXED_PREC_SCALE	=>10,
          AUTO_UNIQUE_VALUE	=>11,
          LOCAL_TYPE_NAME	=>12,
          MINIMUM_SCALE		=>13,
          MAXIMUM_SCALE		=>14,
        };
		# Based on the values from the current provider.
		my @myti;
		my $sth = $dbh->func( DBI::SQL_ALL_TYPES(), 'GetTypeInfo' );
		while(my $row = $sth->fetchrow_hashref) {
			my @tyinfo;
			# Only add items from the above names list.  When
			# this list explans, the code 'should' still work.
			for my $n (keys %{$names}){
				$tyinfo[ $names->{$n} ] = $row->{$n} || '';
			}
			push( @myti, \@tyinfo );
		}
		$sth->finish;
		my $ti = [ $names, @myti ];
		return $ti;
	}

	# This is a function, not a method.
	sub _determine_type_support {
		my ($dbh) = @_;
		die qq{dbh undefined} unless $dbh;
		my @prov_type_return;
		undef(@prov_type_return);
		my $conn = $dbh->{ado_conn};

			$dbh->trace_msg( "-> _determine_type_support\n" );
			# This my attempt to convert data types from ODBC to ADO.
	    my %local_types = (
		DBI::SQL_BINARY()   => [$ado_consts->{adBinary}, $ado_consts->{adVarBinary} ],
		DBI::SQL_BIT()      => [$ado_consts->{adBoolean}],
		DBI::SQL_CHAR()     => [$ado_consts->{adChar}, $ado_consts->{adVarChar}, $ado_consts->{adWChar}, $ado_consts->{adVarWChar}],
		DBI::SQL_DATE()     => [$ado_consts->{adDBTimeStamp}, $ado_consts->{adDate}],
		DBI::SQL_DECIMAL()  => [$ado_consts->{adNumeric}],
		DBI::SQL_DOUBLE()   => [$ado_consts->{adDouble}],
		DBI::SQL_FLOAT()    => [$ado_consts->{adSingle}],
		DBI::SQL_INTEGER()  => [$ado_consts->{adInteger}],
		DBI::SQL_LONGVARBINARY() => [$ado_consts->{adLongVarBinary}, $ado_consts->{adVarBinary}, $ado_consts->{adBinary}],
		DBI::SQL_LONGVARCHAR() => [$ado_consts->{adLongVarChar}, $ado_consts->{adVarChar}, $ado_consts->{adChar}, $ado_consts->{adLongVarWChar}, $ado_consts->{adVarWChar}, $ado_consts->{adWChar}],
		DBI::SQL_NUMERIC()  => [$ado_consts->{adNumeric}],
		DBI::SQL_REAL()     => [$ado_consts->{adSingle}],
		DBI::SQL_SMALLINT() => [$ado_consts->{adSmallInt}],
		DBI::SQL_TIMESTAMP() => [$ado_consts->{adDBTime}, $ado_consts->{adDBTimeStamp}, $ado_consts->{adDate}],
		DBI::SQL_TINYINT()  => [$ado_consts->{adUnsignedTinyInt}],
		DBI::SQL_VARBINARY() => [$ado_consts->{adVarBinary}, $ado_consts->{adLongVarBinary}, $ado_consts->{adBinary}],
		DBI::SQL_VARCHAR()  => [$ado_consts->{adVarChar}, $ado_consts->{adChar}, $ado_consts->{adVarWChar}, $ado_consts->{adWChar}],
		DBI::SQL_WCHAR()  => [$ado_consts->{adWChar}, $ado_consts->{adVarWChar}, $ado_consts->{adLongVarWChar}],
		DBI::SQL_WVARCHAR()  => [$ado_consts->{adVarWChar}, $ado_consts->{adLongVarWChar}, $ado_consts->{adWChar}],
		DBI::SQL_WLONGVARCHAR()  => [$ado_consts->{adLongVarWChar}, $ado_consts->{adVarWChar}, $ado_consts->{adWChar}],
	    );

	    my @sql_type = (
				DBI::SQL_BINARY(), 
				DBI::SQL_BIT(),
				DBI::SQL_CHAR(),
				DBI::SQL_DATE(), 
				DBI::SQL_DECIMAL(), 
				DBI::SQL_DOUBLE(),
				DBI::SQL_FLOAT(), 
				DBI::SQL_INTEGER(), 
				DBI::SQL_LONGVARBINARY(),
				DBI::SQL_LONGVARCHAR(), 
				DBI::SQL_NUMERIC(), 
				DBI::SQL_REAL(),
				DBI::SQL_SMALLINT(), 
				DBI::SQL_TIMESTAMP(),
				DBI::SQL_TINYINT(),
				DBI::SQL_VARBINARY(),
				DBI::SQL_VARCHAR(),
				DBI::SQL_WCHAR(),
				DBI::SQL_WVARCHAR(),
				DBI::SQL_WLONGVARCHAR(),
	    );

			# Get the Provider Types attributes.
			my @sort_rows;
			my %ct;
			my $oLRec = 
				$conn->OpenSchema($ado_consts->{adSchemaProviderTypes});
				my $lastError = DBD::ADO::errors($conn);
				die $lastError if $lastError;
				while(! $oLRec->{EOF}) {
					# Sort by row
					my $type_name = $oLRec->{TYPE_NAME}->{Value};
					my $def;
					push ( @sort_rows,  $def = join( " ",
						$oLRec->{DATA_TYPE}->Value, 
						$oLRec->{BEST_MATCH}->Value || 0,
						$oLRec->{IS_LONG}->Value || 0,
						$oLRec->{IS_FIXEDLENGTH}->Value || 0,
						$oLRec->{COLUMN_SIZE}->Value,
						$oLRec->{TYPE_NAME}->Value, ));

						$dbh->trace_msg( "-> data type $def\n");
					my @out = map { $oLRec->{$_}->Value || '' } @$ado_info;
					$ct{$type_name} = \@out;
					$oLRec->MoveNext unless $oLRec->{EOF};
				}
				$oLRec->Close () if $oLRec and
					$oLRec->State & $ado_consts->{adStateOpen};
				$oLRec = undef;
				my ($g_ref);
			for my $x (@sql_type) {
				# Attempt to work with Long text fields.
				# However for a Long field, the order by
				# isn't always the best pick.  Loop through
				# the rows looking for something with a Is Long
				# mark.
				my $loc_t =join( "|", @{$local_types{$x}});
				if ($x == DBI::SQL_LONGVARCHAR()) {
					$g_ref = qr{^($loc_t)\s\d\s1\s0\s};
				} elsif ( $x == DBI::SQL_LONGVARBINARY() ) {
						$g_ref = qr{^($loc_t)\s\d\s1\s0\s};
				} elsif ( $x == DBI::SQL_VARBINARY()) {
						$g_ref = qr{^($loc_t)\s1\s\d\s0\s};
				} elsif ( $x == DBI::SQL_VARCHAR()) {
						$g_ref = qr{^($loc_t)\s[01]\s0\s0\s};
				} elsif ( $x == DBI::SQL_WVARCHAR()) {
						$g_ref = qr{^($loc_t)\s[01]\s0\s0\s};
				} elsif ( $x == DBI::SQL_WLONGVARCHAR()) {
						$g_ref = qr{^($loc_t)\s\d\s1\s0\s};
				} elsif ( $x == DBI::SQL_CHAR()) {
						$g_ref = qr{^($loc_t)\s\d\s0\s1\s};
				} elsif ( $x == DBI::SQL_WCHAR()) {
						$g_ref = qr{^($loc_t)\s\d\s0\s1\s};
				} else {
						$g_ref = qr{^($loc_t)\s\d\s\d\s};
				}

				my @tm = 
					sort { $b cmp $a }
					grep { /$g_ref/ } @sort_rows;
					next unless @tm;
					foreach (@tm) {
						my ($cc) = m/\d+\s+(\D\w.*)$/;
						# Look for the record.
						carp "$cc does not exist in hash\n" unless exists $ct{$cc};
						my @rec = @{$ct{$cc}};
						my @mrec = @rec;
						push( @myType, $x, \@mrec);
						$dbh->trace_msg( "Changing type " . join( " ", @{$ct{$cc}}) . "$cc $rec[1] -> $x\n");
						$rec[1] = $x;
						push(@prov_type_return, \@rec);
					}
				}

			return \@prov_type_return;
		}

	sub OpenSchema {
		my ($dbh) = @_;
		return &_open_schema;
	}

	sub _open_schema {
		my ($dbh, $var) = @_;

		croak qq{_open_schema called with dbh defined} unless $dbh;
		return undef unless $ado_consts->{$var};

		my $conn = $dbh->{ado_conn};
		my $oLRec = 
			$conn->OpenSchema($ado_consts->{$var});
		my $lastError = DBD::ADO::errors($conn);
		die $lastError if $lastError;
		return _rs_sth_prepare( $dbh, $oLRec );
	}



    sub FETCH {
        my ($dbh, $attrib) = @_;
				# If the attribute contains ado_, return the value.
				$dbh->trace_msg( "->Fetch: $attrib\n", 3);
				my $value;
				if ( exists $dbh->{$attrib} ) {
					return $dbh->{$attrib};
				} else {
					eval {
						$attrib =~ s/^ado_//;
						local $Win32::OLE::Warn = 0;
						$value = $dbh->{ado_conn}->{$attrib};
						my $lastError = DBD::ADO::errors($dbh->{ado_conn});
						$lastError = undef if $lastError =~ m/0x80020007/;
						die "Died on:\n$lastError" if $lastError;
					};
				}
				return $value unless $@;
        # else pass up to DBI to handle
        return $dbh->DBD::_::db::FETCH($attrib);
    }

    sub STORE {
        my ($dbh, $attrib, $value) = @_;
				$dbh->trace_msg( "-> Store: $attrib $value\n", 3);
				# Handle a request to change the AutoCommit value.
				# If the service provider supports Transaction,
				# then allow AutoCommit off.
				if ($attrib eq 'Warn' ) {
					$Win32::OLE::Warn = $value;
				}
        if ($attrib eq 'AutoCommit') {
					# Return the value is auto commit is not support and
					# value is not zero.  Handles defaults.
					return $value if $value
						and not $dbh->{ado_provider_support_auto_commit};
					# Cause the application to die, user attempting to
					# change the auto commit value.
					Carp::croak 
						qq{Provider does not support auto commit: },
						$dbh->{ado_provider_auto_commit_comments},
						qq{\n}
					unless $dbh->{ado_provider_support_auto_commit};
					return $dbh->{AutoCommit} = _auto_commit($dbh, $value);
        }
				# If the attribute contains ado_, return the value.
				# Determine if this is one our expected parameters.
				# If the attribute is all lower case, then it is a driver
				# defined value.  If mixed case, then it is a ADO defined value.
				if ($attrib =~ m/^ado_/ || exists $dbh->{$attrib}) {
					return $dbh->{$attrib} = $value;
				} else {
					unless( $attrib =~ /PrintError|RaiseError/) {
					eval {
						local $Win32::OLE::Warn = 0;
						$dbh->{ado_conn}->{$attrib} = $value;
						my $lastError = DBD::ADO::errors($dbh->{ado_conn});
						die $lastError if $lastError;
					};
					Carp::carp $@ if $@ and $dbh->FETCH('Warn');
					return $value unless $@;
					}
				}
        return $dbh->DBD::_::db::STORE($attrib, $value);
    }

		# Rules for auto commit, if here, the provider supports.
		# If auto commit is off and new value is on, commit the
		# current transaction and start a new.
		# If auto commit is on and new value is off, no immediate effect
		# is needed.
		sub _auto_commit {
			my ($dbh, $value) = @_;

			my $cv = $dbh->FETCH('AutoCommit') || 0;
			if ($cv eq 0 and $value eq 1) { # Current off, turn on
				$dbh->commit;
				return 1;
			} elsif ($cv eq 1 and $value eq 0) {
				return 0;
			}
			# Didn't change the value.
			return $cv;
		}
    sub DESTROY { }

} # ======= Database Handle ========


{   package DBD::ADO::st; # ====== STATEMENT ======
    $imp_data_size = 0;

		use Win32::OLE::Variant;
		use Win32::OLE::NLS qw(:DATE);
    use strict;
		use vars qw($VT_VAR $VT_DAT $VT_STR $VT_BIN);

		$VT_VAR = VT_VARIANT() | VT_BYREF();
		$VT_DAT = VT_DATE();
		$VT_STR = VT_BSTR() | VT_BYREF();
		$VT_BIN = VT_UI1()  | VT_ARRAY();


		#$Comm->CreateParameter( "PubID", adVarChar, adParamInput, 4, "0736");
		# Read a chuck of data from a "long" field.
		sub blob_read {
			my ($sth, $cnum, $offset, $lng, $attr) = @_;
				my $fld = @{$sth->{ado_fields}}[$cnum];
				my $str = "";
				if ($fld->Attributes & $ado_consts->{adFldLong}) {
					$str = $fld->GetChunk( $lng );
				} else {
					my $s = $fld->Value;
					$str = substr($s, $offset, $lng); 
				}
			return( (defined($str) and length($str))? $str: "" );
		}

		sub bind_param {
			my ($sth, $pNum, $val, $attr) = @_;
			my $conn = $sth->FETCH("ado_conn");
			my $comm = $sth->FETCH("ado_comm");

	    my $param_cnt = $sth->FETCH( 'NUM_OF_PARAMS' );
			return DBI::set_err($sth, 1,
				"Bind Parameters called with no parameters defined!")
	    unless $param_cnt;

			return DBI::set_err($sth, 1,
				"Bind Parameter $pNum outside current range of $param_cnt.")
	    if ($pNum > $param_cnt or $pNum < 1);

			# Get the data type
			my $type = (ref $attr) ? $attr->{TYPE}: $attr;

			# Convert from ODBC to ADO type
			my $aType = &_convert_type($type);
			my $pd;

			my $params = $sth->{ado_params};
			$params->[$pNum-1] = $val;
			my $p = $comm->Parameters;
#			Determine if the Parameter is defined.
			my $i = $p->Item( $pNum -1 );
			if ($i->{Type} == $ado_consts->{adVarBinary} or
					$i->{Type} == $ado_consts->{adLongVarBinary}
			) {
#					Deal with an image request.
				my $sz = length $val;
				#my $pic2 = Variant(VT_UI1|VT_ARRAY,$i->{Size});
				my $pic = Variant(VT_UI1|VT_ARRAY,$sz + 10);
				$pic->Put($val);
				$i->{Value} = $pic;
				$sth->trace_msg( "->(VarBinary) : ". $i->Size. " ". $i->Type. "\n");
			} else {
				$i->{Size} = $val? length $val: $aType->[2];
				$i->{Value} = $val if $val;
				$sth->trace_msg( "->(default) : ". $i->Size. " ". $i->Type. "\n");
			}
		return 1;
		}

		sub _convert_type {
			my $t = shift;
			for (my $x = 0; $x <= $#myType; $x += 2) {
				return $myType[$x+1] 
					if ($myType[$x] == $t);
			}
			return $ado_consts->{adUnknown};
		}

		sub execute {
			my ($sth, @bind_values) = @_;
			my $comm = $sth->FETCH("ado_comm");
			my $conn = $sth->FETCH("ado_conn");
			my $sql  = $sth->FETCH("Statement");

			# If a record set is currently defined,
			# release the set.
			my $ors = $sth->FETCH("ado_rowset");
			if (defined $ors) {
				$ors->Close () if $ors and
					$ors->State & $ado_consts->{adStateOpen};
				$sth->STORE(ado_rowset => undef);
				$ors = undef;
			}

			my $rows = Variant->new($VT_I4_BYREF, 0);
			#
			# If the application is excepting arguments, then
			# process them here.
			#

			my $lastError;

			my $rs;
			my $p = $comm->Parameters;
			$lastError = DBD::ADO::errors($conn);
			return DBI::set_err($sth, 1,
				"Execute Parameters failed 'ADODB.Command': $lastError")
	    if $lastError;

			return DBI::set_err( $sth, 1, 
		  		"Bind params passed without place holders")
	    if (@bind_values and $p->{Count} == 0);

			my $x = 0;
			for (@bind_values) {
				my $i = $p->Item($x);
				if ($i->{Type} == $ado_consts->{adVarBinary} and
					$i->{Type} == $ado_consts->{adLongVarBinary}
				) {
#					Deal with an image request.
				my $sz = length $_;
				#my $pic = Variant(VT_UI1|VT_ARRAY,$i->{Size});
				my $pic = Variant(VT_UI1|VT_ARRAY,$sz + 10);
				$pic->Put($_);
				$i->{Value} = $pic;
			} else {
				$i->{Size} = length $_;
				$i->{Value} = $_;
			}
			$sth->trace_msg("-> Bind parameter (execute): " . $i->Type . "\n");
				$x++;
			}

			$x = 0;
			$sth->trace_msg( "-> Parameter count: " . $p->{Count} . "\n");
			while( $x < $p->{Count} ) {
				my $params = $sth->{ado_params};
				$sth->trace_msg( "->> Parameter $x: " . $p->Item($x)->{Value} . "\n");
				$sth->trace_msg( "->> Parameter $x: " . $params->[$x] . "\n");
				$x++;
			}
			$rs = $comm->Execute($rows);

			$lastError = DBD::ADO::errors($conn);
			return DBI::set_err( $sth, 1, 
		  		"Can't execute statement '$sql': $lastError")
	    if $lastError;
			
			$sth->{ado_rowset} = $rs;
			$sth->{ado_fields} = my $ado_fields = [ Win32::OLE::in($rs->Fields) ];
			my $num_of_fields = @$ado_fields;

	if ($num_of_fields == 0) {	# assume non-select statement
			# If the AutoCommit is on, Commit current transaction.
			$conn->CommitTrans 
				if $sth->{ado_dbh}->{AutoCommit} 
					and $sth->{ado_dbh}->{ado_provider_support_auto_commit};
			$lastError = DBD::ADO::errors($conn);
			return DBI::set_err( $sth, 1, 
		  		"Execute: Commit failed: $lastError")
	    if $lastError;
			my $c = ($rows->Value == 0 ? qq{0E0} : $rows->Value);
			$sth->STORE('rows', $c);
	    return ( $c );
	}

		my $nof = $sth->FETCH('NUM_OF_FIELDS');
  	$sth->STORE(Active => 1);
		$sth->STORE('NUM_OF_FIELDS' => $num_of_fields)
			unless ($nof == $num_of_fields);
		$sth->{NAME} = [ map { $_->Name } @$ado_fields ];
		$sth->{TYPE} = [ map { $_->Type } @$ado_fields ];
		$sth->{PRECISION} = [ map { $_->Precision } @$ado_fields ];
		$sth->{SCALE} = [ map { $_->NumericScale } @$ado_fields ];
		$sth->{NULLABLE} = [ map { 1 } @$ado_fields ];
		$sth->{CursorName} = undef;
		#$sth->{Statement} = $sql;
		$sth->{Statement} = $rs->Source;
		$sth->{RowsInCache} = undef;
		$sth->STORE( 'rows', $rows->Value );

		# We need to return a true value for a successful select
		# -1 means total row count unavailable
		return $rows->Value;
    }



    sub fetchrow_arrayref {
			my ($sth) = @_;
			my $rs = $sth->FETCH('ado_rowset');

			return undef unless $sth->FETCH('Active');
			return undef unless $rs;
			return undef if $rs->EOF;

			# required to not move from the current row
			# until the next fetch is called.  blob_read
			# reads the next record without this check.
			if ($sth->{ado_current_row_count} > 0) {
				$rs->MoveNext;	# to check for errors and record for next itteration
			}
			return undef if $rs->{EOF};

			my $lastError = DBD::ADO::errors($sth->{ado_conn});
			return DBI::set_err( $sth, 1, 
		  	"Fetch failed: $lastError")
	    if $lastError;


			my $ado_fields = $sth->{ado_fields};

			my $row = 
				[ map { $rs->Fields($_->{Name})->{Value} } @$ado_fields ];
			# Jan Dubois jand@activestate.com addition to handle changes
			# in Win32::OLE return of Variant types of data.
  		foreach (@$row) {
      	$_ = $_->As(VT_BSTR) 
					if UNIVERSAL::isa($_, 'Win32::OLE::Variant');
  		}
			if ($sth->FETCH('ChopBlanks')) {
				map { $_ =~ s/\s+$//; } @$row;
			}


		$sth->{ado_current_row_count}++;
		return $sth->_set_fbav($row);
    }
    *fetch = \&fetchrow_arrayref;


    sub finish {
      my ($sth) = @_;
      my $rs = $sth->FETCH('ado_rowset');
			$rs->Close () if $rs and
				$rs->State & $ado_consts->{adStateOpen};
			$sth->STORE(ado_rowset => undef);
			return $sth->STORE(Active => 0);
    }

    sub FETCH {
        my ($sth, $attrib) = @_;
        # would normally validate and only fetch known attributes
        # else pass up to DBI to handle
				if ( exists $sth->{$attrib} ) {
					return $sth->{$attrib};
				}
        return $sth->DBD::_::dr::FETCH($attrib);
    }

    sub STORE {
        my ($sth, $attrib, $value) = @_;
        # would normally validate and only store known attributes
				if ( exists $sth->{$attrib} ) {
					return $sth->{$attrib} = $value;
				}
        # else pass up to DBI to handle
        return $sth->DBD::_::dr::STORE($attrib, $value);
    }

    sub ColAttributes {         # maps to SQLColAttributes
			my ($sth, $colno, $desctype) = @_;
    }


    sub DESTROY { 
        my ($sth) = @_;
        my $rs = $sth->{ado_rowset};
				$sth->trace_msg( "<- destroy statement handler\n", 1 );
				$rs->Close () 
					if ($rs 
						and UNIVERSAL::isa($rs, 'Win32::OLE')
						and ($rs->State != $ado_consts->{adStateClosed}));
				$rs = undef;
				$sth->{ado_rowset} = undef;
				$sth->STORE(ado_rowset => undef);
        $sth->STORE(Active => 0);
				undef($sth);
		}
}

1;
__END__

=head1 NAME

DBD::ADO - A DBI driver for Microsoft ADO (Active Data Objects)

=head1 SYNOPSIS

  use DBI;

  $dbh = DBI->connect("dbi:ADO:dsn", $user, $passwd);

	Options in the connect string:
	dbi:ADO:dsn;CommandTimeout=60 (your number)
	dbi:ADO:dsn;ConnectTimeout=60 (your number)
	or include both ConnectTimeout and CommandTimeout.

	The dsn may be a standard ODBC dsn or a dsn-less. 
	See the ADO documentation for more information on
	the dsn-less connection.

  # See the DBI module documentation for full details

=head1 DESCRIPTION

The DBD::ADO module supports ADO access on a Win32 machine.  
DBD::ADO is written to support the standard DBI interface to
data sources.

=head1 Connection

  $dbh = DBI->connect("dbi:ADO:dsn", $user, $passwd, $attribs);

	Connection supports dsn and dsn-less calls.

	$dbh = DBI->connect( "dbi:ADO:File Name=oracle.udl", 
		$user, $passwd, {RaiseError => [0|1], PrintError => [0|1],
		AutoCommit => [0|1]});

	In addition the following attributes may be set in the connect string:
		Attributes
		CommandTimeout
		ConnectionString
		ConnectionTimeout
		CursorLocation
		DefaultDatabase
		IsolationLevel
		Mode
		Provider

	WARNING: The application is responsible for passing the correct
	information when setting any of these attributes.


=head1 Functions support

	Using the standard DBI function call
		$dbh->func( arguments, 'function name')
	
	You may access the following functions: (case sensitave)
		OpenSchema

	All functions return a valid statement handle upon success.

		OpenSchema supports the following arguments:
			Any valid ADO Schema name such as
			adSchemaCatalogs
			adSchemaIndexes
			adSchemaProviderTypes

			example:
			my $sth = $dbh->func( 'adSchemaProviderTypes', 'OpenSchema' );

=head1 Enhanced DBI Methods


=head2 table_info

B<Warning:> This method is experimental and may change or disappear.

	$sth = $dbh->table_info(\%attr);

	$sth = $dbh->table_info({ 
		TABLE_TYPE => 'VIEW', 
		ADO_Columns => 1, 
		Trim_Catalog => 0, 
		Filter => q{TABLE_NAME LIKE 'C%'},
	});

Returns an active statement handle that can be used to fetch
information about tables and views that exist in the database.  
By default the handle contains the columns described in the DBI documentation:

	TABLE_CAT, TABLE_SCHEM, TABLE_NAME, TABLE_TYPE, REMARKS  

=item B<ADO_Columns>

Additional ADO-only fields will be included if the ADO_Columns attribute 
is set to true:

	%attr = (ADO_Columns => 1);

=item B<Trim_Catalog>

Some ADO providers include path info in the TABLE_CAT column.  
This information will be trimmed if the Trim_Catalog attribute is set to true:

	%attr = (Trim_Catalog => 1);

=item B<Criteria>

The ADO driver allows column criteria to be specified.  In this way the 
record set can be restricted, for example, to only include tables of type 'VIEW':

	%attr = (TABLE_TYPE => 'VIEW')

You can add criteria for any of the following columns:

	TABLE_CAT, TABLE_SCHEM, TABLE_NAME, TABLE_TYPE

=item B<Filter>

The ADO driver also allows the recordset to be filtered on a Criteria string: 
a string made up of one or more individual clauses concatenated with AND or OR operators.  

	%attr = (Filter => q{TABLE_TYPE LIKE 'SYSTEM%'})

The criteria string is made up of clauses in the form FieldName-Operator-Value.  
This is more flexible than using column criteria in that the filter allows a number of operators: 

	<, >, <=, >=, <>, =, or LIKE

The Fieldname must be one of the ADO 'TABLES Rowset' column names:

	TABLE_CATALOG, TABLE_SCHEMA, TABLE_NAME, TABLE_TYPE, DESCRIPTION, 
	TABLE_GUID, TABLE_PROPID, DATE_CREATED, DATE_MODIFIED

Value is the value with which you will compare the field values 
(for example, 'Smith', #8/24/95#, 12.345, or $50.00). 
Use single quotes with strings and pound signs (#) with dates. 
For numbers, you can use decimal points, dollar signs, and scientific notation. 
If Operator is LIKE, Value can use wildcards. 
Only the asterisk (*) and percent sign (%) wild cards are allowed, 
and they must be the last character in the string. Value cannot be null. 

=back

=head2 tables

B<Warning:> This method is experimental and may change or disappear.

	@names = $dbh->tables(\%attr);

Returns a list of table and view names. 
Accepts any of the attributes described in the L<table_info> method:

	@names = $dbh->tables({ TABLE_TYPE => 'VIEW' });

=back

=head1 Warnings

	Support for type_info_all is supported, however, you're not using
	a true OLE DB provider (using the MS OLE DB -> ODBC), the first
	hash may not be the "best" solution for the data type.
	adSchemaProviderTypes does provide for a "best match" column, however
	the MS OLE DB -> ODBC provider does not support the best match.
	Currently the types are sorted by DATA_TYPE BEST_MATCH IS_LONG ...

=head1 ADO

It is strongly recommended that you use the latest version of ADO
(2.1 at the time this was written). You can download it from:

  http://www.microsoft.com/Data/download.htm

=head1 AUTHORS

Phlip and Tim Bunce. With many thanks to Jan Dubois, Jochen Wiedmann
and Thomas Lowery for additions, debuggery and general help.

=head1 SEE ALSO

ADO Reference book:  ADO 2.0 Programmer's Reference, David Sussman and
Alex Homer, Wrox, ISBN 1-861001-83-5. If there's anything better please
let me know.

http://www.able-consulting.com/tech.htm

=cut
