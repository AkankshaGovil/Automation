/*
 * Nike.c
 * NextTone Internet Key Exchange Stuff
 * Parts of code may be subject to Cisco's copyright
 * medhavi, created, 2/21/99.
 */


/* Right now, we just put in whatever we want, nothing more.
 * The thing will run on top of TCP, or UDP, between the netoid
 * and the aloid or between the netoid and the netoid
 */

/* The protocol right now is as follows:
 * we dont have to interoperate with any third party stuff, 
 * as we kind of optimize this for our own call setting up, and 
 * aloid lookups.
 * 
 * Netoid <-> Aloid
 * When the netoid sets up a TCP connection with the aloid for
 * registration, we first do authentication. All authentication
 * and encryption initiative is taken by the netoid. The aloid
 * provides verification information and uses verification
 * information supplied by the netoid.
 * 
 * Registration -
 * We use the aggressive mode, as speicifed in ISAKMP.
 * the only difference is that the identification payload
 * is encrypted with the pre-shared key. Also, to the aggressive
 * exchange we have added anothe message which comprises, the
 * Aloid returning a signed certificate back to the netoid.
 * The SA attempted is: Use DH, Use Pre-shared Key. Certificates
 * are exchanged as part of AUTH.
 * After this both have established a shared DH key, authenticated
 * themselves. Also, the netoid obtains a signed version of his DH
 * public key from the aloid. After the first two messages, the
 * SA is identified by a combination of the cookies supplied by
 * the netoid and aloid.
 * Now all the rest of the communication takes place encrypted by
 * the shared DH key. It is possible that using this shared DH key
 * the netoid and the aloid exchange another key.
 * 
 * Lookups -
 * If needed lookups can be encrypted by this shared key.
 *
 * Call Handling
 * Netoid to Netoid Calling (Aloid not involved)
 * Netoid (i) -> Netoid (r)
 * 
 */

#include "isakmp.h"

/* Send the ISAKMP packet */
static int send_request (SA *secass)
{
    int n;
    struct sockaddr *addr;

    /* 
     * these are the source and destination of the security association
     * we're negoatiating. Therefore they're reversed for the receiver--
     * he _sends_ to the "src".
     */

    if(secass->in_or_out == OUTBOUND)
	addr = (struct sockaddr *)&secass->dst;
    else
	addr = (struct sockaddr *)&secass->src;

    if (secass->socket > 0)
    {
	 /* Socket exists, we can use UDP / TCP */
	 
	 if((n = sendto(secass->socket, (char *)secass->packet, 
			secass->packet_len, 0, addr, 
			sizeof(struct sockaddr))) < 0)
	 {
	      LOG((ERR,"Failed to send %d bytes!", secass->packet_len));
	      perror("sendto");
	      return(n);
	 }
    }
    else
    {
	  if((n = sendto(is, (char *)secass->packet, 
			secass->packet_len, 0, addr, 
			sizeof(struct sockaddr))) < 0)
	 {
	      LOG((ERR,"Failed to send %d bytes!", secass->packet_len));
	      perror("sendto");
	      return(n);
	 }
    }

	/*
	 * if we expect a reply, set a timer for 8 seconds. This amount of
	 * time is a guess and should be modified once statistics are
	 * generated on packet loss and round trip + processing times across
	 * the Internet.
	 */
    if(secass->send_dir == BIDIR)
	secass->to_id = bb_add_timeout(context, 30000000, timed_out, 
					(caddr_t)secass);
    return(n); 
}


























