/************************************************************
** FUNCTION:
**	This file implements the sdp  accessor APIs.
**
*************************************************************
**
** FILENAME:
	Sdp.c
**
** DESCRIPTION
**
**
**   DATE              NAME                      REFERENCE
** --------           ------                     -----------
** 23/11/99           B. Borthakur, K. Deepali  Added Origin APIs
**
** Copyright 1999, Hughes Software Systems, Ltd.
*************************************************************/

#include "sipfree.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sdp.h"
#include "sipinternal.h"
#include "sipclone.h"
#include "sipcommon.h"
#include "sipinit.h"
#include "sipfree.h"
#include "siplist.h"
#include "string.h"

/********************************************************************
** FUNCTION:sdp_getVersion
**
** DESCRIPTION: This functon retrieves the pVersion field from an SDP
**		message structure
**
**********************************************************************/

SipBool sdp_getVersion
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit **pVersion, SipError *err)
#else
	( msg, pVersion, err )
	  SdpMessage *msg;
	  SIP_S8bit **pVersion;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_version;
	SIPDEBUGFN("Entering getVersion\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (( msg == SIP_NULL)||(pVersion==SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	
#endif
	temp_version = msg->pVersion;

	if( temp_version == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_version);
	*pVersion = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pVersion == SIP_NULL )
		return SipFail;

	strcpy( *pVersion , temp_version );
#else
	*pVersion = temp_version;
#endif

	SIPDEBUGFN("Exiting getVersion\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sdp_setVersion
**
** DESCRIPTION: This function sets the pVersion field in an SDP message
**		structure
**
**********************************************************************/

SipBool sdp_setVersion
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *pVersion, SipError *err)
#else
	( msg, pVersion, err )
	  SdpMessage *msg;
	  SIP_S8bit *pVersion;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_version;
	SdpMessage *temp_mssg;
	SIPDEBUGFN("Entering setVersion\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pVersion == SIP_NULL)
		temp_version = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pVersion );
		temp_version = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_version== SIP_NULL )
			return SipFail;

		strcpy( temp_version, pVersion );
#else
		temp_version = pVersion;
#endif

	}

	temp_mssg = ( SdpMessage *)msg ;

	if ( temp_mssg->pVersion != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_mssg->pVersion, err) == SipFail)
			return SipFail;
	}
        temp_mssg->pVersion=temp_version;
	SIPDEBUGFN("Exiting setVersion\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/********************************************************************
** FUNCTION:sdp_getOrigin
**
** DESCRIPTION: This function retrieves the pOrigin filed from an SDP
**		message structure
**
**********************************************************************/

SipBool sdp_getOrigin
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SdpMessage *msg, SdpOrigin **pOrigin, SipError *err)
#else
	( SdpMessage *msg, SdpOrigin *pOrigin, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	( msg, pOrigin, err )
	  SdpMessage *msg;
	  SdpOrigin **pOrigin;
	  SipError *err;
#else
	( msg, pOrigin, err )
	  SdpMessage *msg;
	  SdpOrigin *pOrigin;
	  SipError *err;
#endif
#endif
{
	SIPDEBUGFN("Entering getOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if(pOrigin==SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(msg->pOrigin == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*pOrigin = msg->pOrigin;
	HSS_LOCKEDINCREF((*pOrigin)->dRefCount);
#else
	if(__sip_cloneSdpOrigin(pOrigin,msg->pOrigin,err)==SipFail)
	{
		return SipFail;
	}
#endif

	SIPDEBUGFN("Exiting getVersion\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*******************************************************************
** FUNCTION: sdp_setOrigin
**
** DESCRIPTION: This function sets the pOrigin filed in an SDP message
**		structure
**
*******************************************************************/
SipBool sdp_setOrigin
#ifdef ANSI_PROTO
	( SdpMessage *msg, SdpOrigin *pOrigin, SipError *err)
#else
	( msg, pOrigin, err )
	  SdpMessage *msg;
	  SdpOrigin *pOrigin;
	  SipError *err;
#endif
{
	SIPDEBUGFN("Entering setOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(msg->pOrigin!=SIP_NULL)
	{
		sip_freeSdpOrigin(msg->pOrigin);
	}
	if(pOrigin == SIP_NULL)
	{
		msg->pOrigin = SIP_NULL;
	}
	else
	{
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pOrigin->dRefCount);
		msg->pOrigin = pOrigin;
#else
		if(sip_initSdpOrigin(&(msg->pOrigin),err)==SipFail)
				return SipFail;
		if(__sip_cloneSdpOrigin(msg->pOrigin,pOrigin,err)==SipFail)
		{
			return SipFail;
		}
#endif
	}

	SIPDEBUGFN("Exiting setOrigin\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/********************************************************************
** FUNCTION:sdp_getConnection
**
** DESCRIPTION: This function retrieves the slConnection field from an
**		SDP message structure
**
**********************************************************************/

SipBool sdp_getConnection
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SdpMessage *msg, SdpConnection *slConnection, SipError *err)
#else
	( SdpMessage *msg, SdpConnection **slConnection, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	( msg, slConnection, err )
	  SdpMessage *msg;
	  SdpConnection *slConnection;
	  SipError *err;
#else
	( msg, slConnection, err )
	  SdpMessage *msg;
	  SdpConnection **slConnection;
	  SipError *err;
#endif
#endif
{
	SIPDEBUGFN("Entering getOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(slConnection==SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(msg->slConnection == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if(__sip_cloneSdpConnection(slConnection,msg->slConnection,err)==SipFail)
	{
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF((msg->slConnection)->dRefCount);
	*slConnection = msg->slConnection;
#endif

	SIPDEBUGFN("Exiting getVersion\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*******************************************************************
** FUNCTION: sdp_setConnection
**
** DESCRIPTION: This function sets the slConnection field in an SDP
**		message structure
**
*******************************************************************/
SipBool sdp_setConnection
#ifdef ANSI_PROTO
	( SdpMessage *msg, SdpConnection *slConnection, SipError *err)
#else
	( msg, slConnection, err )
	  SdpMessage *msg;
	  SdpConnection *slConnection;
	  SipError *err;
#endif
{

	SIPDEBUGFN("Entering setVersion\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if(msg->slConnection!=SIP_NULL)
	{
		sip_freeSdpConnection(msg->slConnection);
	}
	if(slConnection == SIP_NULL)
	{
		msg->slConnection = SIP_NULL;
	}
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSdpConnection(&(msg->slConnection),err)==SipFail)
				return SipFail;
		if(__sip_cloneSdpConnection(msg->slConnection,slConnection,err)==SipFail)
		{
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slConnection->dRefCount);
		msg->slConnection = slConnection;
#endif
	}

	SIPDEBUGFN("Exiting setVersion\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sdp_getUserFromOrigin
**
** DESCRIPTION: This function retrieves the pUser field from a SDP
**		pOrigin
**
**********************************************************************/

SipBool sdp_getUserFromOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,
	  SIP_S8bit ** pUser,
	  SipError * err)
#else
	( pOrigin, pUser, err )
	  SdpOrigin * pOrigin;
	  SIP_S8bit ** pUser;
	  SipError * err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit *temp_user;
	SIPDEBUGFN("Entering getUserFromOrigin\n") ;
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;
	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pUser == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_user = ( (SdpOrigin *) (pOrigin) )->pUser;

	if( temp_user == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_user );
	*pUser = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pUser == SIP_NULL )
		return SipFail;

	strcpy( *pUser, temp_user );
#else
	*pUser = temp_user;
#endif

	 SIPDEBUGFN("Exiting getUserFromOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sdp_setUserInOrigin
**
** DESCRIPTION: This function sets the pOrigin in an SDP pOrigin
**
*******************************************************************/
SipBool sdp_setUserInOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,	/*Origin pHeader */
	  SIP_S8bit * pUser,
	  SipError * err)
#else
	( pOrigin, pUser, err )	/*Origin pHeader */
	  SdpOrigin * pOrigin;
	  SIP_S8bit * pUser;
	  SipError * err;
#endif

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_user;
	SdpOrigin * temp_origin;
	SIPDEBUGFN("Entering setUserInOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;
	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

#endif
	if( pUser == SIP_NULL)
		temp_user = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pUser );
		temp_user = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_user == SIP_NULL )
			return SipFail;
		strcpy( temp_user, pUser );
#else
		temp_user = pUser;
#endif
	}


	temp_origin = ( SdpOrigin *) ( pOrigin);
	if ( temp_origin->pUser != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_origin->pUser, err) == SipFail)
			return SipFail;
	}


	temp_origin->pUser = temp_user;



	 SIPDEBUGFN("Exitiing setUserInOrigin\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_getSessionIdFromOrigin
**
** DESCRIPTION: This function retrieves the Session Id from an SDP
**		pOrigin
**
**********************************************************************/

SipBool sdp_getSessionIdFromOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,	/*Origin pHeader */
	  SIP_S8bit ** id,
	  SipError * err)
#else
	( pOrigin, id, err )
	  SdpOrigin * pOrigin;      /*Origin pHeader */
	  SIP_S8bit ** id;
	  SipError * err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_id;
	SIPDEBUGFN("Entering getSessionIdFromOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( id == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_id = ( (SdpOrigin *) (pOrigin) )->pSessionid;

	if( temp_id == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_id );
	*id = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *id == SIP_NULL )
		return SipFail;

	strcpy( *id, temp_id );
#else
	*id = temp_id;
#endif

	SIPDEBUGFN("Exiting getSessionIdFromOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sdp_setSessionIdInOrigin
**
** DESCRIPTION: This function sets the pSession-id field in an SDP
**		Origin
**
*******************************************************************/
SipBool sdp_setSessionIdInOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,	/*Origin pHeader */
	  SIP_S8bit * id,
	  SipError * err)
#else
	( pOrigin, id, err )	/*Origin pHeader */
	  SdpOrigin * pOrigin;
	  SIP_S8bit * id;
	  SipError * err;
#endif

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_id;
	SdpOrigin *temp_origin;
	SIPDEBUGFN("Entering setSessionIdInOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( id == SIP_NULL)
		temp_id = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( id );
		temp_id = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_id == SIP_NULL )
			return SipFail;
		strcpy( temp_id, id );
#else
		temp_id = id;
#endif
	}

	temp_origin = ( SdpOrigin *) ( pOrigin);
	if ( temp_origin->pSessionid != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_origin->pSessionid, err) == SipFail)
			return SipFail;
	}


	temp_origin->pSessionid = temp_id;



	SIPDEBUGFN("Exitiing setSessionIdInOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sdp_getVersionFromOrigin
**
** DESCRIPTION: This function retrieves the pVersion filed from an SDP
**		pOrigin
**
**********************************************************************/

SipBool sdp_getVersionFromOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,	/*Origin pHeader */
	  SIP_S8bit ** pVersion,
	  SipError * err)
#else
	( pOrigin, pVersion, err )
	  SdpOrigin * pOrigin;      /*Origin pHeader */
	  SIP_S8bit ** pVersion;
	  SipError * err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_version;
	 SIPDEBUGFN("Entering getVersionFromOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pVersion == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_version = ( (SdpOrigin *) (pOrigin) )->pVersion;

	if( temp_version == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_version );
	*pVersion = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pVersion == SIP_NULL )
		return SipFail;

	strcpy(*pVersion, temp_version);
#else
	*pVersion = temp_version;
#endif

	SIPDEBUGFN("Exiting getVersionFromOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sdp_setVersionInOrigin
**
** DESCRIPTION: This function sets the pVersion filed in an SDP pOrigin
**
*******************************************************************/
SipBool sdp_setVersionInOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,	/*Origin pHeader */
	  SIP_S8bit * pVersion,
	  SipError * err)
#else
	( pOrigin, pVersion, err )	/*Origin pHeader */
	  SdpOrigin * pOrigin;
	  SIP_S8bit * pVersion;
	  SipError * err;
#endif

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_version;
	SdpOrigin * temp_origin;
	SIPDEBUGFN("Entering setVersionInOrigin\n") ;
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( pVersion == SIP_NULL)
		temp_version = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pVersion );
		temp_version = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_version == SIP_NULL )
			return SipFail;
		strcpy( temp_version, pVersion );
#else
		temp_version = pVersion;
#endif
	}
	temp_origin = ( SdpOrigin *) ( pOrigin);
	if ( temp_origin->pVersion != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_origin->pVersion, err) == SipFail)
			return SipFail;
	}


	temp_origin->pVersion = temp_version;

	SIPDEBUGFN("Exitiing setVersionInOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_getNetTypeFromOrigin
**
** DESCRIPTION: This function retrieves the network-dType field from an
**		SDP pOrigin
**
**********************************************************************/
SipBool sdp_getNetTypeFromOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,	/*Origin pHeader */
	  SIP_S8bit ** ntype,
	  SipError * err)
#else
	( pOrigin, ntype, err )
	  SdpOrigin * pOrigin;      /*Origin pHeader */
	  SIP_S8bit ** ntype;
	  SipError * err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_net_type;
	 SIPDEBUGFN("Entering getNetTypeFromOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;
	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (ntype == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_net_type = ( (SdpOrigin *) (pOrigin) )->pNetType;

	if( temp_net_type == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_net_type );
	*ntype = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *ntype == SIP_NULL )
		return SipFail;

	strcpy( *ntype, temp_net_type );
#else
	*ntype = temp_net_type;
#endif

	 SIPDEBUGFN("Exiting getNetTypeFromOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sdp_setNetTypeInOrigin
**
** DESCRIPTION: This function sets the network-dType field in an SDP
**		Origin
**
*******************************************************************/
SipBool sdp_setNetTypeInOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,	/*Origin pHeader */
	  SIP_S8bit * ntype,
	  SipError * err)
#else
	( pOrigin, ntype, err )	/*Origin pHeader */
	  SdpOrigin * pOrigin;
	  SIP_S8bit * ntype;
	  SipError * err;
#endif

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_net_type;
	SdpOrigin * temp_origin;
	SIPDEBUGFN("Entering setNetTypeInOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

#endif
	if( ntype == SIP_NULL)
		temp_net_type = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( ntype );
		temp_net_type = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_net_type == SIP_NULL )
			return SipFail;
		strcpy( temp_net_type, ntype );
#else
		temp_net_type = ntype;
#endif
	}


	temp_origin = ( SdpOrigin *) ( pOrigin);
	if ( temp_origin->pNetType != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_origin->pNetType, err) == SipFail)
			return SipFail;
	}


	temp_origin->pNetType = temp_net_type;



	SIPDEBUGFN("Exitiing setNetTypeInOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}
/********************************************************************
** FUNCTION:sdp_getAddrFromOrigin
**
** DESCRIPTION: This function retrieves the pTranspAddr from an SDP pOrigin
**
**********************************************************************/

SipBool sdp_getAddrFromOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,	/*Origin pHeader */
	  SIP_S8bit ** dAddr,
	  SipError * err)
#else
	( pOrigin, dAddr, err )
	  SdpOrigin * pOrigin;      /*Origin pHeader */
	  SIP_S8bit ** dAddr;
	  SipError * err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_addr;
	SIPDEBUGFN("Entering getAddrFromOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( dAddr == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_addr = ( (SdpOrigin *) (pOrigin) )->dAddr;

	if( temp_addr == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_addr );
	*dAddr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *dAddr == SIP_NULL )
		return SipFail;

	strcpy( *dAddr, temp_addr );
#else
	*dAddr = temp_addr;
