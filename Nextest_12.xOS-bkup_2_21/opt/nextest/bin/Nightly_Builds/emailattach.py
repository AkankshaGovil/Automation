"""
Sending an attachment with the mail, used especially in the case
of sending regression results to the Nextone SIT Team.
"""
import sys, smtplib, MimeWriter, base64, StringIO, string, logging
import pdb
# Classes

class EmailAttachError(Exception):
    """
    General mailer error
    """


class EmailAttach(object):
    """
    This Class is used for attaching  a results of regression file with the
    mail and sending the mail to the concerned SIT people.

    """

# Functions
    def __init__(self,path,to_addr,from_addr,mail_server,login,passwd):

        """
        This is the initialization function of EmailAttach it picks the values
        from userConfig.cfg using the context variable it also initialises the
        path of the file which is to be attached
        """
        self.toaddrs = to_addr
        #self.toaddrs = self.toaddr.split(',')
        self.fromaddr = from_addr
        self.mailserver = mail_server
        self.login = login
        self.passwd = passwd
        
        self.path = path

    def emailAttachment(self,subject):
        """
        It uses the MIMEWriter,StringIO and base64 module for creating the subject of the mail,
        also defining the body of the mail and attaching the file and encoding the mail with
        base64 encrytion technique for security reasons
        """
        #pdb.set_trace()
        message = StringIO.StringIO()
        writer = MimeWriter.MimeWriter(message)
        writer.addheader('Subject', subject)
        writer.startmultipartbody('mixed')

        # start off with a text/plain part
        try :
            part = writer.nextpart()
            body = part.startbody('text/plain')
            body.write('\n\rThe Cumulative Result is stored at the path %s\n\rRegards, \n\rNextone-SIT' %self.path)
        except :
            raise EmailAttachError("Exception :Error in the subject part of the mail ")

        # now add an Attachment
        try :
            part = writer.nextpart()
            part.addheader('Content-Transfer-Encoding', 'base64')
            part.addheader('Content-Disposition', 'attachment; filename %s' % self.path)
            body = part.startbody('application/Octet-Stream; name=%s' % self.path)
            base64.encode(open(self.path, 'r'), body)
        except :
            raise EmailAttachError("Exception :Error in the body part of the mail ")
          

        # finish off
        writer.lastpart()
        # send the mail
        try :
            s_toaddrs = string.join(self.toaddrs,",")
            message_getvalue = message.getvalue() + s_toaddrs
        except :
            raise EmailAttachError("Exception : The toaddress is errorneous")

        smtp = smtplib.SMTP("%s" %self.mailserver)
        try :
            smtp.login(self.login,self.passwd)
        except :
            raise EmailAttachError("Exception :The value assigned to login %s or passwd %s is errorneous" %(self.login,self.passwd))
            
        try :
            smtp.sendmail(self.fromaddr,self.toaddrs,message_getvalue)
        except :
            raise EmailAttachError("Exception :The value assigned to to-addressess %s and from-address %s is errorneous" %(s_toaddrs,self.fromaddr))

        smtp.quit()

