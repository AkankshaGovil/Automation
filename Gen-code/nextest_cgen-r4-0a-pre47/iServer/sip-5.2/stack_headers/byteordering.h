#ifdef __cplusplus
extern "C" {
#endif 

#ifndef	__BYTEORDERING_H__
#define __BYTEORDERING_H__

#include<sipcommon.h>
#include<sipstruct.h>

extern SIP_U32bit SIP_htonl _ARGS_((SIP_U32bit hostlong));
extern SIP_U16bit SIP_htons _ARGS_((SIP_U16bit hostshort));
extern SIP_U32bit SIP_ntohl _ARGS_((SIP_U32bit netlong));
extern SIP_U16bit SIP_ntohs _ARGS_((SIP_U16bit netshort));
extern SipBool sip_init_deserializeSipMessage _ARGS_((SipMessage **s,en_SipMessageType type,SipError *err));
extern SipBool sip_init_deserializeSipRespMessage _ARGS_((SipRespMessage **r,SipError *err));
extern SipBool sip_init_deserializeSipReqMessage _ARGS_((SipReqMessage **r,SipError *err));
#endif
#ifdef __cplusplus
}
#endif 

