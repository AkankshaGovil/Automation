package Date::EzDate;
use strict;
use Carp;
use vars ('$offset', '$VERSION');

# version
$VERSION = '0.93';

# documentation at end of file

#========================================================================================
# calculate offset
# 
{
	my %date;
	@date{'sec','min','hour','mday','mon','year','wday','yday','isdst'} = localtime(0);
	$offset = ( ($date{'hour'} + 1) * 60 * 60) + ($date{'min'} * 60) + $date{'sec'};
}
# 
# calculate offset
#========================================================================================


#========================================================================================
# new
#
sub new {
	my ($class, $init) = @_;
	my (%tiehash);
	tie %tiehash, 'Date::EzDateTie', $init;
	return bless(\%tiehash, $class);
}
#
# new
#========================================================================================


#========================================================================================
# clone
#
sub clone {
	my ($self) = @_;
	my $ob = tied(%{$self});
	
	return ref($self)->new([
		$ob->{'sec'},
		$ob->{'min'},
		$ob->{'hour'},
		$ob->{'dayofmonth'},
		$ob->{'monthnum'},
		$ob->{'year'},
		$ob->{'weekdaynum'},
		$ob->{'yearday'},
		$ob->{'dst'},
		$ob->{'epochsec'}
		]);
}
#
# clone
#========================================================================================


#========================================================================================
# get_settings
#
sub get_settings {
	my ($self) = @_;
	my $ob = tied(%{$self});
	return $ob->{'settings'};
}
#
# get_settings
#========================================================================================


#========================================================================================
# nextmonth
sub nextmonth {
	my ($self, $inc) = @_;
	my ($target);

	unless (defined $inc)
		{$inc = 1}
	return unless $inc;
	$target = $inc;

	if ($target < 0)
		{$target *= -1}
	
	
	# jumping forward
	if ($inc > 0) {
		foreach (1..$target) {
			my ($month, $year);
			
			# if end of year
			if ($self->{'monthnum'} == 11) {
				$month = 0;
				$year = $self->{'year'} + 1;
			}
			else {
				$month = $self->{'monthnum'} + 1;
				$year = $self->{'year'};
			}
			$self->{'year'} = $year;
			$self->{'monthnum'} = $month;
		}
	}

	# jumping backward
	else {
		foreach (1..$target) {
			my ($month, $year);
			
			# if beginning of year
			if ($self->{'monthnum'} == 0) {
				$month = 11;
				$year = $self->{'year'} - 1;
			}
			else {
				$month = $self->{'monthnum'} - 1;
				$year = $self->{'year'};
			}
			$self->{'year'} = $year;
			$self->{'monthnum'} = $month;
		}
	}
}



########################################################################################################################
package Date::EzDateTie;
use strict;
use Carp;
use Tie::Hash;
use Time::Local;
@Date::EzDateTie::ISA = ('Tie::StdHash');

# globals
@EzDate::WeekDayShort = ('Sun','Mon','Tue','Wed','Thu','Fri','Sat');
@EzDate::WeekDayLong =  ('Sunday','Monday','Tuesday','Wednesday','Thursday','Friday','Saturday');
@EzDate::MonthShort = ('Jan',      'Feb',      'Mar',   'Apr',   'May',  'Jun',   'Jul',  'Aug',    'Sep',       'Oct',     'Nov',      'Dec');
@EzDate::MonthLong =  ('January',  'February', 'March', 'April', 'May',  'June',  'July', 'August', 'September', 'October', 'November', 'December');
@EzDate::MonthDays =  (31,         'x',        31,      30,      31,      30,     31,     31,       30,          31,        30,         31);
%EzDate::WeekDayNums = ();
@EzDate::WeekDayNums{'sun', 'mon', 'tue', 'wed', 'thu', 'fri', 'sat'}=(0..6);
%EzDate::MonthNums = ();
@EzDate::MonthNums{'jan', 'feb', 'mar', 'apr', 'may', 'jun', 'jul', 'aug', 'sep', 'oct', 'nov', 'dec'}=(0..11);

# constants
use constant 't_60_60' => 3600;
use constant 't_60_60_24' =>  86400;


#========================================================================================

sub TIEHASH {
	my ($class, $time)=@_;
	my $self = bless ({}, $class);

	# set some non-date properties
	$self->{'formats'} = {};
	$self->{'settings'} = {'dst_kludge' => 1};


	# if clone
	if (ref($time))
		{@{$self}{'sec','min','hour','dayofmonth','monthnum','year','weekdaynum','yearday','dst', 'epochsec'}=@{$time}}

	else {
		# calculate and set properties of current time
		# set time from timefromfull

		$self->setfromtime(time());		

		if ($time)
			{$self->setfromtime($self->timefromfull($time))}
	}

	return  $self;
}



#========================================================================================

sub setfromtime {
	my ($self, $time) = @_;
	
	$self->{'epochsec'} = $time;
	@{$self}{'sec','min','hour','dayofmonth','monthnum','year','weekdaynum','yearday','dst'}=localtime($time);
	$self->{'year'} += 1900;
}


#========================================================================================

