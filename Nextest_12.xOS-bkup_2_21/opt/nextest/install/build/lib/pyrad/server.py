# server.py
#
# Copyright 2003-2004 Wichert Akkerman <wichert@wiggy.net>

"""Generic RADIUS server and proxy
"""

import select, socket
import host, packet

class RemoteHost:
	"""Remote RADIUS capable host we can talk to.
	"""

	def __init__(self, address, secret, name, authport=1812, acctport=1813):
		"""Constructor.

		@param   address: IP address
		@type    address: string
		@param    secret: RADIUS secret
		@type     secret: string
		@param      name: short name (used for logging only)
		@type       name: string
		@param  authport: port used for authentication packets
		@type   authport: integer
		@param  acctport: port used for accounting packets
		@type   acctport: integer
		"""
		self.address=address
		self.secret=secret
		self.authport=authport
		self.acctport=acctport
		self.name=name


class PacketError(Exception):
	"""Exception class for bogus packets
	
	PacketError exceptions are only used inside the Server class to 
	abort processing of a packet.
	"""

class Server(host.Host):
	"""Basic RADIUS server.

	This class implements the basics of a RADIUS server. It takes care
	of the details of receiving and decoding requests; processing of
	the requests should be done by overloading the appropriate methods
	in derived classes.

	@ivar  hosts: hosts who are allowed to talk to us
	@type  hosts: dictionary of Host class instances
	@ivar  _poll: poll object for network sockets
	@type  _poll: select.poll class instance
	@ivar _fdmap: map of filedescriptors to network sockets
	@type _fdmap: dictionary
	@cvar MaxPacketSize: maximum size of a RADIUS packet
	@type MaxPacketSize: integer
	"""

	MaxPacketSize	= 8192

	def __init__(self, addresses=[], authport=1812, acctport=1813, hosts={}, dict=None):
		"""Constructor.

		@param addresses: IP addresses to listen on
		@type  addresses: sequence of strings
		@param  authport: port to listen on for authentication packets
		@type   authport: integer
		@param  acctport: port to listen on for accounting packets
		@type   acctport: integer
		@param     hosts: hosts who we can talk to
		@type      hosts: dictionary mapping IP to RemoteHost class instances
		@param      dict: RADIUS dictionary to use
		@type       dict: Dictionary class instance
		"""
		host.Host.__init__(self, authport, acctport, dict)
		self.hosts=hosts

		self.authfds=[]
		self.acctfds=[]

		for addr in addresses:
			self.BindToAddress(addr)
	

	def BindToAddress(self, addr):
		"""Add an address to listen to.

		An empty string indicated you want to listen on all addresses.

		@param addr: IP address to listen on
		@type  addr: string
		"""
		authfd=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                authfd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		authfd.bind((addr, self.authport))

		acctfd=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                acctfd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		acctfd.bind((addr, self.acctport))

		self.authfds.append(authfd)
		self.acctfds.append(acctfd)
	

	def HandleAuthPacket(self, pkt):
		"""Authentication packet handler.

		This is an empty function that is called when a valid
		authentication packet has been received. It can be overriden in
		derived classes to add custom behaviour.

		@param pkt: packet to process
		@type  pkt: Packet class instance
		"""
		pass


	def HandleAcctPacket(self, pkt):
		"""Accounting packet handler.

		This is an empty function that is called when a valid
		accounting packet has been received. It can be overriden in
		derived classes to add custom behaviour.

		@param pkt: packet to process
		@type  pkt: Packet class instance
		"""
		pass


	def _HandleAuthPacket(self, pkt):
		"""Process a packet received on the authentication port

		If this packet should be dropped instead of processed a
		PacketError exception should be raised. The main loop will
		drop the packet and log the reason.

		@param pkt: packet to process
		@type  pkt: Packet class instance
		"""
		if not self.hosts.has_key(pkt.source[0]):
			raise PacketError, "Received packet from unknown host"

		pkt.secret=self.hosts[pkt.source[0]].secret

		if pkt.code!=packet.AccessRequest:
			raise PacketError, "Received non-authentication packet on authentication port"
		
		self.HandleAuthPacket(pkt)


	def _HandleAcctPacket(self, pkt):
		"""Process a packet received on the accounting port

		If this packet should be dropped instead of processed a
		PacketError exception should be raised. The main loop will
		drop the packet and log the reason.

		@param pkt: packet to process
		@type  pkt: Packet class instance
		"""
		if not self.hosts.has_key(pkt.source[0]):
			raise PacketError, "Received packet from unknown host"

		pkt.secret=self.hosts[pkt.source[0]].secret

		if not pkt.code in [ packet.AccountingRequest,
				packet.AccountingResponse ]:
			raise PacketError, "Received non-accounting packet on accounting port"

		self.HandleAcctPacket(pkt)
	

	def _GrabPacket(self, pktgen, fd):
		"""Read a packet from a network connection.

		This method assumes there is data waiting for to be read.

		@param fd: socket to read packet from
		@type  fd: socket class instance
		@return: RADIUS packet
		@rtype:  Packet class instance
		"""
		(data,source)=fd.recvfrom(self.MaxPacketSize)
		pkt=pktgen(data)
		pkt.source=source
		pkt.fd=fd

		return pkt


	def _PrepareSockets(self):
		"""Prepare all sockets to receive packets.
		"""
		for fd in self.authfds + self.acctfds:
			self._fdmap[fd.fileno()]=fd
			self._poll.register(fd.fileno(), select.POLLIN|select.POLLPRI|select.POLLERR)

		self._realauthfds=map(lambda x: x.fileno(), self.authfds)
		self._realacctfds=map(lambda x: x.fileno(), self.acctfds)
	

	def CreateReplyPacket(self, pkt, **attributes):
		"""Create a reply packet.

		Create a new packet which can be returned as a reply to a received
		packet.

		@param pkt:   original packet 
		@type pkt:    Packet instance
		"""

		reply=pkt.CreateReply(**attributes)
		reply.source=pkt.source

		return reply


	def _ProcessInput(self, fd):
		"""Process available data.

		If this packet should be dropped instead of processed a
		PacketError exception should be raised. The main loop will
		drop the packet and log the reason.

		This function calls either HandleAuthPacket() or
		HandleAcctPacket() depending on which socket is being
		processed.

		@param  fd: socket to read packet from
		@type   fd: socket class instance
		"""
		if fd.fileno() in self._realauthfds:
			pkt=self._GrabPacket(lambda data, s=self: s.CreateAuthPacket(packet=data), fd)
			self._HandleAuthPacket(pkt)
		else:
			pkt=self._GrabPacket(lambda data, s=self: s.CreateAcctPacket(packet=data), fd)
			self._HandleAcctPacket(pkt)



	def Run(self):
		"""Main loop.

		This method is the main loop for a RADIUS server. It waits
		for packets to arrive via the network and calls other methods
		to process them.
		"""
		self._poll=select.poll()
		self._fdmap={}
		self._PrepareSockets()

		while 1  :
			for (fd, event) in self._poll.poll():
				fdo=self._fdmap[fd]
				if event==select.POLLIN:
					try:
						fdo=self._fdmap[fd]
						self._ProcessInput(fdo)
					except PacketError, err:
						print "Dropping packet: " + str(err)
					except packet.PacketError, err:
						print "Received a broken packet: " + str(err)
				else:
					print "Unexpected event!"