#endif
	 SIPDEBUGFN("Exiting getAddrFromOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sdp_setAddrInOrigin
**
** DESCRIPTION: This function sets the pTranspAddr field in an SDP pOrigin
**
*******************************************************************/
SipBool sdp_setAddrInOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,	/*Origin pHeader */
	  SIP_S8bit * dAddr,
	  SipError * err)
#else
	( pOrigin, dAddr, err )	/*Origin pHeader */
	  SdpOrigin * pOrigin;
	  SIP_S8bit * dAddr;
	  SipError * err;
#endif

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_addr;
	SdpOrigin * temp_origin;
	SIPDEBUGFN("Entering setAddrInOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( dAddr == SIP_NULL)
		temp_addr = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( dAddr );
		temp_addr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_addr == SIP_NULL )
			return SipFail;
		strcpy( temp_addr, dAddr );
#else
		temp_addr = dAddr;
#endif
	}

	temp_origin = ( SdpOrigin *) ( pOrigin);
	if ( temp_origin->dAddr != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_origin->dAddr, err) == SipFail)
			return SipFail;
	}


	temp_origin->dAddr = temp_addr;



	SIPDEBUGFN("Exitiing setAddrInOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}
/********************************************************************
** FUNCTION:sdp_getAddrTypeFromOrigin
**
** DESCRIPTION: This function retrieves the dType of pTranspAddr from an SDP
**		Origin
**
**********************************************************************/

SipBool sdp_getAddrTypeFromOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,
	  SIP_S8bit ** pAddrType,
	  SipError * err)
#else
	( pOrigin, pAddrType, err )
	  SdpOrigin * pOrigin;
	  SIP_S8bit ** pAddrType;
	  SipError * err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_addr_type;
	SIPDEBUGFN("Entering getAddrTypeFromOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;
	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pAddrType == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_addr_type = ( (SdpOrigin *) (pOrigin) )->pAddrType;

	if( temp_addr_type == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_addr_type );
	*pAddrType = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pAddrType == SIP_NULL )
		return SipFail;

	strcpy( *pAddrType, temp_addr_type );
#else
	*pAddrType = temp_addr_type;
#endif

	SIPDEBUGFN("Exiting getAddrTypeFromOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sdp_setAddrTypeInOrigin
**
** DESCRIPTION: This function sets the pTranspAddr-dType field in an SDP
**		Origin
**
*******************************************************************/
SipBool sdp_setAddrTypeInOrigin
#ifdef ANSI_PROTO
	( SdpOrigin * pOrigin,
	  SIP_S8bit * pAddrType,
	  SipError * err)
#else
	( pOrigin, pAddrType, err )
	  SdpOrigin * pOrigin;
	  SIP_S8bit * pAddrType;
	  SipError * err;
#endif

{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_addr_type;
	SdpOrigin * temp_origin;
	SIPDEBUGFN("Entering setAddrTypeInOrigin\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( pOrigin == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( pAddrType == SIP_NULL)
		temp_addr_type = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pAddrType );
		temp_addr_type = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_addr_type == SIP_NULL )
			return SipFail;
		strcpy( temp_addr_type, pAddrType );
#else
		temp_addr_type = pAddrType;
#endif
	}

	temp_origin = ( SdpOrigin *) ( pOrigin);
	if ( temp_origin->pAddrType != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_origin->pAddrType, err) == SipFail)
			return SipFail;
	}


	temp_origin->pAddrType = temp_addr_type;



	 SIPDEBUGFN("Exitiing setAddrTypeInOrigin\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sdp_getSession
**
** DESCRIPTION: This function retrieves the pSession field from an SDP
**		message structure
**
**********************************************************************/

SipBool sdp_getSession
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit **pSession, SipError *err)
#else
	( msg, pSession, err )
	  SdpMessage *msg;
	  SIP_S8bit **pSession;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_session;
	SIPDEBUGFN("Entering getSession\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pSession == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_session = msg->pSession;

	if( temp_session == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_session);
	*pSession = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pSession == SIP_NULL )
		return SipFail;

	strcpy( *pSession , temp_session );
#else
	*pSession = temp_session;
#endif

	SIPDEBUGFN("Exiting getSession\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*******************************************************************
** FUNCTION: sdp_setSession
**
** DESCRIPTION: This function sets the pSession field in an SDP
**		message structure
**
*******************************************************************/
SipBool sdp_setSession
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *pSession, SipError *err)
#else
	( msg, pSession, err )
	  SdpMessage *msg;
	  SIP_S8bit *pSession;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_session;
	SdpMessage *temp_mssg;
	 SIPDEBUGFN("Entering setSession\n") ;
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pSession == SIP_NULL)
		temp_session = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pSession );
		temp_session = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_session== SIP_NULL )
			return SipFail;

		strcpy( temp_session, pSession );
#else
		temp_session = pSession;
#endif

	}

	temp_mssg = ( SdpMessage *)msg ;

	if ( temp_mssg->pSession != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_mssg->pSession, err) == SipFail)
			return SipFail;
	}
        temp_mssg->pSession=temp_session;



	SIPDEBUGFN("Exiting setSession\n") ;
	*err = E_NO_ERROR;
	return SipSuccess;

}


/********************************************************************
** FUNCTION:sdp_getUri
**
** DESCRIPTION: This function retrieves the URI from an SDP message
**		structure
**
**********************************************************************/

SipBool sdp_getUri
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit **pUri, SipError *err)
#else
	( msg, pUri, err )
	  SdpMessage *msg;
	  SIP_S8bit **pUri;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_uri;
	 SIPDEBUGFN("Entering getUri\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pUri == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_uri = msg->pUri;

	if( temp_uri == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_uri);
	*pUri = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pUri == SIP_NULL )
		return SipFail;

	strcpy( *pUri , temp_uri );
#else
	*pUri = temp_uri;
#endif

	SIPDEBUGFN("Exiting getUri\n");
	*err = E_NO_ERROR;
	return SipSuccess;

}
/*******************************************************************
** FUNCTION: sdp_setUri
**
** DESCRIPTION: This function sets the URI in an SDP message structure
**
*******************************************************************/
SipBool sdp_setUri
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *pUri, SipError *err)
#else
	( msg, pUri, err )
	  SdpMessage *msg;
	  SIP_S8bit *pUri;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_uri;
	SdpMessage *temp_mssg;
	SIPDEBUGFN("Entering setUri\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pUri == SIP_NULL)
		temp_uri = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pUri );
		temp_uri = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_uri== SIP_NULL )
			return SipFail;

		strcpy( temp_uri, pUri );
#else
		temp_uri = pUri;
#endif

	}

	temp_mssg = ( SdpMessage *)msg ;

	if ( temp_mssg->pUri != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_mssg->pUri, err) == SipFail)
			return SipFail;
	}
        temp_mssg->pUri=temp_uri;
	SIPDEBUGFN("Exiting setUri\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/********************************************************************
** FUNCTION:sdp_getInfo
**
** DESCRIPTION: This function retrieves the Info field from an SDP
** 		message structure
**
**********************************************************************/

SipBool sdp_getInfo
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit **pInformation, SipError *err)
#else
	( msg, pInformation, err )
	  SdpMessage *msg;
	  SIP_S8bit **pInformation;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_info;
	SIPDEBUGFN("Entering getInfo\n")	;
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pInformation == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_info = msg->pInformation;

	if( temp_info == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_info);
	*pInformation = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pInformation == SIP_NULL )
		return SipFail;

	strcpy( *pInformation , temp_info );
#else
	*pInformation = temp_info;
#endif

	SIPDEBUGFN("Exiting getInfo\n");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*******************************************************************
** FUNCTION: sdp_setInfo
**
** DESCRIPTION: This function sets the Info field in an SDP message
**		structure
**
*******************************************************************/
SipBool sdp_setInfo
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *pInformation, SipError *err)
#else
	( msg, pInformation, err )
	  SdpMessage *msg;
	  SIP_S8bit *pInformation;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_info;
	SdpMessage *temp_mssg;
	SIPDEBUGFN("Entering setInfo\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;
	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pInformation == SIP_NULL)
		temp_info = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pInformation );
		temp_info = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_info== SIP_NULL )
			return SipFail;

		strcpy( temp_info, pInformation );
#else
		temp_info = pInformation;
#endif

	}

	temp_mssg = ( SdpMessage *)msg ;

	if ( temp_mssg->pInformation != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_mssg->pInformation, err) == SipFail)
			return SipFail;
	}
        temp_mssg->pInformation=temp_info;


	SIPDEBUGFN("Exiting setInfo\n");
 	*err = E_NO_ERROR;
	return SipSuccess;

}
/********************************************************************
** FUNCTION:sdp_getKey
**
** DESCRIPTION: This function retrieves the Key from an SDP message
**		structure
**
**********************************************************************/

SipBool sdp_getKey
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit **pKey, SipError *err)
#else
	( msg, pKey, err )
	  SdpMessage *msg;
	  SIP_S8bit **pKey;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_key;
	SIPDEBUGFN("Entering getKey\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pKey == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_key = msg->pKey;

	if( temp_key == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_key);
	*pKey = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pKey == SIP_NULL )
		return SipFail;

	strcpy( *pKey , temp_key );
#else
	*pKey = temp_key;
#endif

	SIPDEBUGFN("Exiting getKey\n");

	*err = E_NO_ERROR;
	return SipSuccess;


}

/*******************************************************************
** FUNCTION: sdp_setKey
**
** DESCRIPTION: This function sets the pKey in an SDP message structure
**
*******************************************************************/
SipBool sdp_setKey
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *pKey, SipError *err)
#else
	( msg, pKey, err )
	  SdpMessage *msg;
	  SIP_S8bit *pKey;
	  SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_key;
	SdpMessage *temp_mssg;
	SIPDEBUGFN("Entering setKey\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pKey == SIP_NULL)
		temp_key = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pKey );
		temp_key = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_key== SIP_NULL )
			return SipFail;

		strcpy( temp_key, pKey );
#else
		temp_key = pKey;
#endif

	}

	temp_mssg = ( SdpMessage *)msg ;

	if ( temp_mssg->pKey != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_mssg->pKey, err) == SipFail)
			return SipFail;
	}
        temp_mssg->pKey=temp_key;
	SIPDEBUGFN("Exiting setKey\n");

	*err = E_NO_ERROR;
	return SipSuccess;


}

/********************************************************************
** FUNCTION:sdp_getBandwidthCountFromMedia
**
** DESCRIPTION: This function retrieves the Bandwidth count from an
**		SDP message structure
**
**********************************************************************/

SipBool sdp_getBandwidthCount
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_U32bit *cnt, SipError *err )
#else
	( msg,cnt,err)
	  SdpMessage *msg;
	  SIP_U32bit *cnt;
	  SipError *err;
