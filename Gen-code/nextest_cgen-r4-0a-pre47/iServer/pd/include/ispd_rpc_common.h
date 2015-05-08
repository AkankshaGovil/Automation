#ifndef _ISPD_RPC_COMMON_H_
#define _ISPD_RPC_COMMON_H_

#include <ispd.h>
#include <rpc/rpc.h>

/*	File:
 *
 *		ispd_rpc_common.h
 *
 *	Description:
 *
 *		This file contains structure definitions shared
 *		by both ispd rpc code and the native ispd code.
 */

/*  "$Id: ispd_rpc_common.h,v 1.7 2002/07/01 14:19:19 sturt Exp $" */

// Server startup and shutdown routines

extern void *   ispd_rpcsvc_init( void * args );

extern void     ispd_rpcsvc_shutdown(void );

// Client routines

extern	void		ispd_client_close( 	CLIENT * cl );

extern	int			get_iserver_info(	peer_info_t * peer_info_ptr );

extern	int			ispd_nullproc(	peer_info_t * peer_info_ptr );

extern	void		ispd_nullproc_localhost( void );

extern	int			send_iserver_info(	void );

extern	selection_result_t	
					select_vip_owner(	peer_info_t *	peer_info_ptr,
										vip_id_t		vip_id );

extern	int			send_status_change( void );

#endif /* _ISPD_RPC_COMMON_H_ */
