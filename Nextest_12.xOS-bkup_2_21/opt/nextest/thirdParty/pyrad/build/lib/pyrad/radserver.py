#!/usr/bin/python

from pyrad import dictionary, packet, server, host
import pyrad.packet
import pexpect, sys, os
import socket, time
import logging

class RadserverException(Exception):
        """ This class used to raise error in starting the RADIUS server
                """

class Radserver(server.Server):
	#PR 133009 Adding path variable to store radius packets
        def __init__(self, addr,path):

                """
                This initialize the RADIUS server.
                   addr: IP address of the server
                   dict: instance of dictionary.Dictionary
                """
                self.addr = addr
                self.running=False
                self.prepaid = False
                self.log=logging.getLogger('nextestlog')
                # 32141
                self.acclog = None
                self.count = 0
                self.respTime = 0
	        #PR 133009 Adding path variable to store radius packets
		self.filename=path+'/rad-pkt.log'
                self.pkt_copy=packet.Packet()
                server.Server.__init__(self, dict=dictionary.Dictionary(\
"/usr/lib/python2.3/site-packages/pyrad/dictionary"))


        def Configure(self):
                """ This method can be over written.
                """
                pass


        def Start(self):
                """
                This method binds and run the server by forking.
                 The forked process id is stored for stopping the server.
                Before going to start, it is checked whether it is started already.
                """
                self.log=logging.getLogger('nextestlog')
                if self.running==True:
                        self.log.debug("RADIUS server %s is already running"%self.addr)
                else:
                        self.pid=os.fork()
                        if self.pid==0:
                                server.Server.BindToAddress(self, self.addr)
                                self.log.debug("RADIUS server %s started" %self.addr)
                                server.Server.Run(self)
                        self.running=True

        def HandleAuthPacket(self, pkt):
                """Authentication packet handler.

                This method is called when a valid
                authenticating packet has been received. It is overriden in
                derived server class to add custom behaviour.

                @param pkt: packet to process
                @type  pkt: Packet class instance
                """

                self.log.debug("Received an authentication request at %s RADIUS server from\
                       %s" %(socket.gethostbyaddr(self.addr)[0], socket.gethostbyaddr(pkt.source[0])[0]))
                
                for attr in pkt.keys():
                        self.log.debug("RADIUS server recieved: %s : %s" % (attr, pkt[attr]))

                # 32163 - Handle Cisco Prepaid service request
                if self.prepaid:
                    if pkt.keys().__contains__('NAS-IP-Address') and pkt.keys().__contains__('Called-Station-Id'):
                        # Generate Response for second phase authorization
                        if self.prepaidparam['c_time']: 
                            reply=self.CreateReplyPacket(pkt,
                                       h323_return_code=self.prepaidparam['code'],
                                       h323_preferred_lang=self.prepaidparam['lang'],
                                       h323_credit_time=self.prepaidparam['c_time'])
                        else:
                            reply=self.CreateReplyPacket(pkt,
                                       h323_return_code=self.prepaidparam['code'],
                                       h323_preferred_lang=self.prepaidparam['lang'])

                    elif pkt.keys().__contains__('NAS-IP-Address') and pkt.keys().__contains__('Calling-Station-Id'):
                        # Generate Response for first phase authorization
                        if self.prepaidparam['c_amt']:  
                            reply=self.CreateReplyPacket(pkt,
                                       h323_credit_amount=self.prepaidparam['c_amt'],
                                       h323_return_code=self.prepaidparam['code'],
                                       h323_preferred_lang=self.prepaidparam['lang'],
                                       h323_credit_time=self.prepaidparam['c_time'])
                        else:
                            reply=self.CreateReplyPacket(pkt,
                                       h323_return_code=self.prepaidparam['code'],
                                       h323_preferred_lang=self.prepaidparam['lang'],
                                       h323_credit_time=self.prepaidparam['c_time'])
 
                else:
                    #Create the Radius response packet
                    reply=self.CreateReplyPacket(pkt)

                #Send a Access Reject response if the user name is rejectme
                user = pkt['User-Name'][0]
                # 33528 - Handle cases where the request packet does not 
                # contain User-Password parameter
                pwd = 'test'
                if pkt.keys().__contains__('User-Password'):
                    pwd = pkt['User-Password'][0]
                if user == 'rejectme' or user == '':
                    self.log.debug("RADIUS Server Access Rejected!")
                    reply.code=packet.AccessReject
                elif pwd == '':
                    reply.code=packet.AccessReject
                else:
                    self.log.debug("RADIUS Server Access Accepted")
                    reply.code=packet.AccessAccept
                self.SendReplyPacket(pkt.fd, reply)


        def HandleAcctPacket(self, pkt):
                """Accouting packet handler.

                This method is called when a valid
                accounting packet has been received. It is overriden in
                derived server class to add custom behaviour.

                @param pkt: packet to process
                @type  pkt: Packet class instance
                """
                self.log.debug("Received an accounting request at %s RADIUS server from\
                         %s" %(socket.gethostbyaddr(self.addr)[0], socket.gethostbyaddr(pkt.source[0])[0]))

		#PR 133009 Code for writing radius accouting packets
                self.WriteRadiusPacket(pkt)
                for attr in pkt.keys():
                        self.log.debug("RADIUS server recieved: %s : %s" % (attr, pkt[attr]))
                        
                # 32141 - added code for radius accounting packets

                reply=self.CreateReplyPacket(pkt)

                if pkt.code==packet.AccountingRequest:
                    time.sleep(self.respTime)
                    reply.code=packet.AccountingResponse
                    self.count += 1
                    self.acclog = open('/tmp/rad-acc.log','w')
                    self.acclog.write(str(self.count) + '\n')
                    self.acclog.close()

                else:
                    reply.code=pyrad.packet.AccessAccept
                self.SendReplyPacket(pkt.fd,reply)

        def AddUser(self, user_address, secret, name=None):
                """ This method adds the user into RADIUS server user list
                """

                # 33503 - Add secret as a member variable so that it can be 
                # accessed from the testcase
                self.secret = secret
                self.hosts[user_address]=server.RemoteHost(user_address, secret, name)

        def Stop(self):
                """ This method stops the RADIUS server using forked process id
                        and closes the sockets. Whether RADIUS server is running is checked before
                        going to stop.
                """
                if self.running==True:
                        os.system("kill  -9 %s" % self.pid)
                        os.wait()
                        self.log.debug("RADIUS server %s Stopped" %self.addr)
                        self.running=False

                else:
                        self.log.debug("RADIUS server %s is not running"%self.addr)

        def SetCiscoPrepaidParameters(self,c_amt="",c_time="",lang="en",code="0"):
            """ Interface used to pass the values of the parameters that are to be 
            sent by the Server in the Radius Response packet for Cisco Prepaid Service 

            c_amt    - Credit Amount value
            c_time   - Credit Time value  
            lang     - Preferred Language
            code     - H323 return Code """

            self.prepaidparam = {}
            self.prepaid = True
            self.prepaidparam['c_amt']=c_amt
            self.prepaidparam['c_time']=c_time
            self.prepaidparam['lang']=lang
            self.prepaidparam['code']=code    
      
        #PR 133009 Added a functionto write radius packet
        def WriteRadiusPacket(self,pkt):
            """
            Write the attribute and packtes to a tmp file for comparision with the cdrs
            """
            self.radlog = open(self.filename,'a+')
            self.log.debug("Writing the radius packets to %s" %self.filename)
            for attr in pkt.keys():
                str1="%s : %s" % (attr, pkt[attr])
		# adding a space after : as the time also contains : and hence issues while processing
                self.radlog.write("%s: %s\n" %(attr, pkt[attr]))
	    self.radlog.write("\n\n")
	    self.radlog.close()
