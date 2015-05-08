This file explains the layout of this folder.
The files contained here are the source files for the various test 
applications that are provided with this package. These are:-

i>  parsetime.c:  This contains the source for the binary parsetime which can
				be used to calculate the time that the SIP Stack takes for
				parsing a SIP Message(text buffer).	
				
ii> encodetime.c:This contains the source for the binary encodetime which can be
				used to calculate the time that the SIP Stack takes to encode
				a SIP Message in the form a text buffer from a decoded
				SipMessage.

iii>sendmessage.c: This contains the source for the test application sendmessage  			    which can be used to send a text buffer to any entity 	
                which is listening on the network.

iv>siptest.c:	This contains the source for the test application siptest.c

v> siptxntimer.c:When the stack is compiled with TXN_LAYER then siptest.c needs 
				a reference hash,timer implementation. The source for such a 
				reference implementation can be found in the file siptxntimer.c.
