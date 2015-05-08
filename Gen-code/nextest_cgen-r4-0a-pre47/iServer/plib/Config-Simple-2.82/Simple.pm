package Config::Simple;

require 5.003;

use strict;
use Carp 'croak';
use File::Copy;
use Fcntl qw(:DEFAULT :flock);

require Exporter;
use vars qw($VERSION @ISA @EXPORT $MD5 $errstr $LOCK_FILE);

@ISA = qw(Exporter);

$VERSION = "2.82";

eval {
    for ( @Fcntl::EXPORT ) {
        /^(O_|LOCK_)/   or next;
        push @EXPORT, $_;
    }
    Fcntl->import(@EXPORT);
};


$MD5 = 1;

eval {  require Digest::MD5 };
if ( $@ ) {
    $MD5 = 0;    
}








sub new {
    my $class = shift;
    $class = ref($class) || $class;

    my $self = {
        _options    =>  {
            filename    => undef,
            mode        => O_RDONLY|O_CREAT,
            lockfile    => "",
            c           => 1,
            @_,
        },
        _cfg        => {},
        _checksum   => undef,
        _tied_access => undef,
    };

    # deciding a name for a lock file
    unless ( $self->{_options}{lockfile} ) {
        $self->{_options}{lockfile} = $self->{_options}{filename} . ".lock";
    }

    $self->{_options}{filename} ||=$self->{_options}{_filename};


    # creating a digest of the file...
    if ( $MD5 and -e $self->{_options}{filename} ) {
		sysopen (FH, $self->{_options}->{filename}, O_RDONLY) or croak "Could not open $self->{_options}->{filename}";
        flock (FH, LOCK_SH|LOCK_NB) or croak "Couldn't acquire a lock on $self->{_options}{filename}, $!";

        my $md5 = Digest::MD5->new();
        $md5->addfile(\*FH);
        $self->{_checksum} = $md5->hexdigest();
        close(FH);
    }

    bless $self => $class;

    $self->_init() or return undef;

    return $self;
}




sub DESTROY {   }



sub _init {
    my $self = shift;

    my $mode = $self->{_options}->{mode};
    my $file = $self->{_options}{filename};

	sysopen (FH, $file, $mode) or croak "Could not open $file, $!";
    flock (FH, LOCK_SH) or croak "Couldn't get lock on $file, $!";

	my ($title, $after_dot);
    #my $title = "";
    #my $after_dot = 0;
    local $/ = "\n";
    while ( <FH> ) {

        # whitespace support (\s*) added by Michael Caldwell <mjc@mjcnet.com>
        # date: Tuesday, May 14, 2002
        $after_dot  and $self->{'.'} .= $_, next;
        /^\s*(;|\n|\#)/    and next;   # deal with \n and comments
        /^\s*\[([^\]]+)\]\s*$/ and $title = $1, next;
        /^\s*([^=]+?)\s*=\s*(.+?)\s*$/ and $self->{_cfg}->{$title}{$1} = $self->{_named}{"$title.$1"} = $2, next;
        /^\./           and $after_dot = 1, next;   # don't parse anything after the sole dot (.)

        chomp ($_);
        my $msg = "Syntax error in line $. of $file: \"$_\"";
        $self->{_options}{c}    and croak $msg;

    }

    close(FH);
    return 1;
}



sub _tied_access {
    my $self = shift;

    $self->{_tied_access} = shift;
}


sub _get_block {
    my $self = shift;

    my ($block) =  @_;
    my %hash = ();

    for my $key ( keys %{ $self->{_cfg}{$block} } ) {
        $hash{$key} = $self->_get_single_param($block, $key)
    }

    return \%hash;
}


sub _create_block {
    my $self = shift;
    my ($block, $values) = @_;

    for my $key ( keys %{$values} ) {
        $self->_set_single_param($block, $key, $values->{$key});
    }

}



