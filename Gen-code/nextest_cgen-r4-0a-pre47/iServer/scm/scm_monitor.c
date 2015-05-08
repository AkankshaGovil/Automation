#include "scmrpc_api.h"
#include "gis.h"
#include "lsprocess.h"
#include "scm.h"


// local to this file
typedef struct
{
	ulong 	peerAddr;			// Comes from configuration
	ulong peerId;
	long 	 myId;
	int 	peerChanged;

	time_t	heartbeat;
	int 	missedHeartbeats;
	
	Lock	scmLock;
} SCMPeerState_t;

// One peer only
SCMPeerState_t scmPeer, *peer;
long myId;

// Called to initialize the monitor functions
// returns 0 if successful
// This function is not MT-Safe
int
SCM_Init()
{
	char fn[] = "SCM_Init():";

	if (ispd_ctl.peer_count <= 0)
	{

		// There is no peer defined yet and no need to initialize
		return -1;
	}
	
	myId = lrand48();

	memset(&scmPeer, 0, sizeof(scmPeer));
	peer = &scmPeer;

	LockInit(&peer->scmLock, 0);

	if (SCM_PeerReset() != SCM_Ok)
	{
		return -1;
	}

	// Initialize our own detection mechanism
	scm_initcb_heartbeat(SCM_HeartbeatRx);

	// Launch server thread
	scm_create_server();

	return 0;
}

int
SCM_PeerReset()
{
	char fn[] = "SCM_PeerReset():";
	unsigned int scmPeerAddr;

	memset(peer, 0, sizeof(scmPeer));

	if (ispd_ctl.peer_count > 0)
	{
		if (inet_pton(AF_INET, ispd_ctl.peer_iservers[0], &scmPeerAddr) != 1)
		{
			NETERROR(MSCM, ("%s invalid address for peer %s\n", 
				fn, ispd_ctl.peer_iservers[ispd_ctl.peer_count]));

			return SCM_Error;
		}

		// Always host format in storage
		peer->peerAddr = ntohl(scmPeerAddr);
	}

	// Our Id will not change
	peer->myId = myId;

	// We have the peer configured and its ip address
	// We still don't know whether we are primary or backup

	return SCM_Ok;
}

int
SCM_PeerConnect()
{
	char fn[] = "SCM_PeerConnect():";
	if (scm_create_client(ispd_ctl.peer_iservers[0]) == -1)
	{
		NETDEBUG(MSCM, NETLOG_DEBUG4,
			("%s %s create_client failed\n", fn, SCM_Mode()));
		return SCM_ErrorNoPeer;
	}

	return SCM_Ok;
}

// callback for receiving heartbeat
void
SCM_HeartbeatRx(ulong nbrid, ulong peerip)
{
	char fn[] = "SCM_HeartbeatRx():";
	char ipstring[24];

	if (SCM_SetBackup(nbrid, peerip) == 0)
	{
		time(&peer->heartbeat);
	}
	else
	{
		NETERROR(MSCM, ("%s %s Heartbeat Received from Invalid m/c %s nbrid = %lu\n",
			fn, SCM_Mode(), FormatIpAddress(peerip, ipstring), nbrid));
	}

	return;
}

// send the heartbeat
// returns 0 if success
int
SCM_HeartbeatTx()
{
	// Call RPC function to send heartbeat
	// There is no config except interface name to get our ip from
	// set that to 0 for now
	return scm_heartbeat(peer->myId, 0);
}

// returns 1 if Backup exists, 0 otherwise
int
SCM_BackupExists()
{
	return (peer->peerId);
}

int
SCM_SetBackup(ulong nbrid, ulong peerip)
{
	char fn[] = "SCM_SetBackup():";

	if (nbrid != peer->peerId)
	{
		// backup has changed
		NETDEBUG(MSCM, NETLOG_DEBUG2,
			("%s %s New backup nbr %lu --> %lu\n", fn, SCM_Mode(), peer->peerId, nbrid));

		peer->peerId = nbrid;
		peer->peerChanged = 1;
		peer->peerAddr = peerip;

		SCM_Signal();
	}

	// There is no case in which we return an error yet
	// We assume that the SCM application state can cycle w/o the ispd
	// letting us know. There will be a config check at some point
	// when we check the back for its address we have in the config and 
	// at that point we will have an error scenario in this function

	return 0;
}

// Check to see if the backup changed and 
// return the new backup address
// return 1 if the backup changed since the last time we were called
int
SCM_CheckBackup(unsigned long *addr)
{
	// Get Locks
	*addr = peer->peerAddr;

	if (peer->peerChanged)
	{
		peer->peerChanged = 0;

		// release locks
		return 1;
	}

	// release locks
	return 0;
}

// State Change actions
void
SCM_NowBackup()
{
	// Reset the SCM queue and empty it
	SCM_QueueReset();

	// Reset Peer Id, we want to discover peer
	SCM_PeerReset();
}

void
SCM_NowPrimary()
{
	// Reset peer id, we want the peer to discover us again
	//
	SCM_QueueReset();

	SCM_PeerReset();
}