sub STORE {
	my ($self, $key, $val) = @_;
	my $orgkey = $key;
	my $orgval = $val;
	$key = lc($key);
	($key =~ s/minute$/min/) or ($key =~ s/second$/sec/);
	
	# error checking
	if (! defined $val)
		{croak 'Must send a defined value when setting a property of an EzDate object: key=' . $key}

	elsif ($key =~ m/^(dayofmonth)|(weekdaynum)|(yearday)$/) {
		$self->setfromtime($self->{'epochsec'} - ($self->{$key} *  t_60_60_24) + ($val * t_60_60_24) );
	}
	
	elsif ($key eq 'sec')
		{$self->setfromtime($self->{'epochsec'} - $self->{'sec'} + $val)}
	
	elsif ($key eq 'min')
		{$self->setfromtime($self->{'epochsec'} - ($self->{'min'} * 60) + ($val * 60) )}
	
	elsif ($key eq 'minofday')
		{$self->setfromtime($self->{'epochsec'} - ($self->{'hour'} * t_60_60)  - ($self->{'min'} * 60) + ($val * 60) )}
	
	elsif ($key eq 'hour')
		{$self->setfromtime($self->{'epochsec'} - ($self->{'hour'} * t_60_60) + ($val * t_60_60) )}
	
	# hour and minute
	elsif ( ($key eq 'clocktime') || ($key eq 'miltime') ) {
		
		my ($changed, $hour, $min, $sec) = gettime($val);

		
		unless (defined $hour)
			{$hour = $self->{'hour'}}
		unless (defined $min)
			{$min = $self->{'min'}}
		unless (defined $sec)
			{$sec = $self->{'sec'}}

		#$self->{'full'} = "$hour:$min:$sec";
		$self->setfromtime
			(
			$self->{'epochsec'}
			
			- ($self->{'sec'})
			- ($self->{'min'} * 60)
			- ($self->{'hour'} * t_60_60)
			
			+ ($sec)
			+ ($min * 60)
			+ ($hour * t_60_60)
			);
		
		#exit;
	}
	
	elsif ($key eq 'ampmhour') {
		if ($self->{'hour'} >= 12)
				{$val += 12}
		$self->STORE('hour', $val);
	}
	
	elsif (
		($key eq 'ampm') || 
		($key eq 'ampmlc') || 
		($key eq 'ampmuc')
		) {
		my ($multiplier);
		$val = lc($val);
		
		# error checking
		unless ( ($val eq 'am') || ($val eq 'pm') )
			{croak 'ampm may only be set to am or pm'}
		
		# if no change, we're done
		if ($self->{'hour'} < 12) {
			if ($val eq 'am') {return}
		}
		else {
			if ($val eq 'pm') {return}
		}
		
		if ($val eq 'am')
			{$multiplier = -1}
		else
			{$multiplier = 1}
		
		$self->setfromtime($self->{'epochsec'} + (12 * t_60_60 * $multiplier) );
	}
	
	elsif ($key eq 'dst')
		{croak 'dst property is read-only'}
	
	elsif ($key eq 'epochsec')
		{$self->setfromtime($val)}
	
	elsif ($key eq 'epochmin')
		{$self->setfromtime($self->{'epochsec'} - ($self->getepochmin * 60) + ($val * 60) )}
	
	elsif ($key eq 'epochhour')
		{$self->setfromtime($self->{'epochsec'} - ($self->getepochhour * t_60_60) + ($val * t_60_60) )}
	
	elsif ($key eq 'epochday') {
		my ($oldhour, $oldmin);
		
		# kludge issues with DST
		if ($self->{'settings'}->{'dst_kludge'}) {
			$oldhour = $self->{'hour'};
			$oldmin = $self->{'min'};
		}
		
		$self->setfromtime($self->{'epochsec'} - ($self->getepochday * t_60_60_24) + (int($val) * t_60_60_24) );
		# $self->setfromtime($self->{'epochsec'} - ($self->getepochday * t_60_60_24) + ($val * t_60_60_24) );
		
		# kludge issues with DST
		if (
			$self->{'settings'}->{'dst_kludge'} && 
			($oldhour != $self->{'hour'})
			) {
			# spring forward
			# if (($oldhour == 0) && ($self->{'hour'} == 1)) 
			if ($oldhour == ($self->{'hour'} - 1)  )
				{$self->setfromtime($self->{'epochsec'} - t_60_60)}
			
			# fall back
			elsif (
				(($oldhour == 0) && ($self->{'hour'} == 23)) ||
				($oldhour == ($self->{'hour'} + 1)  )
				)
				{$self->setfromtime($self->{'epochsec'} + t_60_60)}
			
			# else die
			else
				{die "unable to handle epochday++ for date [\$oldhour=$oldhour] [\$self->{'hour'}=$self->{'hour'}]"}
		}

	}
	
	elsif ($key eq 'year') {
		my ($maxday, $targetday);

		# if same year, nothing to do
		if ($self->{'year'} == $val)
			{return}
		
		# make sure day of month isn't greater than maximum day of target month
		$maxday = daysinmonth($self->{'monthnum'}, $val);
		if ($self->{'dayofmonth'} > $maxday)
			{$targetday = $maxday}
		else
			{$targetday = $self->{'dayofmonth'}}
		
		$val = timelocal(0,0,0,$targetday, $self->{'monthnum'}, $val);
		$val /= (24*60*60);
		# my $addone = ($val == int($val)) ? 0 : 1;
		
		$val = int($val) + 1;
		$self->STORE('epochday', $val);
		
		# KLUDGE: not sure why, but sometimes this produces an off-by-one problem
		# still seeking a solution
		if ($self->{'dayofmonth'} > $targetday)
			{$self->STORE('dayofmonth', $self->{'dayofmonth'}-1)}
	}
	
	elsif ($key =~ m/^yeartwodigit/)
		{$self->STORE('year', substr($self->{'year'}, 0, 2), $val)}
	
	elsif ($key =~ m/^monthnumbase(one|1)/)
		{$self->STORE('monthnum', $val - 1)}
	
	elsif ($key eq 'monthnum') {
		my ($i, $maxday, $targetday, $cepoch);

		# if same month, we're done
		if ($val == $self->{'monthnum'})
			{return}

		# make sure day of month isn't greater than maximum day of target month
		$maxday= daysinmonth($val, $self->{'year'});
		if ($self->{'dayofmonth'} > $maxday)
			{$targetday = $maxday}
		else
			{$targetday = $self->{'dayofmonth'}}

		# if we should increment or decrement
		$i = ($val > $self->{'monthnum'}) ? 1 : -1;

		$cepoch = $self->{'epochsec'};

		
		my $sanity = 1000;

		# loop until we get to this day in next or previous month
		while (1) {
			my ($newdaynum, $newmonthnum);
			$cepoch = $cepoch + (t_60_60_24 * $i);
			
			
			if ($sanity-- <= 0)
				{die 'INSANE'}

			# @{$self}{'sec','min','hour','dayofmonth','monthnum','year','weekdaynum','yearday','dst'}=localtime($time);
			($newdaynum, $newmonthnum) = (localtime($cepoch))[3,4];

			# if we have a match, set to new month
			if ( ($newdaynum == $targetday) && ($newmonthnum == $val) ) {
				$self->setfromtime($cepoch);
				return;
			}
		}
	}
	
	elsif ( ($key eq 'monthshort') || ($key eq 'monthlong') ) {
		$val = lc(substr($val, 0, 3));
		$val = $EzDate::MonthNums{$val};
		
		# error checking
		if (! defined $val)
			{croak "Do not understand month: $orgval"}

		$self->STORE('monthnum', $val);
	}
	
	elsif ( ($key eq 'weekdayshort') || ($key eq 'weekdaylong') ) {
		$val = lc(substr($val, 0, 3));
		$val = $EzDate::WeekDayNums{$val};
		

		# error checking
		if (! defined $val)
			{croak "Do not understand weekday: $orgval"}

		$self->STORE('weekdaynum', $val);
	}
	
	# full, dmy, printable time, printable date, printabletimeno
	elsif (
		($key eq 'full') || 
		($key eq 'dmy') || 
		($key eq 'printabletime') || 
		($key eq 'printabletimeno') || 
		($key eq 'printabledate')
		){
			my (%opts);
			
			if ( ($key eq 'dmy') || ($key eq 'printabledate') )
				{$opts{'dateonly'} = 1}
			elsif ( ($key eq 'printabletime') || ($key eq 'printabletimeno') )
				{$opts{'timeeonly'} = 1}


			$self->setfromtime($self->timefromfull($val, %opts));
		}
	
	else
		{croak "Do not understand key: $orgkey"}
}


