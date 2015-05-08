#!/usr/bin/perl

##
## Admininstall
##

## Hello, I am..
$Self = "admininstall";
$SelfVersion = "v0.2, 11/05/01";

## This is what I do...
$Product = "iserver";
$RootDir = "/usr/local/nextone";
$RootDirEtc = "$RootDir/etc";
$RootDirBin = "$RootDir/bin";
$RootDirLib = "$RootDir/lib";
$AdmMediaFile = "iserveradm-install.tar";
$Unpack = "tar xf ";
$TmpDir = "/tmp/.adminst";
$CFG_TMPDIR = "/tmp/$$";
$CFG_FILE = "$RootDirEtc/.admcfg";
$ADMINDEX = ".admindex";

$upgrade = 0;


use Getopt::Std;



sub GetResponse ()
{
	print "Hit <CR> to continue...\n";
	$resp = <>;
}

sub GetSpecialResponse ()
{
	print "Hit <CR> to continue. (OR) <Ctrl-C> to abort..\n";
	$resp = <>;
}


sub PrintHelpMessage ()
{
	print "$Self version $SelfVersion\n";
	print "$Self -v -h \n";
	print "$Self -i  to install \n";
	print "$Self -d  to uninstall \n";
	print "$Self -u  to upgrade\n";
	
	exit 0;
}


##
## Privilege check.
##
sub CheckForSuperuser ()
{
	###########################
	## Must be root
	###########################
	if ($ENV{'LOGNAME'} ne "root" )
	{
		print "\nYou must be super-user to run this script.\n";
		print "Exiting....\n ";
		exit 2;
	}
}



##
## Install
##
sub InstallPackage ()
{

	# Must be privileged.
	CheckForSuperuser ();

	$StartDir = `pwd`;
	chop $StartDir;

	if ( -d $RootDir )
	{
		chdir $RootDir or die "Unable to cd to $RootDir: $! \n";

		## Read .aloidindex
		$aloidindex = `cat .aloidindex`;
		chop $aloidindex;

		$iServerCoreVersion = $aloidindex;

		print "Detected existing iServer Core Package $iServerCoreVersion \n";

		if ( -d $RootDirEtc )
		{
			$admindex = `cat etc/$ADMINDEX`;
			chop $admindex;

			print "Detected existing version $admindex of iServer Administration Package \n";
			print "Perhaps you need to upgrade...\n";
		}


		## Get name of mediafile
		print "Proceeding to install iServer Admin Package. \n";
		print "Enter mediafile [$AdmMediaFile] :";

		# Read..
		$resp = <>;
		chop $resp;
		if ($resp ne "")
		{
			$AdmMediaFile = $resp;
		}
		

		## Go back to where we started from..and more
		chdir "$StartDir/..";

		if ( ! -f $AdmMediaFile)
		{
			die "Unable to find $AdmMediaFile \n";	
		}

		## Copy to temporary space.
		if ( ! -d $TmpDir)
		{
			mkdir ($TmpDir, 0777);
		}
		$status = system ("/bin/cp -p $AdmMediaFile $TmpDir");

		if ($status)
		{
			print "/bin/cp : $status \n";
		}

		chdir $TmpDir;

		## Extract the archive.
		$status = system ("$Unpack $AdmMediaFile");
		if ($status)
		{
			print "/usr/bin/tar : $status \n";
		}

		$AdmVersion = `cat $ADMINDEX`;
		chop $AdmVersion;
		print "\nInstalling Admin Package $AdmVersion \n";


		##
		## Go through the steps.
		##

		## Create the 'etc' directory.
		if ( ! -d $RootDirEtc ) 
		{
			mkdir ($RootDirEtc, 0755);
		}

		if ( ! -d $RootDirLib )
		{
			mkdir ($RootDirLib, 0755);
		}

		## Copy the stuff over to the admin directory 
		my @files;

		$ProdVersion = "$Product-$AdmVersion";

		chdir "$TmpDir/$ProdVersion" or die "Unable to cd to $ProdVersion: $!\n";

		@files = grep(!/^.{1,2}$/, glob("{*,.*}"));

		$status = system ("/bin/cp -p @files $RootDirEtc");

		if ($status)
		{
			print "Error in copying files to $RootDirEtc \n";
		}
		## clean up some -- remove lib files.
		$status = system ("/bin/rm -f $RootDirEtc/lib*");

		if ($status)
		{
			print "Error in partial cleanup...\n";
		}


		@files = glob ("lib*");

		$status = system ("/bin/cp -p @files $RootDirLib");

		if ($status)
		{
			print "Error in copying files to $RootDirLib \n";
		}
		else
		{
			my $prevdir = `pwd`;
			chop $prevdir;
			chdir $RootDirLib;
			my $f;

			foreach $f (@files)
			{
				$status = system ("$Unpack $f");
			}

			chdir $prevdir;
		}
		
		## Copy other files to $RootDir.
		chdir "..";
		@files = glob (".*index");

		$status = system ("/bin/cp -p $ADMINDEX $RootDir");

		if ($status)
		{
			print "Error in copying files over \n";
		}

		##
		## Edit the 'root' user's ksh .profile or .bashrc
		##
		my $RootUserKshProfile = "/.profile";
		my $RootUserBashrc     = "/.bashrc";
		if ( -f $RootUserKshProfile )
		{
			## Check if we've already appended the info.
			my $result = `grep "nExToNe" $RootUserKshProfile`;
			chop $result;

			if ( ! $result)
			{
				open (P, ">> $RootUserKshProfile") or die "Cannot open $RootUserKshProfile: $!";
				print P <<ePROF
###
### ======Added by nExToNe CoMmUnIcAtIoNs=========
. /usr/local/nextone/etc/profile
ePROF
;
				close (P);
			}
		}

		if ( -f $RootUserBashrc )
		{
			## Check if we've already appended the info.
			my $result = `grep "nExToNe" $RootUserBashrc`;
			chop $result;

			if ( ! $result)
			{
				open (P, ">> $RootUserBashrc") or die "Cannot open $RootUserBashrc: $!";
				print P <<ePROF
###
### ======Added by nExToNe CoMmUnIcAtIoNs=========
source /usr/local/nextone/etc/profile
ePROF
;
				close (P);
			}
		}
		##
		## ACTIONs come here.
		##
		my @actfiles;

		chdir "$TmpDir/$ProdVersion" or die "Unable to cd to $ProdVersion: $!\n";
		@actfiles = glob ("*.act");

		foreach $f (@actfiles)
		{
			if ( $f =~ m/(\w+)\.act$/ ) 
			{
				$pname = $1;
				print "Installing module $pname ...\n";
				$status = system ("/bin/sh ./$f");
			}
			else
			{
				print "Unknown package $f \n";
			}
		}

		##
		## Go out and post-process the files appropriately.
		##
		chdir $StartDir;
		$result = `perlcheck.pl`;


		##
		## Clean up.
		##	
		chdir "/tmp";
		$status = system ("/bin/rm -rf $TmpDir");

		if ($upgrade) {
			print "Restoring configuration files... \n";
			if ( -d $CFG_TMPDIR) {
				chdir "$CFG_TMPDIR";
				@f = grep(!/^.{1,2}$/, glob("{*,.*}"));
				for $file (@f) {
					system ("/bin/mv $file $RootDirEtc");
				}
				chdir "/tmp";
				system("/bin/rm -rf $CFG_TMPDIR");
			}
			print "Successfully upgraded Admin package!! \n";
		}
		else {
			print "Successfully installed Admin package!! \n";
		}
	}
	else
	{
		die "You need to install iServer Core Package first.\n";
	}

}


