
/*
 * Copyright (c) 2003 NExtone Communications, Inc.
 * All rights reserved.
 */

/*****************************************************************************
 *
 *  Private Function Prototypes
 *
*****************************************************************************/


  /* Value Methods */
MFCP_Value *mfcp_value_new (MFCP_ValueTypes t);
MFCP_Value *mfcp_value_new_i (int i);
MFCP_Value *mfcp_value_new_s (char *s);

MFCP_Value *mfcp_value_free (MFCP_Value * v);
int mfcp_value_setint (MFCP_Value * p, int v);
int mfcp_value_getint (MFCP_Value * p);
int mfcp_value_setstr (MFCP_Value * p, char *c);
char *mfcp_value_getstr (MFCP_Value * p);
MFCP_Value *mfcp_value_add_list (MFCP_Value * p1, MFCP_Value * p2);
char *mfcp_value_vtoa (MFCP_Value * v, char *c, unsigned int len);



  /* Parameter methods */
MFCP_Parameter *mfcp_param_new (MFCP_ParameterTypes t);
MFCP_Parameter *mfcp_param_free (MFCP_Parameter * p);
MFCP_ValueTypes mfcp_param_lookup (MFCP_ParameterTypes t);


  /* Session Methods: */
MFCP_Return mfcp_req_crs (MFCP_Session * sess, int shutdownInterval, void (*func)(void *), void *appData);
MFCP_Request *mfcp_sreq_crs (MFCP_Session * sess, int shutdownInterval);
int mfcp_sess_rem_sent_req (MFCP_Session * sess, MFCP_Request * rPtr);
int mfcp_sess_listen (unsigned long int addr, unsigned int port);
MFCP_Session *mfcp_sess_accept (int fd);
int mfcp_sess_close (MFCP_Session * sess);
int mfcp_sess_seqid (MFCP_Session * sess);
int mfcp_sess_send_req (MFCP_Session * sess, MFCP_Request * rPtr);
int mfcp_sess_send_res (MFCP_Session * sess, MFCP_Request * rPtr);
int mfcp_send_failure(MFCP_Session *sess, MFCP_Request *rPtr, int respStatus, char *respString);
int  mfcp_sess_display(MFCP_Session *sess, char *caller);
void *mfcp_sess_reconnect(void *sess);

  /* Request Methods */
MFCP_Request *mfcp_req_new (MFCP_RequestType t);
int mfcp_req_callback(MFCP_Request *rPtr,void (*func)(void *),void *appData);

int mfcp_req_add_param (MFCP_Request * r, MFCP_ParameterTypes p,
                        MFCP_Value * v);
int mfcp_res_add_param (MFCP_Request * req, MFCP_ParameterTypes param,
			MFCP_Value * value);

MFCP_Value  *mfcp_req_getval (MFCP_Request * rPtr, MFCP_ParameterTypes type);
MFCP_Value  *mfcp_res_getval (MFCP_Request * rPtr, MFCP_ParameterTypes type);
int mfcp_req_display (MFCP_Request * rPtr,char *caller);

MFCP_Request *mfcp_req_lookup (MFCP_Session * sess, int rId);
MFCP_Request *mfcp_req_check_expiry_lookup (MFCP_Session * sess, int rId, int remove_expired);
int mfcp_req_mkpkt (MFCP_Request * req, char *buff, unsigned int bufferSize,
                    int seq);
int mfcp_req_display (MFCP_Request * rPtr, char *caller);


int mfcp_res_mkpkt (MFCP_Request * req, char *buff, unsigned int bufferSize,
                    int seq);
void mfcp_process_rsp (MFCP_Session * sess, MFCP_Request * rPtr);

  /* param methods */
MFCP_ParameterTypes mfcp_param_gettype (char *str);

  /* misc functions */
int mfcp_version_check (int majorVersion, int minorVersion);
int mfcp_send (MFCP_Session *sess, char *cbuf, int size, int flags);
char *mfcp_make_addrstring (char *buff, unsigned int addr,
                            unsigned short port);
int mfcp_get_addrport (char *addrStr, in_addr_t * addr, uint16_t *port);

void *mfcp_parse(void *);

  /* temporary */
#define mfcp_logit(str) \
NETDEBUG(MFCE, NETLOG_DEBUG4, ("log: file:%s line:%d %s\n", __FILE__,__LINE__ ,str))

int mfcp_trace (char *s, ...);

 
  /* lexor.l parse function */
MFCP_Request *lexor_parseit (MFCP_Session * tsess);