#========================================================================================

sub FETCH {
	my ($self, $key) = @_;
	my ($ampm, $ampmhour);
	
	#---------------------------------------------------------------------------------------------------
	# formatted string
	# 
	# if there is a % in the string, return formatted string
	# 
	if ($key =~ m/%/) {
		my $rv = $key;
		
		# weekday, short	%a	Mon
		if ($rv =~ m/%a/) {
			my $s = $self->FETCH('weekdayshort');
			$rv =~ s/%a/$s/g
		}
		
		# weekday, long	%A	Monday
		if ($rv =~ m/%A/) {
			my $s = $self->FETCH('weekdaylong');
			$rv =~ s/%A/$s/g
		}
		
		# full date	%c	Mon Aug 10 14:40:38 1998
		if ($rv =~ m/%c/) {
			my $s = $self->FETCH('%A %h %d %H:%M:%S %Y');
			$rv =~ s/%c/$s/g
		}
		
		# numeric day of the month	%d	10
		if ($rv =~ m/%d/) {
			my $s = $self->FETCH('dayofmonth');
			$rv =~ s/%d/$s/g
		}
		
		# date as month/date/year	%D	08/10/98
		if ($rv =~ m/%D/) {
			my $s = $self->FETCH('%m/%d/%y');
			$rv =~ s/%D/$s/g
		}
		
		# short month	%h	Aug
		if ($rv =~ m/%h/) {
			my $s = $self->FETCH('monthshort');
			$rv =~ s/%h/$s/g
		}
		
		# hour 00 to 23	%H	14
		if ($rv =~ m/%H/) {
			my $s = $self->FETCH('hour');
			$rv =~ s/%H/$s/g
		}
		
		# hour 00 to 12	%b, no leading zero
		if ($rv =~ m/%b/) {
			my $s = $self->FETCH('ampmhour');
			$s += 0;
			$rv =~ s/%b/$s/g
		}
		
		# hour 00 to 23	%B, no leading zero
		if ($rv =~ m/%B/) {
			my $s = $self->FETCH('hour');
			$s += 0;
			$rv =~ s/%B/$s/g
		}
		
		# numeric month, 1 to 12, no leading zero	%e	8
		if ($rv =~ m/%e/) {
			my $s = $self->FETCH('monthnumbase1');
			$s += 0;
			$rv =~ s/%e/$s/g
		}
		
		# numeric day of month, 1 to 12	%f	3
		if ($rv =~ m/%f/) {
			my $s = $self->FETCH('dayofmonth');
			$s += 0;
			$rv =~ s/%f/$s/g
		}
		
		# day of the year, 001 to 366	%j	222
		if ($rv =~ m/%j/) {
			my $s = $self->FETCH('yearday');
			$s++;  # increment by one to make it one-based
			$rv =~ s/%j/$s/g
		}
		
		# hour, 12 hour format	%k	14
		if ($rv =~ m/%k/) {
			my $s = $self->FETCH('ampmhour');
			$rv =~ s/%k/$s/g
		}
		
		# numeric month, 01 to 12	%m	08
		if ($rv =~ m/%m/) {
			my $s = $self->FETCH('monthnumbase1');
			$rv =~ s/%m/$s/g
		}
		
		# minutes	%M	40
		if ($rv =~ m/%M/) {
			my $s = $self->FETCH('min');
			$rv =~ s/%M/$s/g
		}
		
		# newline	%n
		if ($rv =~ m/%n/) {
			$rv =~ s/%n/\n/g
		}
		
		# AM/PM	%P	PM
		if ($rv =~ m/%P/) {
			my $s = uc($self->FETCH('ampm'));
			$rv =~ s/%P/$s/g
		}
		
		# am/pm	%p	pm
		if ($rv =~ m/%p/) {
			my $s = lc($self->FETCH('ampm'));
			$rv =~ s/%p/$s/g
		}
		
		# hour:minute:second AM/PM	%r	02:40:38 PM
		if ($rv =~ m/%r/) {
			my $s = $self->FETCH('%k:%M:%S %p');
			$rv =~ s/%r/$s/g
		}
		
		# number of seconds since the start of 1970	%s	902774438
		if ($rv =~ m/%s/) {
			my $s = $self->FETCH('epochsec');
			$rv =~ s/%s/$s/g
		}
		
		# seconds	%S	38
		if ($rv =~ m/%S/) {
			my $s = $self->FETCH('sec');
			$rv =~ s/%S/$s/g
		}
		
		# tab	%t
		if ($rv =~ m/%t/) {
			$rv =~ s/%t/\t/g
		}
		
		# hour:minute:second (24 hour format)	%T	14:40:38
		if ($rv =~ m/%T/) {
			my $s = $self->FETCH('%H:%M:%S');
			$rv =~ s/%T/$s/g
		}
		
		# numeric day of the week, 0 to 6 (Sunday is 0)	%w	1
		if ($rv =~ m/%w/) {
			my $s = $self->FETCH('weekdaynum');
			$rv =~ s/%w/$s/g
		}
		
		# last two digits of the year	%y	98
		if ($rv =~ m/%y/) {
			my $s = $self->FETCH('yeartwodigits');
			$rv =~ s/%y/$s/g
		}
		
		# four digit year	%Y	1998
		if ($rv =~ m/%Y/) {
			my $s = $self->FETCH('year');
			$rv =~ s/%Y/$s/g
		}
		
		# time zone	%Z	EDT
		
		# percent sign	%%	%
		if ($rv =~ m/%%/) {
			$rv =~ s/%%/%/g
		}
		
		return $rv;
	}
	# 
	# formatted string
	#---------------------------------------------------------------------------------------------------
	
	# clean up key
	$key = lc($key);
	($key =~ s/minute$/min/) or ($key =~ s/second$/sec/);
	
	# already or mostly calculated
	if (exists $self->{$key}) {
		if ($key =~ m/^(dayofmonth|monthnum|hour|min|sec)$/)
			{return zeropad($self->{$key})}
		return $self->{$key};
	}
	
	# weekday
	if ($key eq 'weekdayshort')
		{return $EzDate::WeekDayShort[$self->{'weekdaynum'}]}
	if ($key eq 'weekdaylong')
		{return $EzDate::WeekDayLong[$self->{'weekdaynum'}]}
	
	# month
	if ($key eq 'monthshort')
		{return $EzDate::MonthShort[$self->{'monthnum'}]}
	if ($key eq 'monthlong')
		{return $EzDate::MonthLong[$self->{'monthnum'}]}
	if ($key =~ m/^monthnumbase(one|1)/)
		{return zeropad($self->{'monthnum'} + 1, 2)}
	

	# year
	if ($key =~ m/^yeartwodigit/)
		{return substr($self->{'year'}, 2)}
	
	# epochs
	if ($key eq 'epochmin')
		{return $self->getepochmin}
	if ($key eq 'epochhour')
		{return $self->getepochhour}
	if ($key eq 'epochday')
		{return $self->getepochday}
	
	# leapyear
	if ($key =~ m/^(is){0,1}leapyear/)
		{return isleapyear($self->{'year'})}

	# days in month
	if ($key eq 'daysinmonth')
		{return daysinmonth($self->{'monthnum'}, $self->{'year'}) }

	# DMY: eg 15JAN2001
	if ($key eq 'dmy')
		{return zeropad($self->{'dayofmonth'}, 2) . uc($EzDate::MonthShort[$self->{'monthnum'}]) . $self->{'year'} }
	
	# full
	if ($key eq 'full') {
		return 
			zeropad($self->{'hour'})   . ':' . 
			zeropad($self->{'min'})    . ':' . 
			zeropad($self->{'sec'})    . ' ' . 
			$EzDate::WeekDayShort[$self->{'weekdaynum'}]    . ' ' . 
			$EzDate::MonthShort[$self->{'monthnum'}]      . ' ' . 
			$self->{'dayofmonth'}      . ', ' . 
			$self->{'year'};
	}
	
	# printable date
	if ($key eq 'printabledate')
		{return $EzDate::MonthShort[$self->{'monthnum'}] . " $self->{'dayofmonth'}, $self->{'year'}"}

	# military time ("miltime")
	if ($key eq 'miltime') 
		{return zeropad($self->{'hour'}) . zeropad($self->{'min'}) }

	# minuteofday, aka minofday
	if ($key eq 'minofday') 
		{return $self->{'min'} + ($self->{'hour'} * 60) }

	# calculate ampm, which is needed in most results from here down
	if ($self->{'hour'} >= 12)
		{$ampm = 'pm'}
	else
		{$ampm = 'am'}
	
	# am/pm	
	if (
		($key eq 'ampm') || 
		($key eq 'ampmlc') 
		)
		{return $ampm}
	
	# AM/PM	uppercase
	if ($key eq 'ampmuc') 
		{return uc($ampm)}
	
	# calculate ampmhour, which is needed from here down
	if ( ($self->{'hour'} == 0) || ($self->{'hour'} == 12) )
		{$ampmhour = 12}
	elsif ($self->{'hour'} > 12)
		{$ampmhour = $self->{'hour'} - 12}
	else
		{$ampmhour = $self->{'hour'}}
	
	# am/pm hour
	if ($key eq 'ampmhour')
		{return zeropad($ampmhour)}

	# hour and minute with ampm
	if ($key eq 'clocktime')
		{return zeropad($self->{'hour'}) . ':' . zeropad($self->{'min'}) . ':' . zeropad($self->{'sec'})  . ' ' . $ampm }

	# printable time
	if ($key =~ m/^printabletime/) {
		my $rv = $ampmhour . ':' . zeropad($self->{'min'});

		if ($key eq 'printabletime')
			{$rv .= ' ' . lc($ampm)}

		return $rv;
	}

	
	
	# else we don't know what property is needed
	return undef;
}


