from smtplib import *
import string
import logging
#import pdb



"classes"

class MailerError(Exception):
    """
    General mailer error
    """

class Mailer(object):

    """
    This will mail to the respective manager when an instance of this is called.
    """

#Functions

    def __init__(self,toaddr=None,fromaddr=None,mailserver=None) :
         """
         Mailer instance is created fro initialization.

          The arguments are:

               - from_addr    : The address sending this mail.
               - to_addrs     : A list of addresses to send this mail to.  A bare
                                string will be treated as a list with 1 address.
               - mailserver   : mailserver through which mails will be sent.

         """
         self.log = logging.getLogger('nextestlog')
         self.toaddrs = toaddr
         self.fromaddr = fromaddr
         self.mailserver = mailserver

    def sendMail(self,subject,msg) :
        """
        The main function that is used for sending the mail.

        The arguments are:

               - subject     : Subject of this mail.
               - msg         : text body of the mail to be sent
        """
        user = "nexassist"
        password = "sitverify"
        self.subject = subject
        try :
            s_toaddrs = string.join(self.toaddrs,",")
        except :
            self.log.error("Error : The to-address %s is errorneous" %self.toaddrs)
            raise MailerError("Exception : The toaddress is errorneous")
        self.msg = ("From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n"\
                    % (self.fromaddr,s_toaddrs,self.subject))
        self.msg += msg

        try :
            server = SMTP()
        except :
            self.log.error("The Error in the connection to %s mailserver"% self.mailserver)
            raise MailerError("Exception : Unable to connect to mailserver")
        server.connect(self.mailserver)
        server.login(user,password)
        try :
            server.sendmail(self.fromaddr, self.toaddrs, self.msg)
        except SMTPHeloError, e:
            self.log.error("Exception : Error in response from server as exception %s is thrown" % str(e))
            raise MailerError("Exception : Error in iresponse from server because of exception %s" %str(e))
        except SMTPSenderRefused, e:
            self.log.error("Exception : Error in fromaddr as exception %s is thrown" %str(e))
            raise MailerError("Exception : Error in fromaddress because of exception %s" %str(e))
        except SMTPRecipientsRefused, e:
            self.log.error("Exception : Error in toaddress as exception %s is thrown" % str(e))
            raise MailerError("Exception : Error in toaddress because of exceptio %s" %str(e))
        except SMTPDataError, e:
            self.log.error("Exception : Error in mailserver as exception %s is thrown" % str(e))
            raise MailerError("Exception : Error in mailserver name because of exception %s" %str(e))
        except :
            self.log.error("Exception : The value assigned to to-addressess %s and from-address %s is errorneous" %(self.toaddrs,self.fromaddr))
            raise MailerError("Exception :The value assigned to to-addressess %s and from-address %s is errorneous" %(self.toaddrs,self.fromaddr))


        server.quit()


##def main() :

##    toaddr = "vijender.singh@induslogic.com"
#    fromaddr = "vijender.singh@globallogic.com"
#    mailserver = "192.168.15.37"
#    toaddrs = string.split(toaddr,',')
#    subject = "hi\n";
#    msg = "\r\n how are you\r\n"
#    mailObj = Mailer(toaddrs,fromaddr,mailserver)
#    mailObj.sendMail(subject,msg)
#    print "\ndone"

#if __name__ == "__main__" :
##    main()