sub _get_single_param {
    my $self = shift;
    my ($block, $key) = @_;

    my $value = $self->{_cfg}->{$block}->{$key} || "";
    $value =~ s/\\n/\n/g;


    # still not sure if it does the trick. Gotta check it out
    if ( wantarray ) {

        my @array = split /,/, $value;
        return map { s/^\s+//, s/\s+$// } @array;

    }


    return $value;

}



sub _set_single_param {
    my $self = shift;

    my ($block, $key, $value) = @_;

    $value =~ s/\n/\\n/g;
    $self->{_cfg}->{$block}->{$key} = $value;
    $self->{_named}->{"$block.$key"} = $value;

}


sub param {
    my $self = shift;

    # @b = $config->param();
    unless ( scalar(@_) ) {
        my %tmp = $self->param_hash();
        return keys %tmp;
    }

    # $config->param('author.f_name');
    if ( scalar(@_) == 1 ) {
        my ($block, $key) = split /\./, $_[0];
        return $self->_get_single_param($block, $key);
    }


    # implementing verion 2.0 API
    my $args = {};
    if ( scalar(@_) == 2 ) {
        # $config->param('author.f_name', 'Shezrod');
        my ($block, $key) = split /\./, $_[0];
        if ( $block && $key ) {
            $self->_set_single_param($block, $key, $_[1]);
        }

        $args = {
            -block  => "",
            -name   => "",
            @_,
        };

        # $config->param(-block=>'author');
        if ( $args->{'-block'} ) {
            return $self->_get_block($args->{'-block'});
        }

        # $config->param(-name=>'author.f_name');
        if ( $args->{'-name'} ) {
            my ($block, $key) = split /\./, $args->{'-name'};
            if ($block && $key) {
                return $self->_get_single_param($block, $key);
            }
        }
    }


    $args = {
        -block  => "",
        -values => {},
        -name   => "",
        -value  => "",
        @_,
    };

    if ( $args->{'-block'} and $args->{'-values'} ) {
        # checking if `-values` is actually a hashref
        unless ( ref( $args->{'-values'} ) eq 'HASH' ) {
            croak "'-values' option requires a hash reference";
        }
        $self->_create_block($args->{'-block'}, $args->{'-values'});
        return 1;
    }


    if ( $args->{'-name'} && $args->{'-value'} ) {
        my ($block, $key) = split /\./, $args->{'-name'};
        if ( $block && $key) {
            $self->_set_single_param($block, $key, $args->{'-value'});
            return 1;
        }
    }
}






# for backward compatability...
sub set_param {
    my $self = shift;

    my ($name, $value) = @_;
    my ($block, $key) = split /\./, $name;
    $self->_set_single_param($block, $key, $value);

}



sub param_hash {
    my $self = shift;

    my %hash = ();
    for my $block ( keys %{$self->{_cfg}} ) {
        $block =~ m/^\./    and next;
        for my $key ( keys %{ $self->{_cfg}{$block} } ) {
            $hash{ $block . '.' . $key } = $self->_get_single_param($block, $key);
        }
    }

    return %hash;
}



sub write {
    my $self = shift;

    my $new_file = $_[0] || undef;
    my $file = $self->{_options}->{filename};
    my $lock = $self->{_options}->{lockfile};
    my $mode = $self->{_options}->{mode};

    # checking if anything was changed manually while
    # program was running...
    if ( $MD5 and !$new_file ) {

		sysopen (CHECKSUM, $self->{_options}->{filename}, O_RDONLY) 
			or croak "Could not open $self->{_options}->{filename}, $!";
        
        flock (CHECKSUM, LOCK_SH) or croak "Couldn't acquire lock on $self->{_options}{filename}, $!";
        my $md5 = Digest::MD5->new();
        $md5->addfile(\*CHECKSUM);
        close(CHECKSUM);

        if ( ($self->{_checksum} ne $md5->hexdigest) and (not $self->{_tied_access} ) ) {            
            File::Copy::copy($file, $file . ".bk");
        }
    }

    sysopen (FH, $new_file || $file, O_RDWR|O_CREAT|O_TRUNC, 0664) or croak "Could not truncate the file, $!";
	flock (FH, LOCK_EX) or croak "Could not  get lock on the configuration file";
	select (FH);

    print "; Maintained by Config::Simple/$VERSION\n";    
    print "; ", "-" x 70, "\n\n";

    while ( my($block, $values) = each %{ $self->{_cfg} } ) {

        print "[$block]\n";
        while ( my ($key, $value) = each %{$values} ) {
            print "$key=$value\n";
        }
        print "\n";
    }

    if ( defined $self->{'.'} )  {
        print ".\n";
        print $self->{'.'};
    }

    select (STDOUT);
	close (FH);

    
    

}








sub error {
    my ($self, $dump) = @_;

    if ( $dump ) {
        require Data::Dumper;
        my $fh = new IO::File ('Config-Simple.core', O_RDWR|O_CREAT|O_TRUNC, 0644);
        $fh->print(Data::Dumper::Dumper($self));
        $fh->close();
    }

    return $errstr;

}







package Config::Simple::Tie;

require Tie::Hash;
use vars qw(@ISA);

@ISA = qw(Config::Simple Tie::Hash);



# TIEHASH() patch submited by Scott.Weinstein@lazard.com
# on Tue, 2 Apr 2002 13:47:34 -0500
sub TIEHASH {
    my $class = shift;
    $class = ref($class) || $class;

    my ($file, $mode, $check) = @_;

    my $self = $class->Config::Simple::new(
        filename=>  $file,
        mode    =>  $mode,
        c       =>  $check
    );

    $self->_tied_access(1) if ($self);

    return $self;
}



sub DESTROY {
    my $self = shift;

    $self->_tied_access(0);
}


sub FETCH {
    my $self = shift;
    my $key = shift;

    return $self->param($key);
}


sub STORE {
    my $self = shift;
    my $key = shift;
    my $value = shift;

    $self->param($key, $value);
    return $self->write();
}


sub FIRSTKEY {
    my $self = shift;

    my $temp = keys %{$self->{_named}};
    return scalar each %{$self->{_named}};
}


sub NEXTKEY {
    my $self = shift;

    return scalar each %{$self->{_named}};
}


sub DELETE {
    my $self = shift;
    my $name = shift;

    my ($block, $key) = split /\./, $name;

    delete $self->{_named}{$name};
    delete $self->{_cfg}->{$block}->{$key};

    $self->write();
}




#sub EXISTS {
#    my $self = shift;
#
#    return exists $self->{_named}{shift};
#}



sub CLEAR {
    my $self = shift;

    for (keys %{$self->{_named}} ) {
        $self->{_named} = {};
        $self->{_cfg} = {};
    }
}



1;

=pod

=head1 NAME

Config::Simple - Simple Configuration File Class

=head1 SYNPOSIS

in the app.cfg configuration file:

    [mysql]
    host=ultracgis.com
    login=sherzodr
    password=secret

    [profile]
    first name=Sherzod
    last name=Ruzmetov
    email=sherzodr@cpan.org


in your Perl application:

    use Config::Simple;
    my $cfg = new Config::Simple(filename=>"app.cfg");

    print "MySQL host: ", $config->param("mysql.host"), "\n";
    print "MySQL login: ", $config->param("mysql.login"), "\n";

    # to get just [mysql] block:
    my $mysql = $cfg->param(-block=>"mysql");

    print "MySQL host: ", $mysql->{host}, "\n";
    print "MySQL login: ", $mysql->{login}, "\n";

    $cfg->write();

    # Tied hash access...

    tie my %Config, "Config::Simple::Tie", "app.cfg", O_RDONLY|O_CREAT or die $Config::Simple::errstr;

    print "MySQL host: ", $Config{'mysql.host'}, "\n";

    # setting new MySQL host value

    $Config{'mysql.host'} = "new.localhost";    # this also updates the file
    delete $Config{'mysql.RaiseError'};         # also updates the file


=head1 WARNING

As of version 2.9 library's C<param()> method slightly changed its behavior
when called without arguments. Prior to this version it used to return list
consisting of the names of blocks in the configuration file. After 2.9 and above
it retruns the list of all the names. Example:

    # in the config file:
    [mysql]
    host = localhost
    user = sherzodr
    password = marley01

    [general]
    site_url = http://www.ultracgis.com
    site_path = /home/sherzodr/public_html


Prior to 2.9 syntac C<$config->param()> used to return list of "mysql" and "general".
Now it returns "mysql.host", "mysql.user", "mysq.password", "general.site_url",
"general.site_path". Sorry about the incompatibility.


=head1 NOTE

This documentation refers to version 2.81 of Config::Simple. If you have a version
older than this, please update it to the latest release.

=head1 DESCRIPTION

This Perl5 library  makes it very easy to parse INI-styled configuration files
and create once on the fly. It optionally requires L<Digest::MD5|Digest::MD5> module.

C<Config::Simple> modules defines too classes, C<Config::Simple> and C<Config::Simple::Tie>.
Latter will allow accessing the Configuration files via tied hash variables. In this case,
modifications to the values of the hash will reflect in the configuration file right away.

Unlike the case with C<Config::Simple::Tie>, C<Config::Simple> uses simple
OO style, and requires calling  L<write()> if you want to update the contents of
the file. Otherwise all the manipulations will take place in memory.

Syntax for Config::Simple::Tie is:

    tie my %Hash, "Config::Simple::Tie", "file.cfg" [,mode] [,c] or die $Config::Simple::errstr;

We'll give you a brief example that makes use of C<Config::Simple::Tie> and we'll continue on
with C<Config::Simple> methods.

    use Config::Simple;     # notice, you still have to load Config::Simple
                            # not Config::Simple::Tie!!!

    tie my %Config, "Config::Simple::Tie", "app.cfg" or die "app.cfg: $Config::Simple::errstr";

    my $host = $Config{'mysql.host'};
    my $login = $Config{'mysql.login'};

    delete $Config{'mysql.password'};   # deletes from the file as well

    $Config{'mysql.login'} = "marley01"; # updates the file

    untie %Config;          # destroys the connection;


As you see, after you tie a hash variable to C<Config::Simple::Tie> class, you can
use it as an ordinary HASH, but in the background it will be dealing with a real
file.


=head2 CONFIGURATION FILE

Configuration file that Config::Simple uses is similar to Window's *.ini file syntax.
Example.,

    ; sample.cfg

    [block1]
    key1=value1
    key2=value2
    ...

    [block2]
    key1=value1
    key2=value2
    ...

It can have infinite number of blocks and infinite number of key/value pairs in each block.
Block and key names are case sensitive. i.e., [block1] and [Block1] are two completely different
blocks. But this might change in the subsequent releases of the library. So please use with caution!

Lines that start with either ';' (semi colon) or '#' (pound) are assumed to be comments
till the end of the line. If a line consists of a sole '.' (dot), then all the lines
till eof are ignored (it's like __END__ Perl token)

When you create Config::Simple object with $cfg = new Config::Simple(filename=>"sample.cfg")
syntax, it reads the above sample.cfg config file, parses the contents, and creates
required data structure, which you can access through its public L<methods|/"METHODS">.

In this documentation when I mention "name", I'll be referring to block name and key delimited with a dot (.).
For example, from the above sample.cfg file, following names could be retrieved:
block1.key1, block1.key2, block2.key1 and block2.key2 etc.

Here is the configuration file that I use in most of my CGI applications, and I'll be using it
in the examples throughout this manual:

    ;app.cfg

    [mysql]
    host=ultracgis
    login=sherzodr
    password=secret
    db_name=test
    RaiseError=1
    PrintError=1

=head2 WHITESPACE

Prior to version 2.8, the following syntax produced a syntax error if C<c=E<gt>1>
swith in the new() was turned on:

    [mysql]
    # this is a comment
    host= ultracgis
    login =sherzodr
    password = secret

because of the whitespace surrounding C<=>. The problem was fixed in version 2.8
of the library (thanks to Michael Caldwell, see L<credits|"CREDITS">), and all the following whitespace combinations
are considered valid:

        [mysql]
        # this is a comment
    host = ultracgis
      login =sherzodr
     password =     secret

This will enable custom identation in your config files.

=head2 fcntl.h CONSTANTS

By default Config::Simple exports C<O_RDONLY>, C<O_RDWR>, C<O_CREAT>, C<O_EXCL> L<fcnl.h|Fcntl> (file control) constants. When you create Config::Simple object by passing it a filename, it will try to read the file.
If it fails it creates the file. This is a default behavior. If you want to control this behavior,
you'll need to pass mode with your desired fcntl O_* constants to the constructor:

    $config = new Config::Simple(filename=>"app.cfg", mode=>O_RDONLY);
    $config = new Config::Simple(filename=>"app.cfg", mode=>O_RDONLY|O_CREAT); # default
    $config = new Config::Simple(filename=>"app.cfg", mode=>O_EXCL);

    # tied hash access method:

    $config = tie %Config, "Config::Simple::Tie", "app.cfg", O_RDONLY;
    $config = tie %Config, "Config::Simple::Tie", "app.cfg", O_RDONLY|O_CREAT;
    $config = tie %Config, "Config::Simple::Tie", "app.cfg", O_EXCL;


Note: in the tied hash access method, you can ignore the return value of tie() (which is a Config::Simple object).
You will not be using it any ways. In case you'll need this at any time in the future, you
can always acquire it by calling tied() Perl built-in function:

    $config = tied(%Config);




fcntl constants:

    +===========+============================================================+
    | constant  |   description                                              |
    +===========+============================================================+
    | O_RDONLY  |  opens a file for reading only, fails if doesn't exist     |
    +-----------+------------------------------------------------------------+
    | O_RDWR    |  opens a file for reading and writing                      |
    +-----------+------------------------------------------------------------+
    | O_CREAT   |  creates a file                                            |
    +-----------+------------------------------------------------------------+
    | O_EXCL    |  creates a file if it doesn't already exist                |
    +-----------+------------------------------------------------------------+

=head1 METHODS

=over 2

=item new( filename=>$scalar [, mode=>O_*] [, lockfile=>$scalar] [,c=>$boolean] )

Constructor method. Requires filename to be present and picks up defaults for the rest
if omitted. mode is used while opening the file, lockfile while updating the file.

It returns Config::Simple object if successful. If it fails, sets the error message to
$Config::Simple::errstr variable, and returns undef.

mode can accept any of the above described L<fcntl|Fcntl> constants. Default is C<O_RDONLY E<verbar> O_CREAT>.
Default lockfile is the name of the configuration file with ".lock" extension

If you set the value of C<c> to 1 (true), then it checks the configuration file for proper
syntax, and throws an exception if it finds a syntax error. Error message looks
something like C<Syntax error in line 2 of sample.cfg: "this is just wrong" at t/default.t line 11>.
If you set it to 0 (false), those lines will just be ignored.

=item param([args])

If called without any arguments, returns the list of all the available blocks in the
config. file.

If called with arguments, this method supports several  different syntax,
and we'll discuss them all separately.

=over 4

=item param($name))

returns the value of $name. $name is block and key separated with a dot. For example,
to retrieve the mysql login name from the app.cfg, we could use the following
syntax:

    $login = $cfg->param("mysql.login");

=item param(-name=>$name)

the same as L<"param($name)">

=item param($name, $value)

updates the value of $name to $value. For example, to set the value of "RaiseError" to 0, and
create an new "AutoCommit" key with a true value:

    $cfg->param("mysql.RaiseError", 0);
    $cfg->param("mysql.AutoCommit", 1);

As I implied above, if either the block or the key does not exist, it will be created for you.
So

    $cfg->param("author.f_name", "Sherzod");

would create an [author] block with "f_name=Sherzod" key/value pair.

=item param(-name=>$name, -value=>$value)

the same as L<"param($name, $value)">

=item param(-block=>$block)

returns the whole block as a hash reference. For example, to get the whole [mysql] block:

    $mysql = $cfg->param(-block=>'mysql');

    $login = $mysql->{login};
    $psswd = $mysql->{password};

=item param(-block=>$block, -values=>{key1=>value1, key2=>value2})

creates a new block with the specified values,
or overrides existing block. For example, to add the [site] block to the above app.cfg with
"title=UltraCgis.com" and "description=The coolest site" key/values we could use the following syntax:

    $cfg->param(-block=>"site", -values=>{
                    title=> "UltraCgis.com",
                    description=>"The coolest site",
                    author=>"Sherzod B. Ruzmetov",
                    });

note that if the [site] block already exists, its contents will be cleared and then re-created with
the new values.

=back

=item set_param($name, $value)

This method is provided for backward compatibility with 1.x version of Config::Simple. It is identical
to param($name, $value) syntax.

=item param_hash()

handy method to save the contents of the config. file into a hash variable.

    %Config = $cfg->param_hash();

Structure of %Config looks like the following:

    %Config = (
        'mysql.PrintError'  => 1,
        'mysql.db_name'     => 'test',
        'mysql.login'       => 'sherzodr',
        'mysql.password'    => 'secret',
        'mysql.host'        => 'ultracgis.com',
        'mysql.RaiseError'  => 1,
    );

=item write([$new_filename])

Notice, that all the above manipulations take place in the object's memory, i.e., changes you
make with param() and set_param() methods do not reflect in the actual config. file.
To update the config file in the end, you'll need to call L<"write()"> method with no arguments.

If you want to save newly updated/created configuration into a new file, pass the new filename
as the first argument to the write() method, and the original config. file will not be
touched.

If it detects that configuration file was updated by a third party while Config::Simple was working
on the file, it throws a harmless warning to STDERR, and will copy the original file to a new location
with the .bk extension, and updates the configuration file with its own contents.

L<"write()"> returns true if successful, undef, otherwise. Error message can be accessed either
vi $Config::Simple::errstr variable, or by calling L<"error()"> method.

=item error()

Returns the value of $Config::Simple::errstr

=back

=head1 BUGS

Please send them to my email if you detect any with a sample code
that triggers that bug. Even if you don't have any, just let me know that you are using it.
It just makes me feel good ;-)

=head1 CREDITS

Following people contributed patches and/or suggestions to the Config::Simple.
In chronological order:

=over 4

=item Michael Caldwell (mjc@mjcnet.com)

Added L<witespace support|"WHITESPACE"> in the configuration files, which
enables custom identation

=item Scott Weinstein (Scott.Weinstein@lazard.com)

Fixed the bugs in the TIEHASH method.

=back

=head1 COPYRIGHT

    Copyright (C) 2002  Sherzod Ruzmetov <sherzodr@cpan.org>

    This program is free software; you can redistribute it and/or
    modify it under the same terms as Perl itself

=head1 AUTHOR

    Sherzod B. Ruzmetov <sherzodr@cpan.org>
    http://www.ultracgis.com

=head1 SEE ALSO

L<Config::General>, L<AppConfig>, L<Config::IniFiles>

=cut
