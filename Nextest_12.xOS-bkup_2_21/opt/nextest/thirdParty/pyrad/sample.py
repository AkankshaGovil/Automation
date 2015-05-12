#!/usr/bin/python
import packet
from client import Client
from dictionary import Dictionary

srv=Client(server="localhost",
dict=Dictionary("/home/test/rajesh/pyrad-0.8/example/dictionary", "/home/test/rajesh/pyrad-0.8/example/dictionary"))

req=srv.CreateAuthPacket(code=packet.AccessRequest,
User_Name="rajesh", NAS_Identifier="localhost")
req["User-Password"]=req.PwCrypt("rajesh123")

reply=srv.SendPacket(req)
if reply.code==packet.AccessAccept:
	print "access accepted"
else:
	print "access denied"

print "Attributes returned by server:"
for i in reply.keys():
      print "%s: %s" % (i, reply[i])

