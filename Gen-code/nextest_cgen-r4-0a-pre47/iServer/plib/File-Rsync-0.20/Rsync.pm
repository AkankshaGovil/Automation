#!/usr/local/bin/perl
#      __
#     /\ \ From the mind of
#    /  \ \
#   / /\ \ \_____ Lee Eakin  <Leakin@dfw.Nostrum.com>
#  /  \ \ \______\       or  <Leakin@japh.net>
# / /\ \ \/____  /       or  <Lee@Eakin.Org>
# \ \ \ \____\/ / Wrapper module for the rsync program
#  \ \ \/____  /  rsync can be found at http://rsync.samba.org/rsync/
#   \ \____\/ /
#    \/______/

package File::Rsync;
require 5.004; # it might work with older versions of 5 but not tested

use FileHandle;
use IPC::Open3 qw(open3);
use POSIX ":sys_wait_h";
use Carp 'carp';
use File::Rsync::Config;

use strict;
use vars qw($VERSION);

$VERSION=do {my @r=(q$Revision: 1.1 $=~/\d+/g);sprintf "%d."."%02d"x$#r,@r};

=head1 NAME

File::Rsync - perl module interface to rsync(1) F<http://rsync.samba.org/rsync/>

=head1 SYNOPSIS

use File::Rsync;

$obj = File::Rsync->new( { archive => 1, compress => 1,
         rsh => '/usr/local/bin/ssh',
         'rsync-path' => '/usr/local/bin/rsync' } );

$obj->exec( { src => 'localdir', dest => 'rhost:remdir' } )
         or warn "rsync failed\n";

=head1 DESCRIPTION

Perl Convenience wrapper for the rsync(1) program.  Written for I<rsync-2.3.2>
and updated for I<rsync-2.4.6> but should perform properly with most recent
versions.

=over 4

=item File::Rsync::new

$obj = I<new> File::Rsync;

   or

$obj = File::Rsync->I<new>;

   or

$obj = File::Rsync->new(@options);

   or

$obj = File::Rsync->new(\%options);

Create a I<File::Rsync> object.  Any options passed at creation are stored in
the object as defaults for all future I<exec> call on that object.  Options
may be passed in the form of a hash and are the same as the long options in
L<rsync> with the leading double-dash removed.  An additional option of
B<path-to-rsync> also exists which can be used to override the hardcoded
path to the rsync binary that is defined when the module is installed,
and B<debug> which causes the module methods to print some debugging
information to STDERR.  The B<outfun> and B<errfun> options take a function
reference.  The function is called once for each line of output from the
I<rsync> program with the output line passed in as the first argument, the
second arg is either 'out' or 'err' depending on the source.  This makes it
possible to use the same function for both and still determine where the output
came from.  Options may also be passed as a reference to a hash.  The
B<exclude> option needs an array reference as its value, since there cannot be
duplicate keys in a hash.  There is an equivalent B<include> option.  Only an
B<exclude> or B<include> option should be used, not both.  Use the '+ ' or '- '
prefix trick to put includes in an B<exclude> array, or to put excludes in an
B<include> array (see L<rsync> for details).  Include/exclude options form an
ordered list.  The order must be retained for proper execution.  There are also
B<source> and B<dest> keys.  The key B<src> is also accepted as an equivalent
to B<source>, and B<dst> or B<destination> may be used as equivalents to
B<dest>.  The B<source> option may take a scalar or an array reference.  If the
source is the local system then multiple B<source> paths are allowed.  In this
case an array reference should be used.  There is also a method for passing
multiple source paths to a remote system.  This method may be triggered in this
module by passing the remote hostname to the B<srchost> key and passing an
array reference to the B<source> key.  If the source host is being accessed via
an Rsync server, the remote hostname should have a single trailing colon on the
name.  When rsync is called, the B<srchost> value and the values in the
B<source> array will be joined with a colon resulting in the double-colon
required for server access.  The B<dest> key only takes a scalar since I<rsync>
only accepts a single destination path.

=back

=cut

sub new {
   my $class=shift;

   # seed the options hash, booleans, scalars, excludes, source, dest, data,
   # status, stderr/stdout storage for last exec
   my $self={
      # the full path name to the rsync binary
      'path-to-rsync' => $RsyncConfig{rsync_path},
      # these are the boolean flags to rsync, all default off, including them
      # in the args list turns them on
      'flag' => {qw(
         archive           0   dry-run           0   perms             0
         backup            0   existing          0   progress          0
         blocking-io       0   force             0   recursive         0
         checksum          0   group             0   relative          0
         compress          0   hard-links        0   safe-links        0
         copy-links        0   help              0   size-only         0
         copy-unsafe-links 0   ignore-errors     0   sparse            0
         cvs-exclude       0   ignore-times      0   stats             0
         daemon            0   links             0   times             0
         delete            0   numeric-ids       0   update            0
         delete-after      0   one-file-system   0   version           0
         delete-excluded   0   owner             0   whole-file        0
         devices           0   partial           0
      )},
      # these have simple scalar args we cannot easily check
      'scalar' => {qw(
         address         0   exclude-from    0   rsh             0
         backup-dir      0   include-from    0   rsync-path      0
         block-size      0   log-format      0   suffix          0
         bwlimit         0   max-delete      0   temp-dir        0
         compare-dest    0   modify-window   0   timeout         0
         config          0   password-file   0
         csum-length     0   port            0
      )},
      # these are not flags but counters, each time they appear it raises the
      # count, so we keep track and pass them the same number of times
      'counter' => {qw(quiet 0    verbose 0)},
      # these can be specified multiple times and are additive, the doc also
      # specifies that it is an ordered list so we must preserve that order
      'exclude'     => [],
      'include'     => [],
      # hostname of source, used if 'source' is an array reference
      'srchost'     => '',
      # source host and/or path names
      'source'      => '',
      # destination host and/or path
      'dest'        => '',
      # return status from last exec
      'status'      => 0,
      'realstatus'  => 0,
      # whether or not to print debug statements
      'debug'       => 0,
      # stderr from last exec in array format (messages from remote rsync proc)
      'err'         => 0,
      'errfun'      => undef,
      # stdout from last exec in array format (messages from local rsync proc)
      'out'         => 0,
      'outfun'      => undef,
      # this flag changes error checking in 'exec' when called by 'list'
      'list'        => 0,
   };
   bless $self, $class; # bless it first so defopts can find out the class
   if (@_) {
      &defopts($self,@_) or return undef;
   }
   return $self;
}

=over 4

=item File::Rsync::defopts

I<defopts> $obj @options;

   or

$obj->defopts(@options);

   or

$obj->defopts(\%options);

Set default options for future exec calls for the object.  See L<rsync>
for a complete list of valid options.  This is really the internal
method that I<new> calls but you can use it too.  The B<verbose> and B<quiet>
options to rsync are actually counters.  When assigning the perl hash-style
options you may specify the counter value directly and the module will pass
the proper number of options to rsync.

=back

=cut

sub defopts {
   # this method has now been split into 2 sub methods (parse and save)
   # _saveopts and _parseopts should only be used via defopts or exec
   my $self=shift;
   &_saveopts($self,&_parseopts($self,@_));
}

sub _parseopts {
   # this method checks and converts it's args into a reference to a hash
   # of valid options and returns it to the caller
   my $self=shift;
   my $pkgname=ref $self;
   my @opts=@_;
   my $opt;
   my %OPT=(); # this is the hash we will return a ref to

   # make sure we are passed the proper number of args
   if (@opts == 1) {
      $opt=shift;
      if (my $reftype=ref $opt) {
         unless ($reftype eq 'HASH') {
            carp "$pkgname: invalid reference type ($reftype) in options";
            return 0;
         }
      } else {
         carp "$pkgname: invalid option ($opt)";
         return 0;
      }
   } elsif (@opts % 2) {
      carp "$pkgname: invalid number of options passed (must be key/value pairs)";
      return 0;
   } else {
      $opt={@opts};
   }

   # now process the options given, we handle debug first since hashes do not
   # have a specific order, and it would not be set first even if we sorted
   if (exists $opt->{'debug'}) {
      $OPT{'debug'}=$opt->{'debug'};
      print(STDERR "setting debug flag\n") if $OPT{'debug'};
   }
   foreach my $hashopt (keys %$opt) {
      my $savopt=$hashopt;
      $savopt=~tr/_/-/;
      next if $hashopt eq 'debug'; # we did this one first (above)
      print STDERR "processing option: $hashopt\n"
         if $OPT{'debug'} or $self->{'debug'};
      if (exists $self->{'flag'}{$savopt} or
            exists $self->{'scalar'}{$savopt} or
            exists $self->{'counter'}{$savopt}) {
         $OPT{$savopt}=$opt->{$hashopt};
      } else {
         my $tag='';
         if ($hashopt eq 'exclude' or $hashopt eq 'include') {
            $tag=$hashopt;
         } elsif ($hashopt eq 'source' or
               $hashopt eq 'src') {
            $tag='source';
         }
         if ($tag) {
            if (my $reftype=ref $opt->{$hashopt}) {
               if ($reftype eq 'ARRAY') {
                  $OPT{$tag}=$opt->{$hashopt};
               } else {
                  carp "$pkgname: invalid reference type for $hashopt option";
                  return 0;
               }
            } elsif ( $tag eq 'source') {
               $OPT{$tag}=$opt->{$hashopt};
            } else {
               carp "$pkgname: $hashopt is not a reference";
               return 0;
            }
         } elsif ($hashopt eq 'dest' or
               $hashopt eq 'destination' or
               $hashopt eq 'dst') {
               $OPT{'dest'}=$opt->{$hashopt};

         } elsif ($hashopt eq 'path-to-rsync' or
               $hashopt eq 'srchost') {
            $OPT{$savopt}=$opt->{$hashopt};
         } elsif ($hashopt eq 'outfun' or $hashopt eq 'errfun') {
            if (ref $opt->{$hashopt} eq 'CODE') {
               $OPT{$hashopt}=$opt->{$hashopt};
            } else {
               carp "$pkgname: $hashopt option is not a function reference";
               return 0;
            }
         } else {
            carp "$pkgname: $hashopt - unknown option";
            return 0;
         }
      }
   }
   return \%OPT;
}

sub _saveopts {
   # this method saves the data from the hash passed to it in the object's
   # hash
   my $self=shift;
   my $pkgname=ref $self;
   my $opts=shift;
   return 0 unless ref $opts eq 'HASH';
   foreach my $opt (keys %$opts) {
      if (exists $self->{'flag'}{$opt}) {
         $self->{'flag'}{$opt}=$opts->{$opt};
      } elsif (exists $self->{'scalar'}{$opt}) {
         $self->{'scalar'}{$opt}=$opts->{$opt};
      } elsif (exists $self->{'counter'}{$opt}) {
         $self->{'counter'}{$opt}=$opts->{$opt};
      } elsif ($opt eq 'exclude' or $opt eq 'include' or
            $opt eq 'source' or $opt eq 'dest' or $opt eq 'debug' or
            $opt eq 'outfun' or $opt eq 'errfun' or
            $opt eq 'path-to-rsync' or $opt eq 'srchost') {
         $self->{$opt}=$opts->{$opt};
      } else {
         carp "$pkgname: unknown option: $opt";
         return 0;
      }
   }
   return 1;
}

=over 4

=item File::Rsync::exec

I<exec> $obj @options or warn "rsync failed\n";

   or

$obj->exec(@options) or warn "rsync failed\n";

   or

$obj->exec(\%options) or warn "rsync failed\n";

This is the method that does the real work.  Any options passed to this
routine are appended to any pre-set options and are not saved.  They effect
the current execution of I<rsync> only.  In the case of conflicts, the options
passed directly to I<exec> take precedence.  It returns B<1> if the return
status was zero (or true), if the I<rsync> return status was non-zero it
returns B<0> and stores the return status.  You can examine the return status
from I<rsync> and any output to stdout and stderr with the methods listed below.

=back

=cut

sub exec {
   my $self=shift;
   my $pkgname=ref $self;
   my $merged=0;
   my $list=$self->{list};
   $self->{list}=0 if $self->{list};
   if (@_) { # If args are passed to exec then we have to merge the saved
      # (default) options with those passed, for any conflicts those passed
      # directly to exec take precidence, and perl-style options take
      # precidence over rsync command-line style options (because they offer
      # more flexibility)
      my $execopts=&_parseopts($self,@_);
      return 0 unless ref $execopts eq 'HASH';
      my %runopts=();
      # first copy the default info from $self
      foreach my $type (qw(flag scalar counter)) {
         foreach my $opt (keys %{$self->{$type}}) {
            $runopts{$type}{$opt}=$self->{$type}{$opt};
         }
      }
      foreach my $opt (qw(path-to-rsync exclude include
            source srchost debug dest outfun errfun)) {
         $runopts{$opt}=$self->{$opt};
      }
      # now allow any args passed directly to exec to override
      foreach my $opt (keys %$execopts) {
         if (exists $runopts{'flag'}{$opt}) {
            $runopts{'flag'}{$opt}=$execopts->{$opt};
         } elsif (exists $runopts{'scalar'}{$opt}) {
            $runopts{'scalar'}{$opt}=$execopts->{$opt};
         } elsif (exists $runopts{'counter'}{$opt}) {
            $runopts{'counter'}{$opt}=$execopts->{$opt};
         } elsif ($opt eq 'exclude' or $opt eq 'include' or
               $opt eq 'source' or $opt eq 'dest' or $opt eq 'debug' or
               $opt eq 'outfun' or $opt eq 'errfun' or
               $opt eq 'path-to-rsync' or $opt eq 'srchost') {
            $runopts{$opt}=$execopts->{$opt};
         } else {
            carp "$pkgname: unknown option: $opt";
            return 0;
         }
      }
      $merged=\%runopts;
   } else {
      $merged=$self;
   }

   my @cmd=($merged->{'path-to-rsync'});

   foreach my $opt (sort keys %{$merged->{'flag'}}) {
      push @cmd,"--$opt" if $merged->{'flag'}{$opt};
   }
   foreach my $opt (sort keys %{$merged->{'scalar'}}) {
      push @cmd,"--$opt=".$merged->{'scalar'}{$opt} if $merged->{'scalar'}{$opt};
   }
   foreach my $opt (sort keys %{$merged->{'counter'}}) {
      for (my $i=0;$i<$merged->{'counter'}{$opt};$i++) {
         push @cmd,"--$opt";
      }
   }
   if (@{$merged->{'exclude'}} and @{$merged->{'include'}}) {
      carp "$pkgname: both 'exclude' and 'include' options specified, only one allowed";
      return 0;
   }
   foreach my $opt (@{$merged->{'exclude'}}) {
      push @cmd,'--exclude='.$opt;
   }
   foreach my $opt (@{$merged->{'include'}}) {
      push @cmd,'--include='.$opt;
   }
   if ($merged->{'source'}) {
      if (ref $merged->{'source'}) {
         if ($merged->{'srchost'}) {
            push @cmd,$merged->{'srchost'}.':'.join ' ',@{$merged->{'source'}};
         } else {
            push @cmd,@{$merged->{'source'}};
         }
      } else {
         if ($merged->{'srchost'}) {
            push @cmd,$merged->{'srchost'}.':'.$merged->{'source'};
         } else {
            push @cmd,$merged->{'source'};
         }
      }
   } elsif ($merged->{'srchost'} and $list) {
      push @cmd,$merged->{'srchost'}.':';
   } else {
      if ($list) {
         carp "$pkgname: no 'source' specified";
         return 0;
      } elsif ($merged->{'dest'}) {
         carp "$pkgname: option 'dest' specified without 'source' option";
         return 0;
      } else {
         carp "$pkgname: no source or destination specified";
         return 0;
      }
   }
   unless ($list) {
      if ($merged->{'dest'}) {
         push @cmd,$merged->{'dest'};
      } else {
         carp "$pkgname: option 'source' specified without 'dest' option";
         return 0;
      }
   }
   print STDERR "exec: @cmd\n" if $merged->{'debug'};
   my $in=FileHandle->new; my $out=FileHandle->new; my $err=FileHandle->new;
   $err->autoflush(1);
   my $pid=eval{ open3 $in,$out,$err,@cmd };
   if ($@) {
      $self->{'realstatus'}=0;
      $self->{'status'}=255;
      $self->{'err'}=[$@,"Execution of rsync failed.\n"];
      return 0;
   }
   $in->close; # we don't use it and neither should rsync (at least not yet)
   my $rmask='';
   foreach my $fh ($err,$out) {
      my $tmask='';
      vec($tmask,$fh->fileno,1)=1;
      $rmask|=$tmask;
   }
   my $odata= my $edata='';
   my $done=0;
   my $opart;
   my $epart;
   while (not $done) {
      $done++ if (waitpid $pid,&WNOHANG);
      my $nfound=select(my $rout=$rmask,undef,undef,1);
      next unless $nfound;
      my @bits=split(//,unpack('b*',$rout));
      if ($bits[$out->fileno]) {
         my $data;
         while (my $c=sysread $out,$data,1024) {
            if ($self->{'outfun'}) {
               my $npart=$1 if ($data=~s/([^\n]+)\z//s);
               foreach my $line (split /^/m,$opart.$data) {
                  &{$self->{outfun}}($line,'out');
               }
               $opart=$npart;
            }
            $odata.=$data;
         }
      }
      if ($bits[$err->fileno]) {
         my $data;
         while (my $c=sysread $err,$data,1024) {
            if ($self->{'errfun'}) {
               my $npart=$1 if ($data=~s/([^\n]+)\z//s);
               foreach my $line (split /^/m,$epart.$data) {
                  &{$self->{errfun}}($line,'err');
               }
               $epart=$npart;
            }
            $edata.=$data;
         }
      }
      last if $out->eof and $err->eof;
   }
   $self->{'out'}=$odata ? [ split /^/m,$odata ] : '';
   $self->{'err'}=$edata ? [ split /^/m,$edata ] : '';
   $out->close;
   $err->close;
   waitpid $pid,0 unless $done;
   $self->{'realstatus'}=$?;
   $self->{'status'}=$?>>8;
   return($self->{'status'} ? 0 : 1);
}

=over 4

=item File::Rsync::list

$out = I<list> $obj @options;

   or

$out = $obj->list(@options);

   or

$out = $obj->list(%options);

   or

@out = $obj->list(%options);

This is a wrapper for I<exec> called without a destination to get a listing.
It returns the output of stdout like the I<out> function below.  When
no destination is given rsync returns the equivalent of 'ls -l' or 'ls -lr'
modified by any include/exclude parameters you specify.  This is useful
for manual comparison without actual changes to the destination or for
comparing against another listing taken at a different point in time.

=back

=cut

sub list {
   my $self=shift;
   $self->{list}++;
   $self->exec(@_);
   if ($self->{'out'}) {
      return(wantarray ? @{$self->{'out'}} : $self->{'out'});
   } else {
      return(wantarray ? () : $self->{'out'});
   }
}

=over 4

=item File::Rsync::status

$rval = I<status> $obj;

   or

$rval = $obj->I<status>;

Returns the status from last I<exec> call right shifted 8 bits.

=back

=cut

sub status {
   my $self=shift;
   return $self->{'status'};
}

=over 4

=item File::Rsync::realstatus

$rval = I<realstatus> $obj;

   or

$rval = $obj->I<realstatus>;

Returns the real status from last I<exec> call (not right shifted).

=back

=cut

sub realstatus {
   my $self=shift;
   return $self->{'realstatus'};
}

=over 4

=item File::Rsync::err

$aref = I<err> $obj;

   or

$aref = $obj->I<err>;

In a scalar context this method will return a reference to an array containing
all output to stderr from the last I<exec> call, or zero (false) if there
was no output.  In an array context it will return an array of all output to
stderr or an empty list.  The scalar context can be used to efficiently test
for the existance of output.  I<rsync> sends all messages from the remote
I<rsync> process and any error messages to stderr.  This method's purpose is
to make it easier for you to parse that output for appropriate information.

=back

=cut

sub err {
   my $self=shift;
   if ($self->{'err'}) {
      return(wantarray ? @{$self->{'err'}} : $self->{'err'});
   } else {
      return(wantarray ? () : $self->{'err'});
   }
}

=over 4

=item File::Rsync::out

$aref = I<out> $obj;

   or

$aref = $obj->I<out>;

Similar to the I<err> method, in a scalar context it returns a reference to an
array containing all output to stdout from the last I<exec> call, or zero
(false) if there was no output.  In an array context it returns an array of all
output to stdout or an empty list.  I<rsync> sends all informational messages
(B<verbose> option) from the local I<rsync> process to stdout.

=back

=cut

sub out {
   my $self=shift;
   if ($self->{'out'}) {
      return(wantarray ? @{$self->{'out'}} : $self->{'out'});
   } else {
      return(wantarray ? () : $self->{'out'});
   }
}

=head1 Author

Lee Eakin E<lt>leakin@nostrum.comE<gt>

=head1 Credits

Gerard Hickey                             C<PGP::Pipe>

Russ Allbery                              C<PGP::Sign>

Graham Barr                               C<Net::*>

Andrew Tridgell and Paul Mackerras        rsync(1)

John Steele   E<lt>steele@nostrum.comE<gt>

Philip Kizer  E<lt>pckizer@nostrum.comE<gt>

Larry Wall                                perl(1)

I borrowed many clues on wrapping an external program from the PGP modules,
and I would not have had such a useful tool to wrap except for the great work
of the B<rsync> authors.  Thanks also to Graham Barr, the author of the libnet
modules and many others, for looking over this code.  Of course I must mention
the other half of my brain, John Steele, and his good friend Philip Kizer for
finding B<rsync> and bringing it to my attention.  And I would not have been
able to enjoy writing useful tools if not for the creator of the B<perl>
language.

=head1 Copyrights

      Copyright (c) 1999 Lee Eakin.  All rights reserved.
 
      This program is free software; you can redistribute it and/or modify
      it under the same terms as Perl itself. 

=cut

1;