#========================================================================================
sub isleapyear {
	my ($year) = @_;

	unless (defined $year)
		{croak 'got undefined year'}
	
	return 1 if ( ($year % 4 == 0) && ( ($year % 100) || ($year % 400 == 0) ) );
	return 0;
}


#========================================================================================
# calculate epochs
#

sub getepochday {
	my ($self) = @_;

	return int( ($self->{'epochsec'} + ${Date::EzDate::offset} ) / t_60_60_24);
}

sub getepochhour {
	my ($self) = @_;
	return int($self->{'epochsec'} / t_60_60);
}

sub getepochmin {
	my ($self) = @_;
	return int($self->{'epochsec'} / 60);
}


#========================================================================================
sub daysinmonth {
	my ($monthnum, $year) = @_;

	if ($monthnum != 1)
		{return $EzDate::MonthDays[$monthnum]}
	if (isleapyear($year))
		{return 29}
	return 28;

}


#========================================================================================
# timefromfull
sub timefromfull {
		my ($self, $val, %opts) = @_;
		my $orgval = $val;
		my ($hour, $min, $sec, $day, $month, $year);
		
		# error checking
		if (! defined $val)
			{croak "did not get a time string"}

		# quick return: if they just put in an integer
		if ($val =~ m/^\d+$/)
			{return $val}
		
		# normalize
		$val = lc($val);
		$val =~ s/[^\w:]/ /g;
		$val =~ s/\s*:\s*/:/g;
		$val =~ s/(\d)([a-z])/$1 $2/g;
		$val =~ s/([a-z])(\d)/$1 $2/g;
		$val =~ s/\s+/ /g;
		$val =~ s/^\s*//;
		$val =~ s/\s*$//;
		$val =~ s/([a-z]{3})[a-z]+/$1/g;
		
		# remove weekday
		$val =~ s/((sun)|(mon)|(tue)|(wed)|(thu)|(fri)|(sat))\s*//;
		$val =~ s/\s*$//;

		# attempt to get time
		unless ($opts{'dateonly'}) {
			($val, $hour, $min, $sec) = gettime($val, 'skipjustdigits'=>1);
		}
		
		# attempt to get date
		unless ($opts{'timeonly'}) {
			if (length $val)
				{($val, $day, $month, $year) = getdate($val)}
		}
		
		# trim
		$val =~ s/^\s*//;
		
		# attempt to get time again
		unless ($opts{'dateonly'}) {
			if (length($val) && (! defined($hour)) ) {
				($val, $hour, $min, $sec) = gettime($val, 'skipjustdigits'=>1, 'croakonfail'=>1);
			}
		}
		
		# if we didn't get a day or an hour, we didn't recognize the pattern
		if (length($val) )
			{croak "Did not recognize date/time pattern [$val]: $orgval"}
		
		# default everything that isn't defined
		unless (defined $hour)
			{$hour = $self->{'hour'}}
		unless (defined $min)
			{$min = $self->{'min'}}
		unless (defined $sec)
			{$sec = $self->{'sec'}}
		
		unless (defined $month)
			{$month = $self->{'monthnum'}}
		unless (defined $year)
			{$year = $self->{'year'}}
		unless (defined $day)
			{$day = maxday($self->{'dayofmonth'}, $month, $year)}
		
		# set year to four digits
		if (length($year) == 2)
			{$year = substr($self->{'year'}, 0, 2) . $year}
		
		# return
		return timelocal($sec, $min, $hour, $day, $month, $year);
		
		# get date sub
		# attempt to get date
		# supported date formats
		# 14 Jan 2001
		# 14 JAN 01
		# 14JAN2001
		
		# Jan 14, 2001
		# Jan 14, 01
		
		# 01-14-01
		# 1-14-01
		# 1-7-01
		# 01-14-2001
		sub getdate {
			my ($val, $day, $month, $year) = @_;

			# 14 Jan 2001
			# 14 JAN 01
			# 14JAN2001   # will be normalized to have spaces
			if ($val =~ s/^(\d+) ([a-z]+) (\d+)//) {
				$day = $1;
				$month = $EzDate::MonthNums{$2};
				$year = $3;
			}
			
			# Jan 14, 2001
			# Jan 14, 01
			elsif ($val =~ s/^([a-z]+) (\d+) (\d+)//) {
				$month = $EzDate::MonthNums{$1};
				$day = $2;
				$year = $3;
			}
			
			# Jan 2001
			# Jan 01
			elsif ($val =~ s/^([a-z]+) (\d+)//) {
				$month = $EzDate::MonthNums{$1};
				$year = $2;
			}
			
			# 01-14-01
			# 1-14-01
			# 1-7-01
			# 01-14-2001
			elsif ($val =~ s/^(\d+) (\d+) (\d+)//) {
				$month = $1 - 1;
				$day = $2;
				$year = $3;
			}
			
			return ($val, $day, $month, $year);
		}
		
		
		sub ampmhour {
			my ($hour, $ampm)=@_;
			
			# if 12
			if ($hour == 12) {
				# if am, set to 0
				if ($ampm =~ m/^a/)
					{$hour = 0}
			}
			
			# else if pm, add 12 
			elsif ($ampm =~ m/^p/)
				{$hour += 12}
			return $hour;
		}

		#exit 0;
		#croak 'full property is read-only';
}



