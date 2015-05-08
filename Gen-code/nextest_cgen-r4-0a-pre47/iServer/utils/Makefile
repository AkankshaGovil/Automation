
##
## Makefile
##
## Directory: $(BASE)/utils



install: .DUMMY
	@(tar cvf cdrpp.tar cdrpp cdrpp.act cdrpp.cron cdrpp.CHANGES cdrpp.pod mail.pl notify.pl Descriptive.pm Sendmail.pm EzDate.pm )

gencodemap: 
	gcc -Wall -I../include -o gencodemap gencodemap.c

clean:
	rm -f cdrpp.tar gencodemap

.DUMMY:

