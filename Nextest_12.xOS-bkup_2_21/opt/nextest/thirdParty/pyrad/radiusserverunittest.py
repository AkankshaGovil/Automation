#!/usr/bin/python

import pyrad.packet
import client
import dictionary
import radserver
import socket
import unittest

class radiusServerTest(unittest.TestCase):

    def testRadiusServerStart(self):        
        ipaddress = socket.gethostbyname(socket.gethostname())
        ipaddress_port = (ipaddress,6000)
        try:
            ## Start the Radius Server
            RadiusSvr = radserver.Radserver('10.0.28.131')
            RadiusSvr.AddUser(ipaddress, 'passwd')
            RadiusSvr.Start()
        except:
            self.assert_(False,"Failed to Start RADIUS server")
                
        ## Test whether the RADIUS server responds by sending an authentication request
        srv=client.Client(server="radiusprimary", secret="passwd", dict=dictionary.Dictionary(\
        "/usr/lib/python2.3/site-packages/pyrad/dictionary"))
        try:
            srv.bind(ipaddress_port)
    
            req=srv.CreateAuthPacket(code=pyrad.packet.AccessRequest,
            User_Name="test", NAS_Identifier=socket.gethostbyname(socket.gethostname()))
            req["User-Password"]=req.PwCrypt("passwd")
    
            reply=srv.SendPacket(req)
            
        except:
            self.assert_(False,"Failed to create and send Authentication request packet")
        print("Authentication Request packet sent")
        if reply.code==pyrad.packet.AccessAccept:
            print("RADIUS server is running and has Accepted the AccessRequest packet")
        else:
            self.assert_(False, "RADIUS server failed to Accept the request") 
            
        try:
            RadiusSvr.Stop()
        except:
            self.assert_(False,"Failed to STOP the RADIUS server")
        
        try:
            reply=srv.SendPacket(req)
        except:
            self.assert_(True,"RADIUS server Started/Stopped successfully")
        else:
            self.assert_(False,"Failed to Stop RADIUS server")

unittest.main()
    
