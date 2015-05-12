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
        def __init__(self, addr):

                """
                This initialize the RADIUS server.
                   addr: IP address of the server
                   dict: instance of dictionary.Dictionary
                """
                self.addr = addr
                self.running=False
                self.log=logging.getLogger('nextestlog')

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
                if self.running==True:
                        self.log.debug("RADIUS server %s is already running"%self.addr)
                else:
                        server.Server.BindToAddress(self, self.addr)
                        self.log.debug("RADIUS server %s started" %self.addr)
                        self.pid=os.fork()
                        if self.pid==0:
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

                #Create the Radius response packet
                reply=self.CreateReplyPacket(pkt)

                #Send a Access Reject response if the user name is rejectme
                if pkt['User-Name'][0] == 'rejectme':
                    self.log.debug("RADIUS Server Access Rejected!")
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
                self.log.debug("Received an authentication request at %s RADIUS server from\
			%s" %(socket.gethostbyaddr(self.addr)[0], socket.gethostbyaddr(pkt.source[0])[0]))

		for attr in pkt.keys():
                        self.log.debug("RADIUS server recieved: %s : %s" % (attr, pkt[attr]))

		reply=self.CreateReplyPacket(pkt)
		reply.code=pyrad.packet.AccessAccept
                self.SendReplyPacket(pkt.fd,reply)

        def AddUser(self, user_address, secret, name=None):
                """ This method adds the user into RADIUS server user list
                """

                self.hosts[user_address]=server.RemoteHost(user_address, secret, name)
                self.log.debug("USer %s is added to RADIUS server %s" %(user_address, self.addr))

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