##
## Uninstall's package.
##
sub UninstallPackage ()
{

	# Must be privileged.
	CheckForSuperuser ();

	$StartDir = `pwd`;
	chop $StartDir;

	if ( -d $RootDir )
	{
		chdir $RootDir or die "Unable to cd to $RootDir: $! \n";
	## Read .aloidindex
		$aloidindex = `cat .aloidindex`;
		chop $aloidindex;

		$iServerCoreVersion = $aloidindex;

		print "Detected existing iServer Core Package $iServerCoreVersion \n";

		if ( -d $RootDirEtc )
		{
			$admindex = `cat $ADMINDEX`;
			chop $admindex;

			print "Detected installed version $admindex of iServer Admin Package \n";
		}
		else
		{
			print "The iServer Admin package seems to have been removed already. \n";
		}

		if ($upgrade) {
			mkdir ($CFG_TMPDIR, 0777);		
			print "Saving configuration files... \n";

			if ( -f $CFG_FILE) {
				open (CFG, "< $CFG_FILE") or die "Can't open file $CFG_FILE: $!\n";
				while ($f = <CFG>) {
					chomp($f);
					if ( -f "$RootDirEtc/$f" ) {
						system ("/bin/cp -p $RootDirEtc/$f $CFG_TMPDIR");
					}
				}
				close(CFG);
			}
			else {
				# Take best shot at all possible configuration files
				@f = ("cdrtrim.cfg", "dbback.cfg", ".dbbackcdrinfo");
				for $f (@f) {
					if ( -f "$RootDirEtc/$f" ) {
						system ("/bin/cp -p $RootDirEtc/$f $CFG_TMPDIR");
					}
				}
			}
		}

		print "Proceeding to remove iServer Admin Package. \n";
		

		## Change to the root directory
		chdir $RootDir;

		$status = system ("/bin/rm -rf $RootDirEtc");
		if ($status)
		{
			print "Error removing files in $RootDirEtc \n";
		}

		$status = system ("/bin/rm -rf $RootDirLib");
		if ($status)
		{
			print "Error removing files in $RootDirLib \n";
		}

		## Remove our index file also.
		$status = system ("/bin/rm -f $ADMINDEX");

		## Come back to our starting point.
		chdir $StartDir;
		print "Uninstall successful.\n";

	}
	else
	{
		die "You need to install iServer Core Package first.\n";
	}

}



##
## Main
##

if (! @ARGV)
{
	PrintHelpMessage ();
}

getopts ("diuhv");

if ($opt_d)
{
	print "Uninstalling iServer Admin package. \n";

	GetResponse ();

	UninstallPackage ();
}

if ($opt_i)
{
	print "\nInstalling iServer Admin package. \n";
	GetResponse ();

	InstallPackage ();

}

if ($opt_u)
{
	print "Upgrading iServer Admin package. \n";
	GetResponse ();

	$upgrade = 1;

	UninstallPackage ();

	InstallPackage ();
}

if ($opt_h)
{
	PrintHelpMessage ();
}

if ($opt_v)
{
	print "$Self version $SelfVersion\n";
}


exit 0;