class Proxy(Server):
	"""Base class for RADIUS proxies.

	This class extends tha RADIUS server class with the capability to
	handle communication with other RADIUS servers as well.

	@ivar _proxyfd: network socket used to communicate with other servers
	@type _proxyfd: socket class instance

	"""

	def _PrepareSockets(self):
		Server._PrepareSockets(self)

		self._proxyfd=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self._fdmap[proxyfd.fileno()]=proxyfd
		self._poll.register(self._proxyfd.fileno(), (select.POLLIN|select.POLLPRI|select.POLLERR))


	def _HandleProxyPacket(self, fd, pkt):
		"""Process a packet received on the reply socket.

		If this packet should be dropped instead of processed a
		PacketError exception should be raised. The main loop will
		drop the packet and log the reason.

		@param  fd: socket to read packet from
		@type   fd: socket class instance
		@param pkt: packet to process
		@type  pkt: Packet class instance
		"""
		if not self.hosts.has_key(pkt.source[0]):
			raise PacketError, "Received packet from unknown host"

		pkt.secret=self.hosts[pkt.source[0]].secret

		if not pkt.code in [ client.AccessAccept, client.AccessReject, client.AccountingResponse ]:
			raise PacketError, "Received non-response on proxy socket"



	def _ProcessInput(self, fd, pkt):
		"""Process available data.

		If this packet should be dropped instead of processed a
		PacketError exception should be raised. The main loop will
		drop the packet and log the reason.

		This function calls either HandleAuthPacket(),
		HandleAcctPacket() or _HandleProxyPacket() depending on which
		socket is being processed.

		@param  fd: socket to read packet from
		@type   fd: socket class instance
		@param pkt: packet to process
		@type  pkt: Packet class instance
		"""
		if fd.fileno()==self._proxyfd.fileno():
			self._HandleProxyPacket(fd, pkt)
		else:
			Server._ProcessInput(self, fd, fd)