# get time sub
# supported time formats:
# 5pm
# 5:34 pm
# 17:34
# 17:34:13
# 5:34:13
# 5:34:13 pm
# 2330 (military time)
sub gettime {
	my ($str, %opts)= @_;
	my ($hour, $min, $sec);


	# clean up a little
	$str =~ s/^://;
	$str =~ s/:$//;


	# 5:34:13 pm
	# 5:34:13 p
	#if ($str =~ s/^(\d+):(\d+):(\d+) (a|p)m{0,1}\s*//) {
	if ($str =~ s/^(\d+):(\d+):(\d+) (a|p)(m|\b)\s*//) {

		$hour = ampmhour($1, $4);
		$min = $2;
		$sec = $3;
	}
	
	# 17:34:13
	elsif ($str =~ s/^(\d+):(\d+):(\d+)\s*//) {

		$hour = $1;
		$min = $2;
		$sec = $3;
	}
	
	# 5:34 pm
	elsif ($str =~ s/^(\d+):(\d+) (a|p)m{0,1}\s*//) {
		$hour = ampmhour($1, $3);
		$min = $2;
	}
	
	# 17:34
	elsif ($str =~ s/^(\d+):(\d+)\s*//) {
		$hour = $1;
		$min = $2;
	}
	
	# 5 pm
	elsif ($str =~ s/^(\d+) (a|p)m{0,1}\b\s*//)
		{$hour = ampmhour($1, $2)}
	
	# elsif just digits
	elsif ( (! $opts{'skipjustdigits'}) && ($str =~ m/^\d+$/) ) {
		$str = zeropad($str, 4);
		$hour = substr($str, 0, 2);
		$min = substr($str, 2, 2);
	}
	
	# else don't recognize format
	elsif ($opts{'croakonfail'}) {
		croak "don't recognize time format: $str";
	}


	return ($str, $hour, $min, $sec);
}


# if the input day is too high for given months, 
# returns the highest possible day for that month,
# otherwise returns the input day
sub maxday {
	my ($day, $month, $year) = @_;
	my $maxday = daysinmonth($month, $year);
	
	if ($day > $maxday)
		{return $maxday}
	return $day;
}


# this may be a better way to do it
# will incorporate at some point
# pad the value with prepended zeros
#sub zeropad($@) {
#	my $len = shift;
#	$_=sprintf "%0${len}d", $_ for @_;
#}


sub zeropad {
	my ($rv, $length) = @_;
	$length ||= 2;
	return ('0' x ($length - length($rv))) . $rv;
}


# return true
1;
__END__

=head1 NAME

Date::EzDate --  Date manipulation made easy.


=begin devmeta

email: miko@idocs.com
stage: beta

=end devmeta


=head1 Synopsis

An EzDate object represents a single point in time and exposes all properties of that 
point. EzDate has many features, here are a few:

 use Date::EzDate;
 my $mydate = Date::EzDate->new();

 # output some date information
 print $mydate->{'full'}, "\n";  # e.g. output:  09:06:26 Wed Apr 11, 2001

 # go to next day
 $mydate->{'epochday'}++;

 # output some other date and time information
 # e.g. output:  Thursday April 12, 2001 09:06 am
 print
	$mydate->{'weekdaylong'},   ' ',
	$mydate->{'monthlong'},     ' ',
	$mydate->{'dayofmonth'},    ', ',
	$mydate->{'year'},          ' ',
	$mydate->{'ampmhour'},      ':',
	$mydate->{'min'},           ' ',
	$mydate->{'ampm'},          "\n";

 # go to Monday of same week, but be lazy and don't spell out 
 # the whole day or case it correctly
 $mydate->{'weekdaylong'} = 'MON';

 print $mydate->{'full'}, "\n";  # e.g. output:  09:06:26 Mon Apr 09, 2001

 # go to previous year
 $mydate->{'year'}--;

 print $mydate->{'full'}, "\n";  # e.g. output:  09:06:26 Sun Apr 09, 2000

=head1 Description

Date::EzDate was motivated by the simple fact that I hate dealing with date and time calculations,
so I put all of them into a single easy-to-use object. The main idea of EzDate is that the object 
represents a specific date and time.  A variety of properties tell you information about that date 
and time such as hour, minute, day of month, weekday, etc.  

The B<real> power of EzDate is that you can assign to (almost) any of those 
properties and EzDate will automatically rework the other properties to produce a new valid 
date with the property you just assigned.  Properties that can be kept the same with the 
new value aren't changed, while those that logically must change to accomodate the new 
value are recalculated.  For example, incrementing I<epochday> by one (i.e. moving the date forward 
one day) does not change the hour or minute but does change the day of week.

So, for example, suppose you wanted to get information about today, then get 
information about tomorrow.  That can be done using the I<epochday> property
which is used for day-granularity calculations.  Let's walk through the steps:

=over

=item Load the module and instantiate the object

 use Date::EzDate;
 my $mydate = Date::EzDate->new();  # the object defaults to the current date and time

=item output all the basic information

 # e.g. outputs:  11:11:40 am, Wed Apr 11, 2001
 print $mydate->{'full'}, "\n";

=item set to tomorrow

To move the date forward one day we simply increment the I<epochday> property (number of days 
since the epoch).   The time (i.e. hour:min:sec) of the object does not change.

 $mydate->{'epochday'}++;

 # outputs:  11:11:40 am, Thu Apr 12, 2001
 print $mydate->{'full'}, "\n";

=back

This demonstrates the basic concept: almost any of the properties can be set as well as read and EzDate will take care
of resetting all other properties as needed.

=head1 Methods

EzDate is noticeably lacking in methods.  Almost everything EzDate does is through reading and setting properties.  
Currently there is only one static method (C<new()>) and two object methods (C<clone()> and C<nextmonth()>).

=head2 new([I<date string>])

Currently, EzDate only accepts a single optional argument when instantiated.  You may pass in either 
a Perl time integer or a string formatted as DDMMMYYYY.  If you don't pass in any argument 
then the returned object represents the time and day at the moment it was created.

The following are valid ways to instantiate an EzDate object:

 # current date and time
 my $date = Date::EzDate->new();
 
 # a specific time (23:27:39, Tue Apr 10, 2001 if you're curious) 
 my $date = Date::EzDate->new(986959659);
 
 # a date in DDMMMYYYY format
 my $date = Date::EzDate->new('14JAN2003');
 
 # a little forgiveness is built in
 my $date = Date::EzDate->new('14 January, 2003');
 

=head2 $mydate->clone()

This method returns an EzDate object exactly like the object it was called from.  C<clone> is much 
cheaper than creating a new EzDate object and then setting the new object to have the same properties 
as another EzDate object.

=head2 $mydate->nextmonth([integer])

EzDate lacks an C<epochmonth> month property (samo samo: haven't figured out 
how to do it yet) so it needed a way to say "same day, next month".  Calling 
C<nextmonth> w/o any argument moves the object to the same day in the next month.
If the day doesn't exist in the next month, such as if you move from Jan 31 to Feb, 
then the date is moved back to the last day of the next month.

The only argument, which defaults to 1, allows you to move backward or forward any number 
of months. For example, the following command moves the date forward two months:

  $mydate->nextmonth(2);

This command moves the date backward three months:

  $mydate->nextmonth(-3);

C<nextmonth()> handles year boundaries without problem.  Calling C<nextmonth()> for a date in 
December moves the date to January of the next year.

=head1 Properties

=head2 Basic properties

All of these properties are both readable and writable.  Where there might be some confusion 
about what happens if you assign to the property more detail is given.

=over

=item hour

Hour in 24 hour clock, 00 to 23.  Two digits, with a leading zero where necessary.

=item ampmhour

Hour in twelve hour clock, 0 to 12.  Two digits, with a leading zero where necessary.

=item ampm

I<am> or I<pm> as appropriate.  Returns lowercase.  If you set this property the object will adjust to the
same day and same hour but in I<am> or I<pm> as you set.

=item ampmuc, ampmlc

ampmuc returns I<AM> or I<PM> uppercased.  ampmlc returns I<am> or I<pm> lowercased.  


=item min, minute

Minute, 00 to 59.  Two digits, with a leading zero where necessary.

=item sec, second

Second, 00 to 59.  Two digits, with a leading zero where necessary.

=item weekdaynum

Number of the weekday.  This number is zero-based, so Sunday is 0, Monday is 1, etc. 
If you assign to this property the object will reset the date to the assigned
weekday of the same week.  So, for example, if the object represents 
Saturday Apr 14, 2001, and you assign 1 (Monday) to I<weekdaynum>:

 $mydate->{'weekdaynum'} = 1;

Then the object will adjust to Monday Apr 9, 2001.

=item weekdayshort

First three letters of the weekday.  I<Sun>, I<Mon>, I<Tue>, etc.  If you assign 
to this property the object will adjust to that day in the same week.  When you assign 
to this property EzDate actually only pays attention to the first three letters and 
ignores case, so I<SUNDAY> would a valid assignment.

=item weekdaylong

Full name of the weekday.  If you assign to this property the object will adjust to the 
day in the same week.  When you assign to this property EzDate actually only pays attention 
to the first three letters and ignores case, so I<SUN> would a valid assignment.

=item dayofmonth

The day of the month.  If you assign to this property the object adjusts to the day in the 
same month.

=item monthnum

Zero-based number of the month.  January is 0, February is 1, etc. If you assign to this 
property the object will adjust to the same month-day in the assigned month.  If the 
current day is greater than allowed in the assigned month then the day will adjust to 
the maximum day of the assigned month.  So, for example, if the object is set to 
31 Dec 2001 and you assign the month to February (1):

  $mydate->{'monthnum'} = 1;

Then I<dayofmonth> will be set to 28.

=item monthnumbase1

1 based number of the month for those of us who are used to thinking of January as 1, 
February as 2, etc.  Can be assigned to.

=item monthshort

First three letters of the month.  Can be assigned to.  Case insensitive in the assignment, 
so 'JANUARY' would be a valid assignment.

=item monthlong

Full name of the month.  Can be assigned to.  In the assignment, EzDate only 
pays attention to the first three letters and ignores case.

=item year

Year of the date the object represents.

=item yeartwodigits

The last two digits of the year.  If you assign to this property, EzDate assumes you mean to 
use the same first two digits.  Therefore, if the current date of the object is 1994 and you assign 
'12' then the year will be 1912... quite possibly not what you intended.  

=item yearday

Number of days into the year of the date.

=item full

A full string representation of the date, e.g. C<04:48:01 pm, Tue Apr 10, 2001>.  
You can assign just about any common date and/or time format to this property.

I<Please take the previous statement as a challenge.>  I've aggressively tried to find
formats that EzDate can't understand.  When I've found one, I've modified the code to 
accomodate it.  If you have some reasonably unambiguous date format that EzDate is unable 
to parse correctly, please send it to me.  I<-Miko>

=item dmy

The day, month and year representation of the date, e.g. C<03JUN2004>.

=item clocktime

The time formatted as HH::MM::SS XM.

=item miltime

The time formatted as HHMM on a 24 hour clock.  For example, 2:20 PM is 1420.

=item minofday

How many minutes since midnight.  Useful for doing math with times in a day.

=item printabledate

A string representing the date.  e.g.  Aug 3, 2001

=item printabletime

A string representing the time.  e.g.  2:05 AM


=back


=head2 Epoch properties

The following properties allow you to do date calculations at different granularities. All of these properties are 
both readable and writable.

=over

=item epochsecond

The basic Perl epoch integer.

=item epochhour

How many hours since the epoch.  

=item epochminute

How many minutes since the epoch.  

=item epochday

How many days since the epoch.

=back


=head2 Read-only properties

The following properties are read-only and will crash if you try to assign to them.


=over

=item dst

If the date is in daylight savings time.

=item leapyear

True if the year is a leap year.

=item daysinmonth

How many days in the month.

=item format string

To make the Unix types happy you can get your date formatted however you like using standard date codes.  The format string 
must contain at least one % or EzDate won't know it's a format string. For example, you could output a date like this:

	print $mydate->{'%h %d, %Y %k:%M %p'}, "\n";

which would give you something like this:

	Oct 31, 2001 02:43 pm

Following is a list of codes.  C<*> indicates that the code acts differently than 
standard Unix codes.  C<x> indicates that the code does not exists in standard Unix 
codes.

  %a   weekday, short                               Mon
  %A   weekday, long                                Monday
  %b * hour, 12 hour format, no leading zero        2
  %B * hour, 24 hour format, no leading zero        2
  %c   full date                                    Mon Aug 10 14:40:38
  %d   numeric day of the month                     10
  %D   date as month/date/year                      08/10/98
  %e x numeric month, 1 to 12, no leading zero      8
  %f x numeric day of month, no leading zero        3
  %h   short month                                  Aug
  %H   hour 00 to 23                                14
  %j   day of the year, 001 to 366                  222
  %k   hour, 12 hour format                         14
  %m   numeric month, 01 to 12                      08
  %M   minutes                                      40
  %n   newline
  %P x AM/PM                                        PM
  %p * am/pm                                        pm
  %r   hour:minute:second AM/PM                     02:40:38 PM
  %s   number of seconds since start of 1970        902774438
  %S   seconds                                      38
  %t   tab
  %T   hour:minute:second (24 hour format)          14:40:38
  %w   numeric day of the week, 0 to 6, Sun is 0    1
  %y   last two digits of the year                  98
  %Y   four digit year                              1998
  %%   percent sign                                 %

=back


=head1 Limitations / Known and suspected bugs

The routine for setting the year has an off-by-one problem which is kludgly fixed but 
which I haven't been able to properly solve.

The formatted string feature is quite new and needs more rigorous testing.  I suspect that if someone really 
tried to break it with weird format strings it could be broken.

EzDate is entirely based on the C<localtime()> and C<timelocal()> functions, so it inherits their limitations.
EzDate is probably not a good choice for handling dates before 1970. 

=head1 TODO

The following list itemizes features I'd like to add to EzDate.  Mostly I haven't added them because I can't 
figure out how to.  (Remember, I created EzDate because I stink at date calculating, not because I'm good at 
it.)  If you want these features in EzDate you're more likely to get them by adding them yourself and sending 
me the new-and-improved code (viva open source!) than you are by emailing me and asking me to add them.

=over

=item Time zone properties

The current version does not address time zone issues.  Frankly, I haven't been able to 
figure out how best to deal with them. I'd like a system where the object knows what time 
zone it's in and if it's daylight savings time.  Changing to another time zone changes 
the other properties such that the object is in the same moment in time in the new time 
zone and it was in the old time zone.  For example, if the object represents 5pm in the 
Eastern Time Zone (e.g. where New York City is) and its time zone is changed to Pacific 
Time (e.g. where Los Angeles is) then the object would have a time of 2pm.

The problem is in dealing with daylight savings times.  DST really confuses me and I don't 
understand how to program for it. 

=item Assignment based on format

Right now the formatted string feature is read-only.  It might be useful if the date could be assigned 
based on a format.  So, for example, you could set the date as Nov 1, 2001 like this:

  $mydate->{'%h %d %Y'} = 'Nov 1 2001';

This would come in handy when dealing with weirdly formatted dates.  However, EzDate is already quite robust 
about handling weirdly formatted dates, so this feature is not as pressingly needed as it might seem.

=item Misc other features

The venerable C<Date::Manip> has many cool features I'd like to add.  (Competitive? B<ME?>)  For example, 
I'd like the option of moving the date forward (or backward) to the next (previous) day of a week.  

=back

=head1 TERMS AND CONDITIONS

Copyright (c) 2001 by Miko O'Sullivan.  All rights reserved.  This program is 
free software; you can redistribute it and/or modify it under the same terms 
as Perl itself. This software comes with B<NO WARRANTY> of any kind.

=head1 AUTHOR

Miko O'Sullivan
F<miko@idocs.com>


=head1 VERSION

 Version 0.90    November 1, 2001
 Version 0.91    December 10, 2001
 Version 0.92    January  15, 2002
 Version 0.93    February 11, 2002

=cut