#endif
{
	SIPDEBUGFN("Entering getBandwidthCount\n");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if( (msg == SIP_NULL) || ( cnt == SIP_NULL ) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if (sip_listSizeOf(&(msg->pBandwidth), cnt , err) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting getBandwidthCount\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sdp_getBandwidthAtIndex
**
** DESCRIPTION: This function retrieves the pBandwidth field at a
**		specified index in an SDP message structure
**
*******************************************************************/
SipBool sdp_getBandwidthAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit **pBandwidth, SIP_U32bit cnt, SipError *err )
#else
	( msg,pBandwidth,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit **pBandwidth;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{
	SIP_Pvoid element_from_list;

	SIPDEBUGFN("Entering getBandwidthAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (msg == SIP_NULL) || (pBandwidth == SIP_NULL) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( sip_listGetAt(&(msg->pBandwidth), cnt, &element_from_list, err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	*pBandwidth = (SIP_S8bit *) STRDUPACCESSOR((SIP_S8bit *)element_from_list);
	if (*pBandwidth == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*pBandwidth = (SIP_S8bit *) element_from_list;
#endif

	SIPDEBUGFN("Exiting getBandwidthAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sdp_setBandwidthAtIndex
**
** DESCRIPTION: This function sets the pBandwidth field at a specified
**		index in an SDP message structure
**
**********************************************************************/

SipBool sdp_setBandwidthAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *pBandwidth, SIP_U32bit cnt, SipError *err )
#else
	( msg,pBandwidth,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit *pBandwidth;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{

	SIP_S8bit * element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif
	SIPDEBUGFN("Enterring setBandwidthAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;


	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pBandwidth == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *)STRDUPACCESSOR(pBandwidth);
		if (element_in_list == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = pBandwidth;
#endif
	}

	if( sip_listSetAt( &(msg->pBandwidth),cnt, (SIP_Pvoid)element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (fast_memfree (0, element_in_list, &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}



	SIPDEBUGFN("Exiting setBandwidthAtIndex\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************************
** FUNCTION: sdp_insertBandwidthAtIndex
**
** DESCRIPTION: This function inserts a pBandwidth field at a specified
**		index in an SDP message structure
**
****************************************************************************/
SipBool sdp_insertBandwidthAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *pBandwidth, SIP_U32bit cnt, SipError *err )
#else
	( msg,pBandwidth,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit *pBandwidth;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{

	SIP_S8bit * element_in_list;
	SipError temp_err;

	SIPDEBUGFN("Entering insertBandwidthAtIndex\n");
	temp_err = E_NO_ERROR;
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pBandwidth == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *) STRDUPACCESSOR(pBandwidth);
		if (element_in_list == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = pBandwidth;
#endif
	}

	if( sip_listInsertAt( &(msg->pBandwidth),cnt , (SIP_Pvoid) element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (fast_memfree(ACCESSOR_MEM_ID, element_in_list, &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting insertBandwidthAtIndex\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_deleteBandwidthAtIndex
**
** DESCRIPTION: This function deletes a pBandwidth field at a specified
**		index in an SDP message structure
**
**********************************************************************/

SipBool sdp_deleteBandwidthAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_U32bit cnt, SipError *err )
#else
	( msg,cnt,err)
	  SdpMessage *msg;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{

	SIPDEBUGFN("Entering deleteBandwidthAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(msg->pBandwidth), cnt, err) == SipFail)
		return SipFail;

	SIPDEBUGFN("Exiting deleteBandwidthAtIndex\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sdp_getPhoneCount
**
** DESCRIPTION: This function retrieves the number of slPhone fields
**		in an SDP message structure
**
*******************************************************************/
SipBool sdp_getPhoneCount
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_U32bit *cnt, SipError *err )
#else
	( msg,cnt,err)
	  SdpMessage *msg;
	  SIP_U32bit *cnt;
	  SipError *err;
#endif
{
	SIPDEBUGFN("Entering getPhoneCount\n");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if( (msg == SIP_NULL) || ( cnt == SIP_NULL ) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

#endif

	if (sip_listSizeOf(&(msg->slPhone), cnt , err) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting getPhoneCount\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}
/********************************************************************
** FUNCTION:sdp_getPhoneAtIndex
**
** DESCRIPTION: This function retrieves the slPhone field at specified
**		index in the SDP message structure
**
**********************************************************************/

SipBool sdp_getPhoneAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit **slPhone, SIP_U32bit cnt, SipError *err )
#else
	( msg,slPhone,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit **slPhone;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{
	SIP_Pvoid element_from_list;

	SIPDEBUGFN("Entering getPhoneAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (msg == SIP_NULL) || (slPhone == SIP_NULL) )
	{

		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( sip_listGetAt(&(msg->slPhone), cnt, &element_from_list, err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	*slPhone = (SIP_S8bit *) STRDUPACCESSOR((SIP_S8bit *)element_from_list);
	if (*slPhone == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*slPhone = (SIP_S8bit *) element_from_list;
#endif
	SIPDEBUGFN("Exiting getPhoneAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sdp_setPhoneAtIndex
**
** DESCRIPTION: This function sets the slPhone field at a specified
**		index in an SDP message structure
**
*******************************************************************/
SipBool sdp_setPhoneAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *slPhone, SIP_U32bit cnt, SipError *err )
#else
	( msg,slPhone,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit *slPhone;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{

	SIP_S8bit * element_in_list;
	SipError temp_err;
	SIPDEBUGFN("Enterring setPhoneAtIndex\n");
	temp_err = E_NO_ERROR;
#ifndef SIP_NO_CHECK

	if( err == SIP_NULL )
		return SipFail;


	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (slPhone == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *)STRDUPACCESSOR(slPhone);
		if (element_in_list == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = slPhone;
#endif
	}

	if( sip_listSetAt( &(msg->slPhone),cnt, (SIP_Pvoid)element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (fast_memfree (0, element_in_list, &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}



	SIPDEBUGFN("Exiting setPhoneAtIndex\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_insertPhoneAtIndex
**
** DESCRIPTION: This function inserts a slPhone filed at a specified
**		index in an SDP message structure
**
**********************************************************************/

SipBool sdp_insertPhoneAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *slPhone, SIP_U32bit cnt, SipError *err )
#else
	( msg,slPhone,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit *slPhone;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{

	SIP_S8bit * element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif

	SIPDEBUGFN("Entering insertPhoneAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (slPhone == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *) STRDUPACCESSOR(slPhone);
		if (element_in_list == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = slPhone;
#endif
	}

	if( sip_listInsertAt( &(msg->slPhone),cnt , (SIP_Pvoid) element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (fast_memfree(ACCESSOR_MEM_ID, element_in_list, &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting insertPhoneAtIndex\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*******************************************************************
** FUNCTION: sdp_deletePhoneAtIndex
**
** DESCRIPTION: This function deletes a slPhone field at specified index
**		in an SDP message structure
**
*******************************************************************/
SipBool sdp_deletePhoneAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_U32bit cnt, SipError *err )
#else
	( msg,cnt,err)
	  SdpMessage *msg;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{

	SIPDEBUGFN("Entering deletePhoneAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(msg->slPhone), cnt, err) == SipFail)
		return SipFail;

	SIPDEBUGFN("Exiting deletePhoneAtIndex\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sdp_getEmailCount
**
** DESCRIPTION: This function retrieves the number of e-mails from an
**		SDP message structure
**
**********************************************************************/

SipBool sdp_getEmailCount
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_U32bit *cnt, SipError *err )
#else
	( msg,cnt,err)
	  SdpMessage *msg;
	  SIP_U32bit *cnt;
	  SipError *err;
#endif
{
	SIPDEBUGFN("Entering getEmailCount\n");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if( (msg == SIP_NULL) || ( cnt == SIP_NULL ) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(msg->slEmail), cnt , err) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting getEmailCount\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sdp_getEmailAtIndex
**
** DESCRIPTION: This function retrieves a e-mail at specified index
**		from an SDP message structure
**
*******************************************************************/
SipBool sdp_getEmailAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit **slEmail, SIP_U32bit cnt, SipError *err )
#else
	( msg,slEmail,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit **slEmail;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{
	SIP_Pvoid element_from_list;

	SIPDEBUGFN("Entering getEmailAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (msg == SIP_NULL) || (slEmail == SIP_NULL) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( sip_listGetAt(&(msg->slEmail), cnt, &element_from_list, err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	*slEmail = (SIP_S8bit *) STRDUPACCESSOR((SIP_S8bit *)element_from_list);
	if (*slEmail == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*slEmail = (SIP_S8bit *) element_from_list;
#endif

	SIPDEBUGFN("Exiting getEmailAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_setEmailAtIndex
**
** DESCRIPTION: This function sets the e-mail at a specified index in
**		an SDP Message structure
**
**********************************************************************/

SipBool sdp_setEmailAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *slEmail, SIP_U32bit cnt, SipError *err )
#else
	( msg,slEmail,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit *slEmail;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{

	SIP_S8bit * element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif

#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;


	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (slEmail == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *)STRDUPACCESSOR(slEmail);
		if (element_in_list == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = slEmail;
#endif
	}

	if( sip_listSetAt( &(msg->slEmail),cnt, (SIP_Pvoid)element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (fast_memfree (0, element_in_list, &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}



	SIPDEBUGFN("Exiting setEmailAtIndex\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}



/*******************************************************************
** FUNCTION: sdp_insertEmailAtIndex
**
** DESCRIPTION: This function inserts an e-mail at a specified index
**		in an SDP message structure
**
*******************************************************************/
SipBool sdp_insertEmailAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit *slEmail, SIP_U32bit cnt, SipError *err )
#else
	( msg,slEmail,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit *slEmail;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{

	SIP_S8bit * element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;
#endif

	SIPDEBUGFN("Entering insertEmailAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (slEmail == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *) STRDUPACCESSOR(slEmail);
		if (element_in_list == SIP_NULL)
		{
			*err = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = slEmail;
#endif
	}

	if( sip_listInsertAt( &(msg->slEmail),cnt , (SIP_Pvoid) element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (fast_memfree(ACCESSOR_MEM_ID, element_in_list, &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting insertEmailAtIndex\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_deleteEmailAtIndex
**
** DESCRIPTION: This function deletes an e-mail at a specified index in
**		an SDP message structure
**
**********************************************************************/

SipBool sdp_deleteEmailAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_U32bit cnt, SipError *err )
#else
	( msg,cnt,err)
	  SdpMessage *msg;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{

	SIPDEBUGFN("Entering deleteEmailAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(msg->slEmail), cnt, err) == SipFail)
		return SipFail;

	SIPDEBUGFN("Exiting deleteEmailAtIndex\n");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sdp_getMediaCount
**
** DESCRIPTION: This function retrieves the number of slMedia parameters
**		from an SDP message stucture
**
*******************************************************************/
SipBool sdp_getMediaCount
#ifdef ANSI_PROTO
	( SdpMessage	*msg,
	  SIP_U32bit	*cnt,
	  SipError	*err  )
#else
	( msg,cnt,err)
	  SdpMessage 	*msg;
	  SIP_U32bit 	*cnt;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN("Entering getMediaCount\n");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if ( (msg == SIP_NULL) || ( cnt == SIP_NULL ))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(msg->slMedia), cnt , err) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting getMediaCount\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sdp_getMediaAtIndex
**
** DESCRIPTION: This function retrieves a slMedia-parameter at a specified
**		index in an SDP message structure
**
**********************************************************************/

SipBool sdp_getMediaAtIndex
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SdpMessage 	*msg,
	  SdpMedia 	*slMedia,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( SdpMessage 	*msg,
	  SdpMedia 	**slMedia,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#endif
#else
#ifndef SIP_BY_REFERENCE
	( msg,slMedia,cnt,err)
	  SdpMessage 	*msg;
	  SdpMedia	*slMedia;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#else
	( msg,slMedia,cnt,err)
	  SdpMessage 	*msg;
	  SdpMedia	**slMedia;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
#endif
{
	SIP_Pvoid 	element_from_list;

	SIPDEBUGFN("Entering getMediaAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (msg == SIP_NULL) || (slMedia == SIP_NULL) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(msg->slMedia), cnt, &element_from_list, err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSdpMedia(slMedia,(SdpMedia *)element_from_list,err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(((SdpMedia *)element_from_list)->dRefCount);
	*slMedia = (SdpMedia *)element_from_list;
#endif
	SIPDEBUGFN("Exiting getMediaAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}
/*****************************************************************
**
** FUNCTION:  sdp_setMediaAtIndex
**
** DESCRIPTION: This function sets a slMedia-parameter at specified
**		index in an SDP message structure
**
******************************************************************/
SipBool sdp_setMediaAtIndex
#ifdef ANSI_PROTO
	( SdpMessage 	*msg,
	  SdpMedia	*slMedia,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( msg,slMedia,cnt,err)
	  SdpMessage 	*msg;
	  SdpMedia	*slMedia;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
{
	SdpMedia 	*element_in_list;

	SIPDEBUGFN("Entering setMediaAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( slMedia == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSdpMedia(&element_in_list,err) == SipFail)
			return SipFail;
		if ( __sip_cloneSdpMedia(element_in_list,slMedia,err) == SipFail)
		{
			sip_freeSdpMedia(element_in_list);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slMedia->dRefCount);
		element_in_list = (SdpMedia *)slMedia;
#endif
	}
	if(sip_listSetAt(&(msg->slMedia),cnt,element_in_list,err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		sip_freeSdpMedia(element_in_list);
#else
		HSS_LOCKEDDECREF(slMedia->dRefCount);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting setMediaAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_insertMediaAtIndex
**
** DESCRIPTION: This function inserts a slMedia-parameter at a specified
**		index in an SDP message
**
**********************************************************************/

SipBool sdp_insertMediaAtIndex
#ifdef ANSI_PROTO
	( SdpMessage 	*msg,
	  SdpMedia	*slMedia,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( msg,slMedia,cnt,err)
	  SdpMessage 	*msg;
	  SdpMedia	*slMedia;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
{
	SdpMedia 	*element_in_list;

	SIPDEBUGFN("Entering InsertMediaAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the slMedia structure */
	if ( slMedia == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSdpMedia(&element_in_list,err) == SipFail)
			return SipFail;

		if ( __sip_cloneSdpMedia(element_in_list, slMedia,err) == SipFail)
		{
			sip_freeSdpMedia(element_in_list);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slMedia->dRefCount);
		element_in_list = (SdpMedia *)slMedia;
#endif
	}

	if( sip_listInsertAt(&(msg->slMedia), cnt, element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		sip_freeSdpMedia(element_in_list);
#else
		HSS_LOCKEDDECREF(slMedia->dRefCount);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting InsertMediaAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sdp_deleteMediaAtIndex
**
** DESCRIPTION: This function deletes a slMedia-parameter at a specified
**		index in an SDP message structure
**
******************************************************************/
SipBool sdp_deleteMediaAtIndex
#ifdef ANSI_PROTO
	( SdpMessage 	*msg,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( msg,cnt,err)
	  SdpMessage 	*msg;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN("Entering deleteMediaAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(msg->slMedia), cnt, err) == SipFail)
		return SipFail;


	SIPDEBUGFN("Exiting deleteMediaAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sdp_getTimeCount
**
** DESCRIPTION: This function retrieves the number of slTime-parameters
**		from an SDP message
**
**********************************************************************/

SipBool sdp_getTimeCount
#ifdef ANSI_PROTO
	( SdpMessage	*msg,
	  SIP_U32bit	*cnt,
	  SipError	*err  )
#else
	( msg,cnt,err)
	  SdpMessage 	*msg;
	  SIP_U32bit 	*cnt;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN("Entering getTimeCount\n");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if ( (msg == SIP_NULL) || ( cnt == SIP_NULL ))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(msg->slTime), cnt , err) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting getTimeCount\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}
/*****************************************************************
**
** FUNCTION:  sdp_getTimeAtIndex
**
** DESCRIPTION: This function retrieves a slTime parameter a specified
**		index in an SDP message structure
**
******************************************************************/
SipBool sdp_getTimeAtIndex
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SdpMessage 	*msg,
	  SdpTime 	*slTime,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( SdpMessage 	*msg,
	  SdpTime 	**slTime,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#endif
#else
#ifndef SIP_BY_REFERENCE
	( msg,slTime,cnt,err)
	  SdpMessage 	*msg;
	  SdpTime	*slTime;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#else
	( msg,slTime,cnt,err)
	  SdpMessage 	*msg;
	  SdpTime	**slTime;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
#endif
{
	SIP_Pvoid 	element_from_list;

	SIPDEBUGFN("Entering getTimeAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (msg == SIP_NULL) || (slTime == SIP_NULL) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(msg->slTime), cnt, &element_from_list, err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if ( __sip_cloneSdpTime(slTime,(SdpTime *)element_from_list,err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(((SdpTime *)element_from_list)->dRefCount);
	*slTime = (SdpTime *)element_from_list;
#endif

	SIPDEBUGFN("Exiting getTimeAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_setTimeAtIndex
**
** DESCRIPTION: This function sets a slTime-parameter at a specified
**		index in an SDP message structure
**
**********************************************************************/

SipBool sdp_setTimeAtIndex
#ifdef ANSI_PROTO
	( SdpMessage 	*msg,
	  SdpTime	*slTime,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( msg,slTime,cnt,err)
	  SdpMessage 	*msg;
	  SdpTime	*slTime;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
{
	SdpTime 	*element_in_list;

	SIPDEBUGFN("Entering setTimeAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( slTime == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if ( sip_initSdpTime(&element_in_list,err) == SipFail)
			return SipFail;
		if ( __sip_cloneSdpTime(element_in_list, slTime,err) == SipFail)
		{
			sip_freeSdpTime(element_in_list);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slTime->dRefCount);
		element_in_list = slTime;
#endif
	}

	if( sip_listSetAt(&(msg->slTime), cnt, (SIP_Pvoid) element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		sip_freeSdpTime(element_in_list);
#else
		HSS_LOCKEDDECREF(slTime->dRefCount);
#endif
		return SipFail;
	}
	SIPDEBUGFN("Exiting setTimeAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sdp_insertTimeAtIndex
**
** DESCRIPTION: This function inserts a slTime-parameter at specified
**		index from an SDP message structure
**
******************************************************************/
SipBool sdp_insertTimeAtIndex
#ifdef ANSI_PROTO
	( SdpMessage 	*msg,
	  SdpTime	*slTime,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( msg,slTime,cnt,err)
	  SdpMessage 	*msg;
	  SdpTime	*slTime;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
{
	SdpTime 	*element_in_list;

	SIPDEBUGFN("Entering InsertTimeAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( slTime == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if ( sip_initSdpTime(&element_in_list,err) == SipFail)
			return SipFail;
		if ( __sip_cloneSdpTime(element_in_list, slTime,err) == SipFail)
		{
			sip_freeSdpTime(element_in_list);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slTime->dRefCount);
		element_in_list = slTime;
#endif
	}

	if( sip_listInsertAt(&(msg->slTime), cnt, (SIP_Pvoid) element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		sip_freeSdpTime(element_in_list);
#else
		HSS_LOCKEDDECREF(slTime->dRefCount);
#endif
		return SipFail;
	}
	SIPDEBUGFN("Exiting InsertTimeAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_deleteTimeAtIndex
**
** DESCRIPTION: This function deletes a slTime-parameter at specified
**		index in an SDP message structure
**
**********************************************************************/

SipBool sdp_deleteTimeAtIndex
#ifdef ANSI_PROTO
	( SdpMessage 	*msg,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( msg,cnt,err)
	  SdpMessage 	*msg;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN("Entering deleteTimeAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(msg->slTime), cnt, err) == SipFail)
		return SipFail;


	SIPDEBUGFN("Exiting deleteTimeAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sdp_getAttrCount
**
** DESCRIPTION: This function gets the numebr of attributes from an
**		SDP message structure
**
******************************************************************/
SipBool sdp_getAttrCount
#ifdef ANSI_PROTO
	( SdpMessage	*msg,
	  SIP_U32bit	*cnt,
	  SipError	*err  )
#else
	( msg,cnt,err)
	  SdpMessage 	*msg;
	  SIP_U32bit 	*cnt;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN("Entering getAttrCount\n");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if ( (msg == SIP_NULL) || ( cnt == SIP_NULL ))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(msg->slAttr), cnt , err) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting getAttrCount\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sdp_getAttrAt
**
** DESCRIPTION: This function retrieves an attribute at a specified
**		index from an SDP message stucture
**
**********************************************************************/

SipBool sdp_getAttrAtIndex
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SdpMessage 	*msg,
	  SdpAttr 	*slAttr,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( SdpMessage 	*msg,
	  SdpAttr 	**slAttr,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#endif
#else
#ifndef SIP_BY_REFERENCE
	( msg,slAttr,cnt,err)
	  SdpMessage 	*msg;
	  SdpAttr	*slAttr;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#else
	( msg,slAttr,cnt,err)
	  SdpMessage 	*msg;
	  SdpAttr	**slAttr;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
#endif
{
	SIP_Pvoid 	element_from_list;

	SIPDEBUGFN("Entering getAttrAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (msg == SIP_NULL) || (slAttr == SIP_NULL) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(msg->slAttr), cnt, &element_from_list, err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if ( __sip_cloneSdpAttr(slAttr,(SdpAttr *)element_from_list,err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(((SdpAttr *)element_from_list)->dRefCount);
	*slAttr = (SdpAttr *)element_from_list;
#endif
	SIPDEBUGFN("Exiting getAttrAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sdp_setAttrAtIndex
**
** DESCRIPTION: This function sets the attribute at specified index
**		in an SDP message structure
**
******************************************************************/
SipBool sdp_setAttrAtIndex
#ifdef ANSI_PROTO
	( SdpMessage 	*msg,
	  SdpAttr	*slAttr,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( msg,slAttr,cnt,err)
	  SdpMessage 	*msg;
	  SdpAttr	*slAttr;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
{
	SdpAttr 	*element_in_list;

	SIPDEBUGFN("Entering setAttrAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( slAttr == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSdpAttr(&element_in_list,err)==SipFail)
			return SipFail;
		if ( __sip_cloneSdpAttr(element_in_list, slAttr,err) == SipFail)
		{
			sip_freeSdpAttr(element_in_list);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slAttr->dRefCount);
		element_in_list = slAttr;
#endif
	}

	if( sip_listSetAt(&(msg->slAttr), cnt, (SIP_Pvoid) element_in_list, err) == SipFail)
	{
		if ( element_in_list != SIP_NULL )
#ifndef SIP_BY_REFERENCE
			sip_freeSdpAttr(element_in_list);
#else
			HSS_LOCKEDDECREF(slAttr->dRefCount);
#endif
		return SipFail;
	}
	SIPDEBUGFN("Exiting setAttrAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_insertAttrAtIndex
**
** DESCRIPTION: This function inserts an attribute at a specified index
**		in an SDP message structure
**
**********************************************************************/

SipBool sdp_insertAttrAtIndex
#ifdef ANSI_PROTO
	( SdpMessage 	*msg,
	  SdpAttr	*slAttr,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( msg,slAttr,cnt,err)
	  SdpMessage 	*msg;
	  SdpAttr	*slAttr;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
{
	SdpAttr 	*element_in_list;

	SIPDEBUGFN("Entering InsertAttrAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the slAttr structure */
	if ( slAttr == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if(sip_initSdpAttr(&element_in_list,err)==SipFail)
			return SipFail;


		if ( __sip_cloneSdpAttr(element_in_list, slAttr,err) == SipFail)
		{
			sip_freeSdpAttr(element_in_list);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slAttr->dRefCount);
		element_in_list = (SdpAttr *)slAttr;
#endif
	}

	if( sip_listInsertAt(&(msg->slAttr), cnt, element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( element_in_list != SIP_NULL )
			sip_freeSdpAttr(element_in_list);
#else
		HSS_LOCKEDDECREF(slAttr->dRefCount);
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting InsertAttrAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************
**
** FUNCTION:  sdp_deleteAttrAtIndex
**
** DESCRIPTION: This function deletes an attrubute at a specified
**		index in an SDP message structure
**
******************************************************************/
SipBool sdp_deleteAttrAtIndex
#ifdef ANSI_PROTO
	( SdpMessage 	*msg,
	  SIP_U32bit 	cnt,
	  SipError 	*err )
#else
	( msg,cnt,err)
	  SdpMessage 	*msg;
	  SIP_U32bit 	cnt;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN("Entering deleteAttrAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(msg->slAttr), cnt, err) == SipFail)
		return SipFail;


	SIPDEBUGFN("Exiting deleteAttrAtIndex\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:  sdp_getNetTypeFromConnection
**
**********************************************************************
**
** DESCRIPTION:Gets the net dType field pValue from slConnection structure
**
*********************************************************************/
SipBool sdp_getNetTypeFromConnection
#ifdef ANSI_PROTO
	( SdpConnection	*slConnection,
	  SIP_S8bit 	**ntype,
	  SipError 	*err )
#else
	( slConnection, ntype, err)
	  SdpConnection	*slConnection;
	  SIP_S8bit 	**ntype;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_net_type;

	SIPDEBUGFN( "Entering getNetTypeFromConnection");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slConnection == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( ntype == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_net_type = slConnection->pNetType;

	if( temp_net_type == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_net_type);
	*ntype = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *ntype == SIP_NULL )
		return SipFail;

	strcpy( *ntype , temp_net_type );
#else
	*ntype = temp_net_type;
#endif

	SIPDEBUGFN ( "Exiting getNetTypeFromConnection");

	*err = E_NO_ERROR;
	return SipSuccess;

}



/*********************************************************************
** FUNCTION:  sdp_setNetTypeInConnection
**
**********************************************************************
**
** DESCRIPTION: Sets the net dType field pValue in slConnection structure.
**
*********************************************************************/
SipBool sdp_setNetTypeInConnection
#ifdef ANSI_PROTO
	( SdpConnection	*slConnection,
	  SIP_S8bit 	*ntype,
	  SipError 	*err )
#else
	( slConnection, ntype, err)
	  SdpConnection	*slConnection;
	  SIP_S8bit 	*ntype;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_net_type;

	SIPDEBUGFN ( "Entering setNetTypeInConnection");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slConnection == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( ntype == SIP_NULL)
		temp_net_type = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( ntype );
		temp_net_type = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_net_type == SIP_NULL )
			return SipFail;

		strcpy( temp_net_type, ntype );
#else
		temp_net_type = ntype;
#endif

	}

	if ( slConnection->pNetType != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slConnection->pNetType)), err) == SipFail)
			return SipFail;
	}


	slConnection->pNetType = temp_net_type;

	SIPDEBUGFN ( "Exiting setNetTypeInConnection");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sdp_getAddrTypeFromConnection
**
**********************************************************************
**
** DESCRIPTION: Gets the dAddr dType field pValue from slConnection
**		structure.
**
*********************************************************************/
SipBool sdp_getAddrTypeFromConnection
#ifdef ANSI_PROTO
	( SdpConnection	*slConnection,
	  SIP_S8bit 	**atype,
	  SipError 	*err )
#else
	( slConnection, atype, err)
	  SdpConnection	*slConnection;
	  SIP_S8bit 	**atype;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_addr_type;

	SIPDEBUGFN ( "Entering getAddrTypeFromConnection");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slConnection == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( atype == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_addr_type = slConnection->pAddrType;

	if( temp_addr_type == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_addr_type);
	*atype = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *atype == SIP_NULL )
		return SipFail;

	strcpy( *atype , temp_addr_type );
#else
	*atype = temp_addr_type;
#endif

	SIPDEBUGFN ( "Exiting getAddrTypeFromConnection");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sdp_setAddrTypeInConnection
**
**********************************************************************
**
** DESCRIPTION: Sets the dAddr dType field pValue in slConnection structure.
**
*********************************************************************/
SipBool sdp_setAddrTypeInConnection
#ifdef ANSI_PROTO
	( SdpConnection	*slConnection,
	  SIP_S8bit 	*atype,
	  SipError 	*err )
#else
	( slConnection, atype, err)
	  SdpConnection	*slConnection;
	  SIP_S8bit 	*atype;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_addr_type;

	SIPDEBUGFN ( "Entering setAddrTypeInConnection");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slConnection == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( atype == SIP_NULL)
		temp_addr_type = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( atype );
		temp_addr_type = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_addr_type == SIP_NULL )
			return SipFail;

		strcpy( temp_addr_type, atype );
#else
		temp_addr_type = atype;
#endif
	}

	if ( slConnection->pAddrType != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slConnection->pAddrType)), err) == SipFail)
			return SipFail;
	}


	slConnection->pAddrType = temp_addr_type;

	SIPDEBUGFN ( "Exiting setAddrTypeInConnection");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sdp_getStartFromTime
**
**********************************************************************
**
** DESCRIPTION: Gets the pStart slTime field pValue from slTime structure.
**
*********************************************************************/
SipBool sdp_getStartFromTime
#ifdef ANSI_PROTO
	( SdpTime	*slTime,
	  SIP_S8bit 	**pStart,
	  SipError 	*err )
#else
	( slTime, pStart, err)
	  SdpTime	*slTime;
	  SIP_S8bit 	**pStart;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_start;

	SIPDEBUGFN ( "Entering getStartFromTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slTime == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (  pStart == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_start = slTime->pStart;

	if( temp_start == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_start);
	*pStart = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pStart == SIP_NULL )
		return SipFail;

	strcpy( *pStart , temp_start );
#else
	*pStart = temp_start;
#endif
	SIPDEBUGFN ( "Exiting getStartFromTime");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sdp_setStartInTime
**
**********************************************************************
**
** DESCRIPTION: Sets the pStart slTime field pValue in slTime structure.
**
*********************************************************************/
SipBool sdp_setStartInTime
#ifdef ANSI_PROTO
	( SdpTime	*slTime,
	  SIP_S8bit 	*pStart,
	  SipError 	*err )
#else
	( slTime, pStart, err)
	  SdpTime	*slTime;
	  SIP_S8bit 	*pStart;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_start;

	SIPDEBUGFN ( "Entering setStartInTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slTime == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pStart == SIP_NULL)
		temp_start = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pStart );
		temp_start = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_start == SIP_NULL )
			return SipFail;

		strcpy( temp_start, pStart );
#else
		temp_start = pStart;
#endif
	}

	if ( slTime->pStart != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slTime->pStart)), err) == SipFail)
			return SipFail;
	}


	slTime->pStart = temp_start;

	SIPDEBUGFN ( "Exiting setStartInTime");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sdp_getStopFromTime
**
**********************************************************************
**
** DESCRIPTION: Gets the pStop field pValue from slTime structure.
**
*********************************************************************/
SipBool sdp_getStopFromTime
#ifdef ANSI_PROTO
	( SdpTime	*slTime,
	  SIP_S8bit 	**pStop,
	  SipError 	*err )
#else
	( slTime, pStop, err)
	  SdpTime	*slTime;
	  SIP_S8bit 	**pStop;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_stop;

	SIPDEBUGFN ( "Entering getStopFromTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slTime == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pStop == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_stop = slTime->pStop;

	if( temp_stop == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_stop);
	*pStop = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pStop == SIP_NULL )
		return SipFail;

	strcpy( *pStop , temp_stop );
#else
	*pStop = temp_stop;
#endif
	SIPDEBUGFN ( "Exiting getStopFromTime");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setStopInTime
**
**********************************************************************
**
** DESCRIPTION: Sets the pStop field pValue in slTime structure.
**
*********************************************************************/
SipBool sdp_setStopInTime
#ifdef ANSI_PROTO
	( SdpTime	*slTime,
	  SIP_S8bit 	*pStop,
	  SipError 	*err )
#else
	( slTime, pStop, err)
	  SdpTime	*slTime;
	  SIP_S8bit 	*pStop;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_stop;

	SIPDEBUGFN ( "Entering setStopInTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slTime == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pStop == SIP_NULL)
		temp_stop = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pStop );
		temp_stop = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_stop == SIP_NULL )
			return SipFail;

		strcpy( temp_stop, pStop );
#else
		temp_stop = pStop;
#endif
	}

	if ( slTime->pStop != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slTime->pStop)), err) == SipFail)
			return SipFail;
	}


	slTime->pStop = temp_stop;

	SIPDEBUGFN ( "Exiting setStopInTime");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_getRepeatCountFromTime
**
**********************************************************************
**
** DESCRIPTION: Gets the number of slRepeat nodes present in slTime
**		structure.
**
*********************************************************************/
SipBool sdp_getRepeatCountFromTime
#ifdef ANSI_PROTO
	( SdpTime	*slTime,
	  SIP_U32bit	*index,
	  SipError	*err  )
#else
	( slTime,index,err)
	  SdpTime 	*slTime;
	  SIP_U32bit 	*index;
	  SipError 	*err;
#endif
{

	SIPDEBUGFN ( "Entering GetRepeatCountFromTime");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if ( (slTime == SIP_NULL) || ( index == SIP_NULL ))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(slTime->slRepeat), index , err) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting GetRepeatCountFromTime");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_getRepeatAtIndexFromTime
**
**********************************************************************
**
** DESCRIPTION: Gets the slRepeat pValue at a specified index ( starting
**		from 0 ) from slTime strucutre.
**
*********************************************************************/
SipBool sdp_getRepeatAtIndexFromTime
#ifdef ANSI_PROTO
	( SdpTime 	*slTime,
	  SIP_S8bit 	**slRepeat,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slTime,slRepeat,index,err)
	  SdpTime *slTime;
	  SIP_S8bit **slRepeat;
	  SIP_U32bit index;
	  SipError *err;
#endif
{
	SIP_Pvoid element_from_list;
#ifndef SIP_BY_REFERENCE
	SIP_U32bit size;
#endif

	SIPDEBUGFN ( "Entering GetRepeatAtIndexFromTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (slTime == SIP_NULL) || (slRepeat == SIP_NULL) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(slTime->slRepeat), index, &element_from_list, err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	size = strlen( (SIP_S8bit * )element_from_list);
	*slRepeat = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID, size +1, err);
	if(*slRepeat == SIP_NULL)
		return SipFail;

	strcpy(*slRepeat, (SIP_S8bit*)element_from_list);
#else
	*slRepeat = (SIP_S8bit *) element_from_list;
#endif

	SIPDEBUGFN ( "Exiting GetRepeatAtIndexFromTime");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:  sdp_setRepeatAtIndexInTime
**
**********************************************************************
**
** DESCRIPTION: Sets the slRepeat pValue at a specified index ( starting
**		from 0 )in slTime strucutre.
**
*********************************************************************/
SipBool sdp_setRepeatAtIndexInTime
#ifdef ANSI_PROTO
	( SdpTime 	*slTime,
	  SIP_S8bit 	*slRepeat,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slTime,slRepeat,index,err)
	  SdpTime 	*slTime;
	  SIP_S8bit 	*slRepeat;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
{

	SIP_S8bit * element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;		/* used in freeing memory after an error has happened */
#endif

	SIPDEBUGFN ( "Entering SetRepeatAtIndexInTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (slTime == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( slRepeat == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,strlen(slRepeat) + 1, err);
		if( element_in_list == SIP_NULL )
			return SipFail;

		strcpy(element_in_list, slRepeat);
#else
		element_in_list = slRepeat;
#endif
	}

	if( sip_listSetAt(&(slTime->slRepeat), index, element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( element_in_list != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(element_in_list)), &temp_err);
#endif
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting SetRepeatAtIndexinTime");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_insertRepeatAtIndexInTime
**
**********************************************************************
**
** DESCRIPTION: Inserts a slRepeat pValue at a specified index ( starting
**		from 0 )  in slTime strucutre.
**
*********************************************************************/
SipBool sdp_insertRepeatAtIndexInTime
#ifdef ANSI_PROTO
	( SdpTime 	*slTime,
	  SIP_S8bit 	*slRepeat,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slTime,slRepeat,index,err)
	  SdpTime 	*slTime;
	  SIP_S8bit 	*slRepeat;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
{

	SIP_S8bit *element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_err;		/* used in freeing memory after an error has happened */
#endif


	SIPDEBUGFN ( "Entering InsertRepeatAtIndexInTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slTime == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the slRepeat structure/char*  */
	if ( slRepeat == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,strlen(slRepeat) + 1, err);
		if( element_in_list == SIP_NULL )
			return SipFail;

		strcpy(element_in_list, slRepeat);
#else
		element_in_list = slRepeat;
#endif
	}

	if( sip_listInsertAt(&(slTime->slRepeat), index, element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( element_in_list != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(element_in_list)), &temp_err);
#endif
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting InsertRepeatAtIndexinTime");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_deleteRepeatAtIndexInTime
**
**********************************************************************
**
** DESCRIPTION: Deletes a slRepeat pValue at a specified index ( starting
**		from 0 )  in slTime strucutre.
**
*********************************************************************/
SipBool sdp_deleteRepeatAtIndexInTime
#ifdef ANSI_PROTO
	( SdpTime 	*slTime,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slTime,index,err)
	  SdpTime 	*slTime;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
{

	SIPDEBUGFN ( "Entering DeleteRepeatAtIndexInTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slTime == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(slTime->slRepeat), index, err) == SipFail)
		return SipFail;

	SIPDEBUGFN ( "Exiting DeleteRepeatAtIndexinTime");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_getZoneFromTime
**
**********************************************************************
**
** DESCRIPTION: Gets the pZone pValue from slTime strucutre.
**
*********************************************************************/
SipBool sdp_getZoneFromTime
#ifdef ANSI_PROTO
	( SdpTime	*slTime,
	  SIP_S8bit 	**pZone,
	  SipError 	*err )
#else
	( slTime, pZone, err)
	  SdpTime	*slTime;
	  SIP_S8bit 	**pZone;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_zone;

	SIPDEBUGFN ( "Entering getZoneFromTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slTime == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (  pZone == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_zone = slTime->pZone;

	if( temp_zone == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_zone);
	*pZone = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pZone == SIP_NULL )
		return SipFail;

	strcpy( *pZone , temp_zone );
#else
	*pZone = temp_zone;
#endif
	SIPDEBUGFN ( "Exiting getZoneFromTime");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setZoneInTime
**
**********************************************************************
**
** DESCRIPTION: Sets the pZone pValue in slTime strucutre.
**
*********************************************************************/
SipBool sdp_setZoneInTime
#ifdef ANSI_PROTO
	( SdpTime	*slTime,
	  SIP_S8bit 	*pZone,
	  SipError 	*err )
#else
	( slTime, pZone, err)
	  SdpTime	*slTime;
	  SIP_S8bit 	*pZone;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_zone;

	SIPDEBUGFN ( "Entering setZoneInTime");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slTime == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pZone == SIP_NULL)
		temp_zone = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pZone );
		temp_zone = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_zone == SIP_NULL )
			return SipFail;

		strcpy( temp_zone, pZone );
#else
		temp_zone = pZone;
#endif
	}

	if ( slTime->pZone != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slTime->pZone)), err) == SipFail)
			return SipFail;
	}


	slTime->pZone = temp_zone;

	SIPDEBUGFN ( "Exiting setZoneInTime");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sdp_getAddrFromConnection
**
**********************************************************************
**
** DESCRIPTION: Gets the dAddr field pValue from slConnection strucutre.
**
*********************************************************************/
SipBool sdp_getAddrFromConnection
#ifdef ANSI_PROTO
	( SdpConnection	*slConnection,
	  SIP_S8bit 	**dAddr,
	  SipError 	*err )
#else
	( slConnection, dAddr, err)
	  SdpConnection	*slConnection;
	  SIP_S8bit 	**dAddr;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_addr;

	SIPDEBUGFN ( "Entering getAddrFromConnection");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slConnection == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( dAddr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_addr = slConnection->dAddr;

	if( temp_addr == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_addr);
	*dAddr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *dAddr == SIP_NULL )
		return SipFail;

	strcpy( *dAddr , temp_addr );
#else
	*dAddr = temp_addr;
#endif
	SIPDEBUGFN ( "Exiting getAddrFromConnection");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setAddrInConnection
**
**********************************************************************
**
** DESCRIPTION: Sets the dAddr field pValue in slConnection strucutre.
**
*********************************************************************/
SipBool sdp_setAddrInConnection
#ifdef ANSI_PROTO
	( SdpConnection	*slConnection,
	  SIP_S8bit 	*dAddr,
	  SipError 	*err )
#else
	( slConnection, dAddr, err)
	  SdpConnection	*slConnection;
	  SIP_S8bit 	*dAddr;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_addr;

	SIPDEBUGFN ( "Entering setAddrInConnection");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slConnection == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( dAddr == SIP_NULL)
		temp_addr = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( dAddr );
		temp_addr = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_addr == SIP_NULL )
			return SipFail;

		strcpy( temp_addr, dAddr );
#else
		temp_addr = dAddr;
#endif
	}

	if ( slConnection->dAddr != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(slConnection->dAddr)), err) == SipFail)
			return SipFail;
	}


	slConnection->dAddr = temp_addr;

	SIPDEBUGFN ( "Exiting setAddrInConnection");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_getNameFromAttr
**
**********************************************************************
**
** DESCRIPTION: Gets the pName field pValue from attribute strucutre.
**
*********************************************************************/
SipBool sdp_getNameFromAttr
#ifdef ANSI_PROTO
	( SdpAttr	*slAttr,
	  SIP_S8bit 	**pName,
	  SipError 	*err )
#else
	( slAttr, pName, err)
	  SdpAttr	*slAttr;
	  SIP_S8bit 	**pName;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_name;

	SIPDEBUGFN ( "Entering getNameFromAttr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slAttr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pName == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_name = slAttr->pName;

	if( temp_name == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_name);
	*pName = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pName == SIP_NULL )
		return SipFail;

	strcpy( *pName , temp_name );
#else
	*pName = temp_name;
#endif
	SIPDEBUGFN ( "Exiting getNameFromAttr");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setNameInAttr
**
**********************************************************************
**
** DESCRIPTION: Sets the pName field pValue in attribute strucutre.
**
*********************************************************************/
SipBool sdp_setNameInAttr
#ifdef ANSI_PROTO
	( SdpAttr	*slAttr,
	  SIP_S8bit 	*pName,
	  SipError 	*err )
#else
	( slAttr, pName, err)
	  SdpAttr	*slAttr;
	  SIP_S8bit 	*pName;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_name;

	SIPDEBUGFN ( "Entering setNameInAttr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slAttr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pName == SIP_NULL)
		temp_name = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pName );
		temp_name = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_name == SIP_NULL )
			return SipFail;

		strcpy( temp_name, pName );
#else
		temp_name = pName;
#endif
	}

	if ( slAttr->pName != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(slAttr->pName)), err) == SipFail)
			return SipFail;
	}


	slAttr->pName = temp_name;

	SIPDEBUGFN ( "Exiting setNameInAttr");

	*err = E_NO_ERROR;
	return SipSuccess;

}
/*********************************************************************
** FUNCTION:  sdp_getValueFromAttr
**
**********************************************************************
**
** DESCRIPTION: Gets the pValue field from sdp attribute structure.
**
*********************************************************************/
SipBool sdp_getValueFromAttr
#ifdef ANSI_PROTO
	( SdpAttr	*slAttr,
	  SIP_S8bit 	**pValue,
	  SipError 	*err )
#else
	( slAttr, pValue, err)
	  SdpAttr	*slAttr;
	  SIP_S8bit 	**pValue;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_value;

	SIPDEBUGFN ( "Entering getValueFromAttr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slAttr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_value = slAttr->pValue;

	if( temp_value == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_value);
	*pValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pValue == SIP_NULL )
		return SipFail;

	strcpy( *pValue , temp_value );
#else
	*pValue = temp_value;
#endif
	SIPDEBUGFN ( "Exiting getValueFromAttr");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setValueInAttr
**
**********************************************************************
**
** DESCRIPTION: Sets the pValue field in sdp attribute structure.
**
*********************************************************************/
SipBool sdp_setValueInAttr
#ifdef ANSI_PROTO
	( SdpAttr	*slAttr,
	  SIP_S8bit 	*pValue,
	  SipError 	*err )
#else
	( slAttr, pValue, err)
	  SdpAttr	*slAttr;
	  SIP_S8bit 	*pValue;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_value;

	SIPDEBUGFN ( "Entering setValueInAttr");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slAttr == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pValue == SIP_NULL)
		temp_value = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pValue );
		temp_value = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_value == SIP_NULL )
			return SipFail;

		strcpy( temp_value, pValue );
#else
		temp_value = pValue;
#endif
	}

	if ( slAttr->pValue != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slAttr->pValue)), err) == SipFail)
			return SipFail;
	}


	slAttr->pValue = temp_value;

	SIPDEBUGFN ( "Exiting setValueInAttr");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sdp_getInfoFromMedia
**
**********************************************************************
**
** DESCRIPTION: Gets the pInformation field pValue from sdp slMedia
** structure.
**
*********************************************************************/
SipBool sdp_getInfoFromMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	**pInformation,
	  SipError 	*err )
#else
	( slMedia, pInformation, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	**pInformation;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_info;

	SIPDEBUGFN ( "Entering getInfoFromMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pInformation == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_info = slMedia->pInformation;

	if( temp_info == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_info);
	*pInformation = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pInformation == SIP_NULL )
		return SipFail;

	strcpy( *pInformation , temp_info );
#else
	*pInformation = temp_info;
#endif

	SIPDEBUGFN ( "Exiting getInfoFromMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setInfoInMedia
**
**********************************************************************
**
** DESCRIPTION: Sets the pInformation field pValue in sdp slMedia
** structure.
**
*********************************************************************/
SipBool sdp_setInfoInMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	*pInformation,
	  SipError 	*err )
#else
	( slMedia, pInformation, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	*pInformation;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_info;

	SIPDEBUGFN ( "Entering setInfoInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pInformation == SIP_NULL)
		temp_info = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pInformation );
		temp_info = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_info == SIP_NULL )
			return SipFail;

		strcpy( temp_info, pInformation );
#else
		temp_info = pInformation;
#endif
	}

	if ( slMedia->pInformation != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slMedia->pInformation)), err) == SipFail)
			return SipFail;
	}


	slMedia->pInformation = temp_info;

	SIPDEBUGFN ( "Exiting setInfoInMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sdp_getBandwidthCountFromMedia
**
** DESCRIPTION: This function retrieves the Bandwidth count from an
**		SDP media line structure
**
**********************************************************************/

SipBool sdp_getBandwidthCountFromMedia
#ifdef ANSI_PROTO
	( SdpMedia *pMedia, SIP_U32bit *pBWidthCount, SipError *pErr )
#else
	( pMedia,pBWidthCount,pErr)
	  SdpMedia *pMedia;
	  SIP_U32bit *pBWidthCount;
	  SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sdp_getBandwidthCountFromMedia\n");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if( (pMedia == SIP_NULL) || ( pBWidthCount == SIP_NULL ) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if (sip_listSizeOf(&(pMedia->slBandwidth), pBWidthCount , pErr) == \
						SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN("Exiting sdp_getBandwidthCountFromMedia\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sdp_getBandwidthAtIndexFromMedia
**
** DESCRIPTION: This function retrieves the pBandwidth field at a
**		specified index in an SDP media line structure
**
*******************************************************************/
SipBool sdp_getBandwidthAtIndexFromMedia
#ifdef ANSI_PROTO
	( SdpMedia *pMedia, SIP_S8bit **ppBandwidth, SIP_U32bit dIndex, \
		SipError *pErr )
#else
	( pMedia,ppBandwidth,dIndex,pErr)
	  SdpMedia *pMedia; 
	  SIP_S8bit **ppBandwidth;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{
	SIP_Pvoid element_from_list;

	SIPDEBUGFN("Entering sdp_getBandwidthAtIndexFromMedia\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pMedia == SIP_NULL) || (ppBandwidth == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( sip_listGetAt(&(pMedia->slBandwidth), dIndex, &element_from_list, \
								pErr) == SipFail)
	{	
		return SipFail;
	}	

	if ( element_from_list == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	*ppBandwidth = (SIP_S8bit *) STRDUPACCESSOR((SIP_S8bit *)element_from_list);
	if (*ppBandwidth == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppBandwidth = (SIP_S8bit *) element_from_list;
#endif

	SIPDEBUGFN("Exiting sdp_getBandwidthAtIndexFromMedia\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sdp_setBandwidthAtIndexInMedia
**
** DESCRIPTION: This function sets the pBandwidth field at a specified
**		index in an SDP media line.
**
**********************************************************************/
SipBool sdp_setBandwidthAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia *pMedia, SIP_S8bit *pBandwidth, SIP_U32bit dIndex, \
		SipError *pErr )
#else
	( pMedia,pBandwidth,dIndex,pErr)
	  SdpMedia *pMedia;
	  SIP_S8bit *pBandwidth;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SIP_S8bit * element_in_list;
#ifndef SIP_BY_REFERENCE
	SipError temp_pErr;
#endif
	SIPDEBUGFN("Enterring setBandwidthAtIndexInMedia\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;


	if ( pMedia == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pBandwidth == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *)STRDUPACCESSOR(pBandwidth);
		if (element_in_list == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = pBandwidth;
#endif
	}

	if( sip_listSetAt( &(pMedia->slBandwidth),dIndex, \
		    		(SIP_Pvoid)element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (fast_memfree (0, element_in_list, &temp_pErr) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting setBandwidthAtIndexInMedia\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***************************************************************************
** FUNCTION: sdp_insertBandwidthAtIndexInMedia
**
** DESCRIPTION: This function inserts a pBandwidth field at a specified
**		index in an SDP media line structure
**
****************************************************************************/
SipBool sdp_insertBandwidthAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia *pMedia, SIP_S8bit *pBandwidth, SIP_U32bit dIndex, \
		SipError *pErr )
#else
	( pMedia,pBandwidth,dIndex,pErr)
	  SdpMedia *pMedia;
	  SIP_S8bit *pBandwidth;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SIP_S8bit * element_in_list;
	SipError temp_err;

	SIPDEBUGFN("Entering insertBandwidthAtIndexInMedia\n");
	temp_err = E_NO_ERROR;
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pMedia == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pBandwidth == SIP_NULL)
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		element_in_list = (SIP_S8bit *) STRDUPACCESSOR(pBandwidth);
		if (element_in_list == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
#else
		element_in_list = pBandwidth;
#endif
	}

	if( sip_listInsertAt( &(pMedia->slBandwidth),dIndex , \
							(SIP_Pvoid) element_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if (fast_memfree(ACCESSOR_MEM_ID, element_in_list, &temp_err) == SipFail)
			return SipFail;
#endif
		return SipFail;
	}

	SIPDEBUGFN("Exiting insertBandwidthAtIndexInMedia\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_deleteBandwidthAtIndexInMedia
**
** DESCRIPTION: This function deletes a pBandwidth field at a specified
**		index in an SDP media line structure
**
**********************************************************************/

SipBool sdp_deleteBandwidthAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia *pMedia, SIP_U32bit dIndex, SipError *pErr )
#else
	( pMedia,dIndex,pErr)
	  SdpMedia *pMedia;
	  SIP_U32bit dIndex;
	  SipError *pErr;
#endif
{

	SIPDEBUGFN("Entering deleteBandwidthAtIndexInMedia\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMedia == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(pMedia->slBandwidth), dIndex, pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN("Exiting deleteBandwidthAtIndexInMedia\n");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
 
/*********************************************************************
** FUNCTION:  sdp_getKeyFromMedia
**	.
**********************************************************************
**
** DESCRIPTION: Gets the pKey field pValue from sdp slMedia structure
**
*********************************************************************/
SipBool sdp_getKeyFromMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	**pKey,
	  SipError 	*err )
#else
	( slMedia, pKey, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	**pKey;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_key;

	SIPDEBUGFN ( "Entering getKeyFromMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pKey == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_key = slMedia->pKey;

	if( temp_key == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_key);
	*pKey = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pKey == SIP_NULL )
		return SipFail;

	strcpy( *pKey , temp_key );
#else
	*pKey = temp_key;
#endif
	SIPDEBUGFN ( "Exiting getKeyFromMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setKeyInMedia
**
**********************************************************************
**
** DESCRIPTION: Sets the pKey field pValue in sdp slMedia structure.
**
*********************************************************************/
SipBool sdp_setKeyInMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	*pKey,
	  SipError 	*err )
#else
	( slMedia, pKey, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	*pKey;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_key;

	SIPDEBUGFN ( "Entering setKeyInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pKey == SIP_NULL)
		temp_key = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pKey );
		temp_key = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_key == SIP_NULL )
			return SipFail;

		strcpy( temp_key, pKey );
#else
		temp_key = pKey;
#endif
	}

	if ( slMedia->pKey != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slMedia->pKey)), err) == SipFail)
			return SipFail;
	}


	slMedia->pKey = temp_key;

	SIPDEBUGFN ( "Exiting setKeyInMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_getMvalueFromMedia
**
**********************************************************************
**
** DESCRIPTION: Gets the  m pValue from sdp slMedia structure.
**
*********************************************************************/
SipBool sdp_getMvalueFromMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	**pMediaValue,
	  SipError 	*err )
#else
	( slMedia, pMediaValue, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	**pMediaValue;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_mvalue;

	SIPDEBUGFN ( "Entering getMvalueFromMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pMediaValue == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_mvalue = slMedia->pMediaValue;

	if( temp_mvalue == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_mvalue);
	*pMediaValue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pMediaValue == SIP_NULL )
		return SipFail;

	strcpy( *pMediaValue , temp_mvalue );
#else
	*pMediaValue = temp_mvalue;
#endif
	SIPDEBUGFN ( "Exiting getMvalueFromMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setMvalueInMedia
**
**********************************************************************
**
** DESCRIPTION: Sets the m pValue in sdp slMedia structure.
**
*********************************************************************/
SipBool sdp_setMvalueInMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	*pMediaValue,
	  SipError 	*err )
#else
	( slMedia, pMediaValue, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	*pMediaValue;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_mvalue;

	SIPDEBUGFN ( "Entering setMvalueInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pMediaValue == SIP_NULL)
		temp_mvalue = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pMediaValue );
		temp_mvalue = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_mvalue == SIP_NULL )
			return SipFail;

		strcpy( temp_mvalue, pMediaValue );
#else
		temp_mvalue = pMediaValue;
#endif

	}

	if ( slMedia->pMediaValue != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slMedia->pMediaValue)), err) == SipFail)
			return SipFail;
	}


	slMedia->pMediaValue = temp_mvalue;

	SIPDEBUGFN ( "Exiting setMvalueInMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sdp_getFormatFromMedia
**
**********************************************************************
**
** DESCRIPTION: Gets the fromat field pValue from sdp slMedia structure.
**
*********************************************************************/
SipBool sdp_getFormatFromMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	**pFormat,
	  SipError 	*err )
#else
	( slMedia, pFormat, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	**pFormat;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_format;

	SIPDEBUGFN ( "Entering getFormatFromMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pFormat == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_format = slMedia->pFormat;

	if( temp_format == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_format);
	*pFormat = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pFormat == SIP_NULL )
		return SipFail;

	strcpy( *pFormat , temp_format );
#else
	*pFormat = temp_format;
#endif

	SIPDEBUGFN ( "Exiting getFormatFromMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setFormatInMedia
**
**********************************************************************
**
** DESCRIPTION: Sets the pFormat field pValue in sdp slMedia structure.
**
*********************************************************************/
SipBool sdp_setFormatInMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	*pFormat,
	  SipError 	*err )
#else
	( slMedia, pFormat, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	*pFormat;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_format;

	SIPDEBUGFN ( "Entering setFormatInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pFormat == SIP_NULL)
		temp_format = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pFormat );
		temp_format = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_format == SIP_NULL )
			return SipFail;

		strcpy( temp_format, pFormat );
#else
		temp_format = pFormat;
#endif
	}

	if ( slMedia->pFormat != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slMedia->pFormat)), err) == SipFail)
			return SipFail;
	}


	slMedia->pFormat = temp_format;

	SIPDEBUGFN ( "Exiting setFormatInMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_getProtoFromMedia
**
**********************************************************************
**
** DESCRIPTION: Gets the pProtocol field pValue from sdp slMedia
** structure.
**
*********************************************************************/
SipBool sdp_getProtoFromMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	**pProtocol,
	  SipError 	*err )
#else
	( slMedia, pProtocol, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	**pProtocol;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_proto;

	SIPDEBUGFN ( "Entering getProtoFromMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pProtocol == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_proto = slMedia->pProtocol;

	if( temp_proto == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	dLength = strlen(temp_proto);
	*pProtocol = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
	if ( *pProtocol == SIP_NULL )
		return SipFail;

	strcpy( *pProtocol , temp_proto );
#else
	*pProtocol = temp_proto;
#endif
	SIPDEBUGFN ( "Exiting getProtoFromMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setProtoInMedia
**
**********************************************************************
**
** DESCRIPTION: Sets the pProtocol field pValue in sdp slMedia
** structure.
**
*********************************************************************/
SipBool sdp_setProtoInMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_S8bit 	*pProtocol,
	  SipError 	*err )
#else
	( slMedia, pProtocol, err)
	  SdpMedia	*slMedia;
	  SIP_S8bit 	*pProtocol;
	  SipError 	*err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_proto;

	SIPDEBUGFN ( "Entering setProtoInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( pProtocol == SIP_NULL)
		temp_proto = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pProtocol );
		temp_proto = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,err);
		if ( temp_proto == SIP_NULL )
			return SipFail;

		strcpy( temp_proto, pProtocol );
#else
		temp_proto = pProtocol;
#endif
	}

	if ( slMedia->pProtocol != SIP_NULL )
	{
		if ( sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slMedia->pProtocol)), err) == SipFail)
			return SipFail;
	}


	slMedia->pProtocol = temp_proto;

	SIPDEBUGFN ( "Exiting setProtoInMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_getPortFromMedia
**
**********************************************************************
**
** DESCRIPTION: Gets the dPort field from an SDP slMedia structure.
**
*********************************************************************/
SipBool sdp_getPortFromMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_U16bit 	*dPort,
	  SipError 	*err )
#else
	( slMedia, dPort, err)
	  SdpMedia	*slMedia;
	  SIP_U16bit 	*dPort;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN ( "Entering getPortFromMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (slMedia == SIP_NULL ) || ( dPort == SIP_NULL ))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	*dPort = slMedia->dPort;

	SIPDEBUGFN ( "Exiting getPortFromMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setPortInMedia
**
**********************************************************************
**
** DESCRIPTION: Sets the dPort field pValue in sdp slMedia structure.
**
*********************************************************************/
SipBool sdp_setPortInMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_U16bit 	dPort,
	  SipError 	*err )
#else
	( slMedia, dPort, err)
	  SdpMedia	*slMedia;
	  SIP_U16bit 	dPort;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN ( "Entering setPortInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	slMedia->dPort = dPort;

	SIPDEBUGFN ( "Exiting setPortInMedia");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*********************************************************************
** FUNCTION:  sdp_getNumportFromMedia
**
**********************************************************************
**
** DESCRIPTION: Gets the pPortNum field pValue from sdp slMedia
** structure.
**
*********************************************************************/

SipBool sdp_getNumportFromMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*slMedia,
	  SIP_U32bit 	*pPortNum,
	  SipError 	*err)
#else
	( slMedia, pPortNum, err )
	  SdpMedia *slMedia;
	  SIP_U32bit *pPortNum;
	  SipError *err;
#endif
{
	SIP_U32bit * temp_numport;

	SIPDEBUGFN ( "Entering getNumportFromMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (slMedia == SIP_NULL) || (pPortNum == SIP_NULL) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_numport = slMedia->pPortNum;
	if ( temp_numport == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

	*pPortNum = *temp_numport;
	SIPDEBUGFN ( "Exiting getNumportFromMedia");
	*err = E_NO_ERROR;
	return SipSuccess;

}

/*********************************************************************
** FUNCTION:  sdp_setNumportInMedia
**
**********************************************************************
**
** DESCRIPTION: Sets the pPortNum field pValue in sdp slMedia structure.
**
*********************************************************************/
SipBool sdp_setNumportInMedia
#ifdef ANSI_PROTO
	( SdpMedia *slMedia,
	  SIP_U32bit pPortNum,
	  SipError *err)
#else
	( slMedia, pPortNum, err )
	  SdpMedia *slMedia;
	  SIP_U32bit pPortNum;
	  SipError *err;
#endif
{
	SIP_U32bit * temp_numport;

	SIPDEBUGFN ( "Entering setNumportInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_numport = (SIP_U32bit*)fast_memget(ACCESSOR_MEM_ID, sizeof(SIP_U32bit), err);
	if ( temp_numport == SIP_NULL)
		return SipFail;

	*temp_numport = pPortNum;

	if( slMedia->pPortNum != SIP_NULL )
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(slMedia->pPortNum)),err) == SipFail )
			return SipFail;

	slMedia->pPortNum  = temp_numport;

	SIPDEBUGFN ( "Exiting setNumportInMedia");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_getConnectionCountFromMedia
**
**********************************************************************
**
** DESCRIPTION: Gets the number of slConnection structures present in
** the slMedia strucutre.
**
*********************************************************************/

SipBool sdp_getConnectionCountFromMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_U32bit	*index,
	  SipError	*err  )
#else
	( slMedia,index,err)
	  SdpMedia 	*slMedia;
	  SIP_U32bit 	*index;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN ( "Entering getConnectionCountFromMedia");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if ( (slMedia == SIP_NULL) || ( index == SIP_NULL ))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(slMedia->slConnection), index , err) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting getConnectionCountFromMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_getConnectionAtIndexFromMedia
**	.
**********************************************************************
**
** DESCRIPTION: Gets the slConnection structure at a specified index
**		( starting from 0 ) from the slMedia strucutre
**
*********************************************************************/
SipBool sdp_getConnectionAtIndexFromMedia
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SdpMedia 	*slMedia,
	  SdpConnection *slConnection,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( SdpMedia 	*slMedia,
	  SdpConnection **slConnection,
	  SIP_U32bit 	index,
	  SipError 	*err )
#endif
#else
#ifndef SIP_BY_REFERENCE
	( slMedia,slConnection,index,err)
	  SdpMedia 	*slMedia;
	  SdpConnection *slConnection;
	  SIP_U32bit 	index;
	  SipError 	*err;
#else
	( slMedia,slConnection,index,err)
	  SdpMedia 	*slMedia;
	  SdpConnection **slConnection;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
#endif
{
	SIP_Pvoid 	element_from_list;

	SIPDEBUGFN ( "Entering getConnectionAtIndexFromMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (slMedia == SIP_NULL) || (slConnection == SIP_NULL) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(slMedia->slConnection), index, &element_from_list, err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if ( __sip_cloneSdpConnection(slConnection,(SdpConnection *)element_from_list,err) == SipFail)
	{
		/* this part must be changed if the fields inside slConnection is changed */
		sip_freeString(slConnection->pNetType);
		sip_freeString( slConnection->pAddrType);
		sip_freeString( slConnection->dAddr);
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(((SdpConnection *)element_from_list)->dRefCount);
	*slConnection = (SdpConnection *)element_from_list;
#endif
	SIPDEBUGFN ( "Exiting getConnectionAtIndexFromMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_setConnectionAtIndexInMedia
**
**********************************************************************
**
** DESCRIPTION: Sets the slConnection structure at a specified index
**		( starting from 0 ) in the slMedia strucutre.
**
*********************************************************************/
SipBool sdp_setConnectionAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*slMedia,
	  SdpConnection	*slConnection,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slMedia,slConnection,index,err)
	  SdpMedia 	*slMedia;
	  SdpConnection	*slConnection;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
{
	SdpConnection 	*element_in_list;

	SIPDEBUGFN ( "Entering setConnectionAtIndexInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( slConnection == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSdpConnection(&element_in_list, err) == SipFail)
			return SipFail;
		if ( __sip_cloneSdpConnection(element_in_list, slConnection,err) == SipFail)
		{
			sip_freeSdpConnection(element_in_list );
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slConnection->dRefCount);
		element_in_list = (SdpConnection *)slConnection;
#endif
	}

	if( sip_listSetAt(&(slMedia->slConnection), index, (SIP_Pvoid) element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( element_in_list != SIP_NULL )
			 sip_freeSdpConnection(element_in_list );
#else
		HSS_LOCKEDDECREF(slConnection->dRefCount);
#endif
		return SipFail;
	}
	SIPDEBUGFN ( "Exiting setConnectionAtIndexInMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_insertConnectionAtIndexInMedia
**
**********************************************************************
**
** DESCRIPTION: Inserts the slConnection structure at a specified index
**		( starting from 0 ) in the slMedia strucutre.
**
*********************************************************************/
SipBool sdp_insertConnectionAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*slMedia,
	  SdpConnection	*slConnection,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slMedia,slConnection,index,err)
	  SdpMedia 	*slMedia;
	  SdpConnection	*slConnection;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
{
	SdpConnection 	*element_in_list;

	SIPDEBUGFN ( "Entering InsertConnectionAtIndexInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the slConnection structure */
	if ( slConnection == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		/* call init slConnection */
		if ( sip_initSdpConnection(&element_in_list, err) == SipFail)
			return SipFail;

		if ( __sip_cloneSdpConnection(element_in_list, slConnection,err) == SipFail)
		{
			sip_freeSdpConnection(element_in_list);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slConnection->dRefCount);
		element_in_list = (SdpConnection * ) slConnection;
#endif
	}

	if( sip_listInsertAt(&(slMedia->slConnection), index, element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( element_in_list != SIP_NULL )
			sip_freeSdpConnection(element_in_list);
#else
		HSS_LOCKEDDECREF(slConnection->dRefCount);
#endif
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting InsertConnectionAtIndexInMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_deleteConnectionAtIndexInMedia
**
**********************************************************************
**
** DESCRIPTION: Deletes a slConnection structure at a specified index
**		( starting from 0 ) in the slMedia strucutre.
**
*********************************************************************/
SipBool sdp_deleteConnectionAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*slMedia,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slMedia,index,err)
	  SdpMedia 	*slMedia;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN ( "Entering deleteConnectionAtIndexInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(slMedia->slConnection), index, err) == SipFail)
		return SipFail;


	SIPDEBUGFN ( "Exiting deleteConnectionAtIndexInMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************************
** FUNCTION:  sdp_getAttrCountFromMedia
**
**********************************************************************
**
** DESCRIPTION:	Gets the number of attribute structures present in
**		Media Structure.
**
*********************************************************************/
SipBool sdp_getAttrCountFromMedia
#ifdef ANSI_PROTO
	( SdpMedia	*slMedia,
	  SIP_U32bit	*index,
	  SipError	*err  )
#else
	( slMedia,index,err)
	  SdpMedia 	*slMedia;
	  SIP_U32bit 	*index;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN ( "Entering getAttrCounFromMedia");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if ( (slMedia == SIP_NULL) || ( index == SIP_NULL ))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(slMedia->slAttr), index , err) == SipFail )
	{
		return SipFail;
	}
	SIPDEBUGFN ( "Exiting getAttrCountFromMedia");

	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_getAttrAtIndexFromMedia
**
**********************************************************************
**
** DESCRIPTION: Gets the attribute structure at a specified index
**		( starting from 0 ) from the slMedia strucutre.
**
*********************************************************************/
SipBool sdp_getAttrAtIndexFromMedia
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SdpMedia 	*slMedia,
	  SdpAttr *slAttr,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( SdpMedia 	*slMedia,
	  SdpAttr **slAttr,
	  SIP_U32bit 	index,
	  SipError 	*err )
#endif
#else
#ifndef SIP_BY_REFERENCE
	( slMedia,slAttr,index,err)
	  SdpMedia 	*slMedia;
	  SdpAttr *slAttr;
	  SIP_U32bit 	index;
	  SipError 	*err;
#else
	( slMedia,slAttr,index,err)
	  SdpMedia 	*slMedia;
	  SdpAttr **slAttr;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
#endif
{
	SIP_Pvoid 	element_from_list;

	SIPDEBUGFN ( "Entering getAttrAtIndexFromMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( (slMedia == SIP_NULL) || (slAttr == SIP_NULL) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(slMedia->slAttr), index, &element_from_list, err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if ( __sip_cloneSdpAttr(slAttr,(SdpAttr *)element_from_list,err) == SipFail)
	{
		/* this part must be changed if slAttr structure fields are changed */
		sip_freeString(slAttr->pName);
		sip_freeString( slAttr->pValue);
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(((SdpAttr *)element_from_list)->dRefCount);
	*slAttr = (SdpAttr *)element_from_list;
#endif
	SIPDEBUGFN ( "Exiting getAttrAtIndexFromMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_setAttrAtIndexInMedia
**
**********************************************************************
**
** DESCRIPTION: Sets the attribute structure at a specified index
**		( starting from 0 ) in the slMedia strucutre.
**
*********************************************************************/
SipBool sdp_setAttrAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*slMedia,
	  SdpAttr	*slAttr,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slMedia,slAttr,index,err)
	  SdpMedia 	*slMedia;
	  SdpAttr	*slAttr;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
{
	SdpAttr 	*element_in_list;

	SIPDEBUGFN ( "Entering setAttrAtIndexInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (slMedia == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( slAttr == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if ( sip_initSdpAttr(&element_in_list,err)== SipFail)
			return SipFail;
		if ( __sip_cloneSdpAttr(element_in_list, slAttr,err) == SipFail)
		{
			sip_freeSdpAttr(element_in_list);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slAttr->dRefCount);
		element_in_list = slAttr;
#endif
	}

	if( sip_listSetAt(&(slMedia->slAttr), index, (SIP_Pvoid) element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( element_in_list != SIP_NULL )
			sip_freeSdpAttr(element_in_list);
#else
		HSS_LOCKEDDECREF(slAttr->dRefCount);
#endif

		return SipFail;
	}

	SIPDEBUGFN ( "Exiting setAttrAtIndexInMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_insertAttrAtIndexInMedia
**
**********************************************************************
**
** DESCRIPTION: Inserts the attribute structure at a specified index
**		( starting from 0 ) in the slMedia strucutre.
**
*********************************************************************/
SipBool sdp_insertAttrAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*slMedia,
	  SdpAttr	*slAttr,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slMedia,slAttr,index,err)
	  SdpMedia 	*slMedia;
	  SdpAttr	*slAttr;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
{
	SdpAttr 	*element_in_list;

	SIPDEBUGFN ( "Entering insertAttrAtIndexInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the slAttr structure */
	if ( slAttr == SIP_NULL )
		element_in_list = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if ( sip_initSdpAttr(&element_in_list,err) == SipFail)
			return SipFail;
		if ( __sip_cloneSdpAttr(element_in_list, slAttr,err) == SipFail)
		{
			sip_freeSdpAttr(element_in_list);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(slAttr->dRefCount);
		element_in_list = slAttr;
#endif
	}

	if( sip_listInsertAt(&(slMedia->slAttr), index, element_in_list, err) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( element_in_list != SIP_NULL )
			sip_freeSdpAttr(element_in_list);
#else
		HSS_LOCKEDDECREF(slAttr->dRefCount);
#endif
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting insertAttrAtIndexInMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_deleteAttrAtIndexInMedia
**
**********************************************************************
**
** DESCRIPTION: Deletes a attribute structure at a specified index
**		( starting from 0 ) from the slMedia strucutre.
**
*********************************************************************/
SipBool sdp_deleteAttrAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*slMedia,
	  SIP_U32bit 	index,
	  SipError 	*err )
#else
	( slMedia,index,err)
	  SdpMedia 	*slMedia;
	  SIP_U32bit 	index;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN ( "Entering deletAttrAtIndexInMedia");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( slMedia == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(slMedia->slAttr), index, err) == SipFail)
		return SipFail;

	SIPDEBUGFN ( "Exiting deleteAttrAtIndexInMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getLengthFromUnknownMessage
**
** DESCRIPTION: This function retrieves the dLength of an Unknown]
**		message
**
******************************************************************/
SipBool sip_getLengthFromUnknownMessage
#ifdef ANSI_PROTO
	( SipUnknownMessage *msg, SIP_U32bit *dLength, SipError *err)
#else
	( msg, dLength, err )
	  SipUnknownMessage *msg;
	  SIP_U32bit *dLength;
	  SipError *err;
#endif
{
	SIPDEBUGFN ( "Entering getlengthFromUnknownMessage");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if(dLength == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	*dLength = msg->dLength;

	SIPDEBUGFN ( "Exiting getlengthFromUnknownMessage");

	*err = E_NO_ERROR;
	return SipSuccess;

}


/*****************************************************************
**
** FUNCTION:  sip_getBufferFromUnknownMessage
**
** DESCRIPTION: This function is used to retrieves th unknown mesg
**		pBuffer
**
******************************************************************/
SipBool sip_getBufferFromUnknownMessage
#ifdef ANSI_PROTO
	( SipUnknownMessage *msg, SIP_S8bit **pBuffer, SipError *err)
#else
	( msg, pBuffer, err )
	  SipUnknownMessage *msg;
	  SIP_S8bit **pBuffer;
	  SipError *err;
#endif
{
	SIPDEBUGFN ( "Entering getBufferFromUnknownMessage");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( pBuffer == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
#ifndef SIP_BY_REFERENCE
	 *pBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID,msg->dLength+1, err );
	if ( *pBuffer == SIP_NULL )
		return SipFail;

	if ( msg->pBuffer == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
	memcpy(*pBuffer, msg->pBuffer, msg->dLength);
	(*pBuffer)[msg->dLength]='\0';
#else
	*pBuffer = msg->pBuffer;
#endif

	SIPDEBUGFN ( "Exiting getBufferFromUnknownMessage");

	*err = E_NO_ERROR;
	return SipSuccess;
}
/*****************************************************************
**
** FUNCTION:  sip_setBufferInUnknownMessage
**
** DESCRIPTION: This function sets th unknoen message pBuffer in an
**		Unknown message
**
******************************************************************/
SipBool sip_setBufferInUnknownMessage
#ifdef ANSI_PROTO
	( SipUnknownMessage *msg, SIP_S8bit *pBuffer, SIP_U32bit dLength,SipError *err)
#else
	( msg, pBuffer,dLength, err )
	  SipUnknownMessage *msg;
	  SIP_S8bit 		*pBuffer;
	  SIP_U32bit		dLength;
	  SipError 			*err;
#endif
{
	SIPDEBUGFN ( "Entering setBuferInUnknownMessage");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( msg->pBuffer !=SIP_NULL )
		sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(msg->pBuffer)), err);

	msg->dLength = dLength;
#ifndef SIP_BY_REFERENCE
	msg->pBuffer = (SIP_S8bit*)fast_memget(ACCESSOR_MEM_ID, dLength, err);
	if(msg->pBuffer == SIP_NULL)
		return SipFail;

	memcpy( msg->pBuffer, pBuffer, dLength);
#else
	msg->pBuffer = pBuffer;
#endif


	SIPDEBUGFN ( "Exiting setBufferInUnknownMessage");

	*err = E_NO_ERROR;
	return SipSuccess;
}


/*****************************************************************
**
** FUNCTION:  sip_getMsgBodyType
**
** DESCRIPTION: This function retrieves the message pBody dType from
**		a SIP Message pBody
**
******************************************************************/
SipBool sip_getMsgBodyType
#ifdef ANSI_PROTO
	( SipMsgBody	*pBody,
	  en_SipMsgBodyType *dType,
	  SipError *err)
#else
	( pBody, dType, err )
	  SipMsgBody	*pBody;
	  en_SipMsgBodyType *dType;
	  SipError *err;
#endif
{
	SIPDEBUGFN ( "Enter getMsgBodyType");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL)
		return SipFail;

	if ( (dType == SIP_NULL) || ( pBody == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	*dType = pBody->dType;

	*err = E_NO_ERROR;
	SIPDEBUGFN ( "Exiting getMsgBodyType");
	return SipSuccess;

}

/*****************************************************************
**
** FUNCTION:  sip_getUnknownFromMsgBody
**
** DESCRIPTION: This function retrieves the unknown part from a
**		SIp message pBody
**
******************************************************************/
SipBool sip_getUnknownFromMsgBody
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SipMsgBody *msg,
	  SipUnknownMessage *unknown,
	  SipError *err)
#else
	( SipMsgBody *msg,
	  SipUnknownMessage **unknown,
	  SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	( msg, unknown, err )
	  SipMsgBody *msg;
	  SipUnknownMessage *unknown;
	  SipError *err;
#else
	( msg, unknown, err )
	  SipMsgBody *msg;
	  SipUnknownMessage **unknown;
	  SipError *err;
#endif
#endif
{
	SipUnknownMessage * temp_ub;

	SIPDEBUGFN ( "Entering getUnknownFromMsgBody");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( msg->dType != SipUnknownBody )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
	if ( unknown == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	temp_ub = msg->u.pUnknownMessage;

	if( temp_ub == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if(__sip_cloneSipUnknownMessage(unknown, temp_ub, err) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_ub->dRefCount);
	*unknown = temp_ub;
#endif

	SIPDEBUGFN ( "Exiting getUnknownFromMsgBodyHdr");

	*err = E_NO_ERROR;
	return SipSuccess;
}
/*****************************************************************
**
** FUNCTION:  sip_setUnknownInMsgBody
**
** DESCRIPTION: This function sets the unknown message pBody in a
**		SIP message pBody
**
******************************************************************/
SipBool sip_setUnknownInMsgBody
#ifdef ANSI_PROTO
	( SipMsgBody *msg,
	  SipUnknownMessage *unknown,
	  SipError *err)
#else
	( msg, unknown, err )
	  SipMsgBody *msg;
	  SipUnknownMessage *unknown;
	  SipError *err;
#endif
{

	SipUnknownMessage * temp_unknown, *temp_msg;

	SIPDEBUGFN ( "Entering setUnknownInMsgBody");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if ( msg->dType != SipUnknownBody )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif
	if( unknown == SIP_NULL)
		temp_unknown = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipUnknownMessage( &temp_unknown, err) == SipFail)
			return SipFail;
		if ( __sip_cloneSipUnknownMessage(temp_unknown, unknown, err) == SipFail)
		{
			sip_freeSipUnknownMessage(temp_unknown);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(unknown->dRefCount);
		temp_unknown = unknown;
#endif

	}



	temp_msg = msg->u.pUnknownMessage;
	if ( temp_msg != SIP_NULL )
	{
		 sip_freeSipUnknownMessage(temp_msg);
	}

	msg->u.pUnknownMessage = temp_unknown;

	SIPDEBUGFN ( "Exiting setUnknownInMsgBody");

	*err = E_NO_ERROR;
	return SipSuccess;

}

/********************************************************************
** FUNCTION:sip_getSdpFromMsgBody
**
** DESCRIPTION: this function retieves the SDP part of the SIP message
**
**********************************************************************/
SipBool sip_getSdpFromMsgBody
#ifdef ANSI_PROTO
#ifndef SIP_BY_REFERENCE
	( SipMsgBody *msg, SdpMessage *sdp, SipError *err)
#else
	( SipMsgBody *msg, SdpMessage **sdp, SipError *err)
#endif
#else
#ifndef SIP_BY_REFERENCE
	( msg, sdp, err )
	  SipMsgBody *msg;
	  SdpMessage *sdp;
	  SipError *err;
#else
	( msg, sdp, err )
	  SipMsgBody *msg;
	  SdpMessage **sdp;
	  SipError *err;
#endif
#endif
{
	SIPDEBUGFN("Entering getSdpFromMsgBody\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
	{
		return SipFail;
	}

	if ( (msg == SIP_NULL) || (sdp == SIP_NULL ) )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if ( msg->dType != SipSdpBody)
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( msg->u.pSdpMessage == SIP_NULL)
	{
		*err = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if(__sip_cloneSdpMessage(sdp,msg->u.pSdpMessage,err) == SipFail)
	{
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF((msg->u.pSdpMessage)->dRefCount);
	*sdp = (msg->u).pSdpMessage;
#endif

	SIPDEBUGFN("Exiting getSdpFromMsgBody");
	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
** FUNCTION:sdp_setSdpInMsgBody
**
** DESCRIPTION: This function sets the SDP part in a SIP message
**
**********************************************************************/

SipBool sip_setSdpInMsgBody
#ifdef ANSI_PROTO
	( SipMsgBody *msg, SdpMessage *sdp, SipError *err)
#else
	( msg, sdp, err )
	  SipMsgBody *msg;
	  SdpMessage *sdp;
	  SipError *err;
#endif
{
	SdpMessage	*temp_sdp;

	SIPDEBUGFN("Entering sip_setSdpInBody\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
	{
		return SipFail;
	}

	if (msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}

	if( msg->dType!=SipSdpBody )
	{
		*err = E_INV_TYPE;
		return SipFail;
	}
#endif
	if ( msg->u.pSdpMessage != SIP_NULL)
		sip_freeSdpMessage( msg->u.pSdpMessage);

	if (sdp==SIP_NULL)
	{
		temp_sdp=SIP_NULL;	
	}
	else
	{	
#ifndef SIP_BY_REFERENCE
		if ( sip_initSdpMessage(&temp_sdp, err) == SipFail)
			return SipFail;

		if ( __sip_cloneSdpMessage(temp_sdp, sdp, err) == SipFail)
		{
			sip_freeSdpMessage( temp_sdp );
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(sdp->dRefCount);
		temp_sdp = sdp;
#endif
	}	

	msg->u.pSdpMessage = temp_sdp;

	SIPDEBUGFN("Exiting sip_setSdpBody\n");
	*err = E_NO_ERROR;
	return SipSuccess;
}


/*******************************************************************
** FUNCTION: sdp_getIncorrectLineAtIndex
**
** DESCRIPTION: This function retrieves an incorrect line at
**				specified index from an SDP message structure
**
*******************************************************************/
SipBool sdp_getIncorrectLineAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_S8bit **ppIncorrectLine, \
		SIP_U32bit cnt, SipError *err )
#else
	( msg,ppIncorrectLine,cnt,err)
	  SdpMessage *msg;
	  SIP_S8bit **ppIncorrectLine;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{
	SIP_Pvoid element_from_list;

	SIPDEBUGFN("Entering sdp_getIncorrectLineAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (msg == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (ppIncorrectLine == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( sip_listGetAt(&(msg->slIncorrectLines), cnt, &element_from_list, \
		err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	*ppIncorrectLine = (SIP_S8bit *) STRDUPACCESSOR((SIP_S8bit *) \
									element_from_list);
	if (*ppIncorrectLine == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*ppIncorrectLine = (SIP_S8bit *) element_from_list;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting sdp_getIncorrectLineAtIndex\n");
	return SipSuccess;
}


/********************************************************************
** FUNCTION:sdp_deleteIncorrectLineAtIndex
**
** DESCRIPTION: This function deletes an incorrect SDP line (allowed by
**				the application during decode) at the specified index in
**				an SDP message structure
**
**********************************************************************/

SipBool sdp_deleteIncorrectLineAtIndex
#ifdef ANSI_PROTO
	( SdpMessage *msg, SIP_U32bit cnt, SipError *err )
#else
	( msg,cnt,err)
	  SdpMessage *msg;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{
	SIPDEBUGFN("Entering sdp_deleteIncorrectLineAtIndex\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( msg == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(msg->slIncorrectLines), cnt, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting sdp_deleteIncorrectLineAtIndex\n");
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_getIncorrectLineAtIndexFromMedia
**
** DESCRIPTION: Gets the Incorrect line at a specified index
**				(starting from 0) from the slMedia strucutre.
**
*********************************************************************/
SipBool sdp_getIncorrectLineAtIndexFromMedia
#ifdef ANSI_PROTO
	( SdpMedia *media, SIP_S8bit **ppIncorrectLine, \
		SIP_U32bit cnt, SipError *err )
#else
	(media,ppIncorrectLine,cnt,err)
	  SdpMessage *media;
	  SIP_S8bit **ppIncorrectLine;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{
	SIP_Pvoid element_from_list;

	SIPDEBUGFN("Entering sdp_getIncorrectLineAtIndexFromMedia\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if (media == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	if (ppIncorrectLine == SIP_NULL)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( sip_listGetAt(&(media->slIncorrectLines), cnt, &element_from_list, \
		err) == SipFail)
		return SipFail;

	if ( element_from_list == SIP_NULL )
	{
		*err = E_NO_EXIST;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	*ppIncorrectLine = (SIP_S8bit *) STRDUPACCESSOR((SIP_S8bit *) \
									element_from_list);
	if (*ppIncorrectLine == SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
#else
	*ppIncorrectLine = (SIP_S8bit *) element_from_list;
#endif

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting sdp_getIncorrectLineAtIndexFromMedia\n");
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_deleteIncorrectLineAtIndexInMedia
**
** DESCRIPTION: Deletes an incorrect line at a specified index
**				(starting from 0) from the SdpMedia strucutre.
**
*********************************************************************/
SipBool sdp_deleteIncorrectLineAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia *media, SIP_U32bit cnt, SipError *err )
#else
	( media,cnt,err)
	  SdpMedia *media;
	  SIP_U32bit cnt;
	  SipError *err;
#endif
{
	SIPDEBUGFN("Entering sdp_deleteIncorrectLineAtIndexInMedia\n");
#ifndef SIP_NO_CHECK
	if( err == SIP_NULL )
		return SipFail;

	if ( media == SIP_NULL )
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt( &(media->slIncorrectLines), cnt, err) == SipFail)
		return SipFail;

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting sdp_deleteIncorrectLineAtIndexInMedia\n");
	return SipSuccess;
}

#ifdef SIP_ATM
/*********************************************************************
** FUNCTION:  sdp_getConnectionIDFromMedia
**
** DESCRIPTION: Get the ConnectionID from the media line
**
**
*********************************************************************/
SipBool sdp_getConnectionIDFromMedia
#ifdef ANSI_PROTO
	( SdpMedia *pMedia, SIP_S8bit **ppVirtualID, SipError *pErr )
#else
	( pMedia, ppVirtualID, pErr )
	SdpMedia *pMedia;
	SIP_S8bit **ppVirtualID;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering sdp_getConnectionIDFromMedia\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMedia == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if ( ppVirtualID == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	if ( pMedia->pVirtualCID == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	*ppVirtualID = (SIP_S8bit *) STRDUPACCESSOR((SIP_S8bit *) \
									pMedia->pVirtualCID);
	if (*ppVirtualID == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#else
	*ppVirtualID = (SIP_S8bit *) pMedia->pVirtualCID;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting sdp_getConnectionIDFromMedia\n");
	return SipSuccess;
}
/*********************************************************************
** FUNCTION:  sdp_setConnectionIDInMedia
**
** DESCRIPTION: Set the connectionID in the Media line
**
**
*********************************************************************/

SipBool sdp_setConnectionIDInMedia
#ifdef ANSI_PROTO
	( SdpMedia *pMedia, SIP_S8bit *pVirtualID, SipError *pErr )
#else
	( pMedia, pVirtualID, pErr )
	SdpMedia *pMedia;
	SIP_S8bit *pVirtualID;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
#endif
	SIP_S8bit * temp_virtualCID;
	SdpMedia * temp_media;

	SIPDEBUGFN("Entering sdp_setConnectionIDInMedia\n");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMedia == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pVirtualID == SIP_NULL )
		temp_virtualCID = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		dLength = strlen( pVirtualID);
		temp_virtualCID = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( temp_virtualCID == SIP_NULL )
			return SipFail;
		strcpy( temp_virtualCID, pVirtualID );
#else
		temp_virtualCID = pVirtualID;
#endif
	}

	temp_media = ( SdpMedia *) ( pMedia);
	if ( temp_media->pVirtualCID != SIP_NULL )
	{
		if ( fast_memfree(ACCESSOR_MEM_ID, temp_media->pVirtualCID, pErr) == SipFail)
			return SipFail;
	}

	temp_media->pVirtualCID = temp_virtualCID;
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting sdp_setConnectionIDInMedia\n");
	return SipSuccess;
}
/*********************************************************************
** FUNCTION:  sdp_getProtoFmtCountFromMedia
**
** DESCRIPTION: Get the count of Protocol-Format
**					Pair in the Media Line
**
**
*********************************************************************/

SipBool sdp_getProtoFmtCountFromMedia
#ifdef ANSI_PROTO
	( SdpMedia	*pMedia,
	  SIP_U32bit	*count,
	  SipError	*err  )
#else
	( pMedia,count,err)
	  SdpMedia 	*pMedia;
	  SIP_U32bit 	*count;
	  SipError 	*err;
#endif
{
	SIPDEBUGFN ( "Entering sdp_getProtoFmtCountFromMedia");
#ifndef SIP_NO_CHECK
	if ( err == SIP_NULL )
		return SipFail;

	if ( (pMedia == SIP_NULL) || ( count == SIP_NULL ))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pMedia->slProtofmt), count , err) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting sdp_getProtoFmtCountFromMedia");
	*err = E_NO_ERROR;
	return SipSuccess;
}
/*********************************************************************
** FUNCTION:  sdp_getProtoFmtAtIndexFromMedia
**
** DESCRIPTION: Get the Protocol-Format pair at a specified index
**
**
*********************************************************************/

SipBool sdp_getProtoFmtAtIndexFromMedia
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	( SdpMedia 	*pMedia,
	  SipNameValuePair **ppProtofmt,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( SdpMedia 	*pMedia,
	  SipNameValuePair *pProtofmt,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#endif
#else
#ifdef SIP_BY_REFERENCE
	( pMedia,ppProtofmt,dIndex,pErr)
	  SdpMedia 	*pMedia;
	  SipNameValuePair **ppProtofmt;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#else
	( pMedia,pProtofmt,dIndex,pErr)
	  SdpMedia 	*pMedia;
	  SipNameValuePair *pProtofmt;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
#endif
{
	SIP_Pvoid 	pElement_from_list;

	SIPDEBUGFN ( "Entering sdp_getProtoFmtAtIndexFromMedia");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
#ifdef SIP_BY_REFERENCE
	if ( (pMedia == SIP_NULL) || (ppProtofmt == SIP_NULL) )
#else
	if ( (pMedia == SIP_NULL) || (pProtofmt == SIP_NULL) )
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(pMedia->slProtofmt), dIndex, &pElement_from_list,\
			pErr) == SipFail)
		return SipFail;

	if ( pElement_from_list == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if ( sip_cloneSipNameValuePair(pProtofmt,\
		(SipNameValuePair *)pElement_from_list,pErr) == SipFail)
	{
		sip_freeString(pProtofmt->pName);
		sip_freeString(pProtofmt->pValue);
		return SipFail;
	}
#else
	HSS_LOCKEDINCREF(((SipNameValuePair *)pElement_from_list)->dRefCount);
	*ppProtofmt = (SipNameValuePair *)pElement_from_list;
#endif
	SIPDEBUGFN ( "Exiting sdp_getProtoFmtAtIndexFromMedia");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}
/*********************************************************************
** FUNCTION:  sdp_setProtoFmtAtIndexInMedia
**
** DESCRIPTION: set the Protocol-Format pair at a specified index
**
**
*********************************************************************/

SipBool sdp_setProtoFmtAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*pMedia,
	  SipNameValuePair *pProtofmt,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pMedia,pProtofmt,dIndex,pErr)
	  SdpMedia 	*pMedia;
	  SipNameValuePair *pProtofmt;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SIP_Pvoid 	pElement_in_list;

	SIPDEBUGFN ( "Entering sdp_setProtoFmtAtIndexInMedia");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pMedia == SIP_NULL) || (pProtofmt == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

#ifndef SIP_BY_REFERENCE
	if (sip_initSipNameValuePair((SipNameValuePair **)&pElement_in_list,\
		pErr) == SipFail)
		return SipFail;
	if ( sip_cloneSipNameValuePair((SipNameValuePair *)pElement_in_list,\
		pProtofmt,pErr) == SipFail)
	{
		sip_freeSipNameValuePair((SipNameValuePair *)pElement_in_list);
		return SipFail;
	}
#else
		HSS_LOCKEDINCREF(pProtofmt->dRefCount);
		pElement_in_list = (SipNameValuePair *)pProtofmt;
#endif

	if( sip_listSetAt(&(pMedia->slProtofmt), dIndex,\
		(SIP_Pvoid) pElement_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( pElement_in_list != SIP_NULL )
			 sip_freeSipNameValuePair((SipNameValuePair *)pElement_in_list);
#else
		HSS_LOCKEDDECREF(pProtofmt->dRefCount);
#endif
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting sdp_setProtoFmtAtIndexInMedia");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_insertProtoFmtAtIndexInMedia
**
** DESCRIPTION: Insert the Protocol-Format pair at a specified index
**
**
*********************************************************************/
SipBool sdp_insertProtoFmtAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*pMedia,
	  SipNameValuePair *pProtofmt,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pMedia,pProtofmt,dIndex,pErr)
	  SdpMedia 	*pMedia;
	  SipNameValuePair *pProtofmt;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SIP_Pvoid 	pElement_in_list;

	SIPDEBUGFN ( "Entering sdp_insertProtoFmtAtIndexInMedia");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pMedia == SIP_NULL) || (pProtofmt == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

#ifndef SIP_BY_REFERENCE
	if (sip_initSipNameValuePair((SipNameValuePair **)&pElement_in_list,\
		pErr) == SipFail)
		return SipFail;
	if ( sip_cloneSipNameValuePair((SipNameValuePair *)pElement_in_list,\
		pProtofmt,pErr) == SipFail)
	{
		sip_freeSipNameValuePair((SipNameValuePair *)pElement_in_list);
		return SipFail;
	}
#else
		HSS_LOCKEDINCREF(pProtofmt->dRefCount);
		pElement_in_list = (SipNameValuePair *)pProtofmt;
#endif

	if( sip_listInsertAt(&(pMedia->slProtofmt), dIndex, \
		(SIP_Pvoid) pElement_in_list, pErr) == SipFail)
	{
#ifndef SIP_BY_REFERENCE
		if ( pElement_in_list != SIP_NULL )
			 sip_freeSipNameValuePair((SipNameValuePair *)pElement_in_list);
#else
		HSS_LOCKEDDECREF(pProtofmt->dRefCount);
#endif
		return SipFail;
	}

	SIPDEBUGFN ( "Exiting sdp_insertProtoFmtAtIndexInMedia");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*********************************************************************
** FUNCTION:  sdp_deleteProtoFmtAtIndexInMedia
**
** DESCRIPTION: Delete Protocol-Format pair at a specified index
**
**
*********************************************************************/
SipBool sdp_deleteProtoFmtAtIndexInMedia
#ifdef ANSI_PROTO
	( SdpMedia 	*pMedia,
	  SIP_U32bit 	dIndex,
	  SipError 	*pErr )
#else
	( pMedia,dIndex,pErr)
	  SdpMedia 	*pMedia;
	  SIP_U32bit 	dIndex;
	  SipError 	*pErr;
#endif
{
	SIPDEBUGFN ( "Entering sdp_deleteProtoFmtAtIndexInMedia");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pMedia == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if( sip_listDeleteAt(&(pMedia->slProtofmt), dIndex, pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN ( "Exiting sdp_deleteProtoFmtAtIndexInMedia");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

#endif

/********************************************************************
** FUNCTION:sdp_getSdpType
**
** DESCRIPTION: This functon retrieves the Sdp Type from an SDP
**		message structure
**
**********************************************************************/
SipBool sip_getSdpType
     (SdpMessage *pMsg, SIP_S8bit **pSdpType, SipError *pErr)
{
	SipBool dRetVal = SipFail ;	
	SdpConnection *pSlConn = SIP_NULL ;
	
   SIPDEBUGFN("Entering sip_getSdpType") ;
#ifndef SIP_BY_REFERENCE
	  sip_initSdpConnection(&pSlConn,pErr) ;
		dRetVal= sdp_getConnection(pMsg,pSlConn, pErr) ;
#else
		 dRetVal= sdp_getConnection(pMsg, &pSlConn,pErr) ;
#endif

		if ( dRetVal == SipSuccess)
		{
				if (sdp_getNetTypeFromConnection(pSlConn,pSdpType,pErr)\
								== SipSuccess)
				{
						sip_freeSdpConnection(pSlConn) ;
						SIPDEBUGFN("Exiting sip_getSdpType") ;
						return SipSuccess ;
				}
				sip_freeSdpConnection(pSlConn) ;
		}
		SIPDEBUGFN("Exiting sip_getSdpType") ;
		return SipFail ;
}
