use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'    => 'Mail::Sendmail',
    'VERSION_FROM' => 'Sendmail.pm', # finds $VERSION 
    'dist' => { COMPRESS=>"gzip", SUFFIX=>"gz" },
    ($] ge '5.005')
        ? (
            'AUTHOR' => 'Milivoj Ivkovic <mi@alma.ch>',
            'ABSTRACT' => 'Simple platform independent mailer',
           )
         : (),
    
);

print <<EOT

**********************************************************************

NOTE
----

The test (test.pl) is a real test for this module: it sends an e-mail.

In older versions, the user was prompted for his address, and the
mail sent to him. This was incompatible with installs done through
CPAN.pm, so now the mail is sent to ME. 

You may want to edit test.pl before running it (directly or through
make test), so the mail is sent to you. If you don't mind that I
also get a copy, just add your address to mine. I have fun receiving
such mails, telling me the module works.

Read the docs, and have fun...

**********************************************************************

EOT
;