#include "bits.h"
#include "ipc.h"

#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"
#include "mem.h"
#include "age.h"
#include "xmltags.h"
#include "radclient.h"

#include "gis.h"
#include <netdb.h>
#include <malloc.h>
#include "bcpt.h"
#include "siputils.h"
#include "sipclone.h"

/* create an unquoted string from a quoted string */
static char *unq(char *s1)
{
	char *s2 = NULL;
	int len = strlen(s1);

	if(s1[0] == '"' && s1[len - 1] == '"' && len >= 2)
	{
		len -= 2;

		s2 = malloc(len + 1);

		if(len > 0)
		{
			strncpy(s2, (s1 + 1), len);
		}

		s2[len] = 0;
	}

	return(s2);
}


/* create a quoted string from an unquoted string */
static char *quote(char *s1)
{
	int len = strlen(s1) + 1;
	char *s2 = malloc(len + 2);

	strcpy(s2, "\"");
	strcat(s2, s1);
	strcat(s2, "\"");

	return(s2);
}


/* return -1 if CheckAuthorization fails, 0 if CheckAuthorization passes */
int SipCheckAuthorization(SipMessage *s, char *method)
{
	char fn[] = "SipCheckAuthorization:";
	SipError err;
	SipHeader *header = NULL;
	en_HeaderType dType;
	SipGenericCredential *pCredential=NULL;
	SipGenericChallenge *pChallenge=NULL;
	SIP_S8bit *pScheme=NULL,*pName=NULL,*pValue=NULL;
	SIP_U32bit count,k;
	SipParam *pSipParam=NULL;
	SIP_S8bit *pusername=NULL,*prealm=NULL,*pnonce=NULL,*puri=NULL,*presponse=NULL,*palgorithm=NULL;
	SIP_S8bit stringA1[256],stringA2[256],*tmpptr;
	header_url *from = NULL;
	char *callid = NULL;
	Credentials credentials;
	int returnvalue=-1;

	if(strcmp(method,"ACK")==0)
	{
		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s method ACK\n",fn));
		return 0;
	}

	if(strcmp(method,"REGISTER"))
	{
		/* non-register msg, check Proxyauthorization */
		dType=SipHdrTypeProxyauthorization;
	}
	else
	{
		/* register msg, check Authorization */
		dType=SipHdrTypeAuthorization;
	}

	if(sip_getHeaderCount(s,dType,&count,&err)==SipFail)
	{
		NETERROR(MSIP,("%s could not get hdr count %d\n",fn,err));
		return returnvalue;
	}
	if(count==0)
	{
		NETERROR(MSIP,("%s didn't find %d Hdr\n",fn,dType));
		return returnvalue;
	}

	if ((sip_initSipHeader(&header, SipHdrTypeAny, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize sip header %d\n",fn,err));
		goto _error;
	}
	if(sip_getHeader(s,dType,header,&err)==SipFail)
	{
		NETERROR(MSIP, ("%s could not retrieve %d header %d\n",fn,dType,err));
		goto _error;
	}

	switch(dType)
	{
	case SipHdrTypeAuthorization:
		if(sip_getCredentialsFromAuthorizationHdr(header,&pCredential,&err)==SipFail)
		{
			NETERROR(MSIP, ("%s Fail to get Credentials from AuthorHdr\n",fn));
			goto _error;
		}
		break;
	case SipHdrTypeProxyauthorization:
		if(sip_getCredentialsFromProxyAuthorizationHdr
		   (header,&pCredential,&err)==SipFail)
		{
			NETERROR(MSIP, ("%s Fail to get Credentials from ProxyAuthHdr\n",fn));
			goto _error;
		}
		break;
	default:
		goto _error;
	}

	if(sip_getChallengeFromCredential(pCredential,&pChallenge,&err)==SipFail)
	{
		NETERROR(MSIP, ("%s Fail to get Challenge from credential\n",fn));
		goto _error;
	}
	if(sip_getSchemeFromChallenge(pChallenge,&pScheme,&err)==SipFail)
	{
		NETERROR(MSIP, ("%s Fail to get scheme from challenge\n",fn));
		goto _error;
	}
	/* strcasecmp is used here and below, because PingTel incorrectly uses all capitals */
	if(strcasecmp(pScheme,"Digest"))
	{
		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s scheme (%s) not Digest\n",fn,pScheme));
		goto _error;
	}
	if(sip_getAuthorizationParamCountFromChallenge(pChallenge,&count,&err)==SipFail)
	{
		NETERROR(MSIP, ("%s Fail to get auth param count \n",fn));
		goto _error;
	}
	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s scheme=%s count=%d\n",fn,pScheme,count));
	k=0;
	while(k<count)
	{
		if(sip_getAuthorizationParamAtIndexFromChallenge(pChallenge,&pSipParam,k,&err)
		   ==SipFail)
		{
			NETERROR(MSIP, ("%s Fail to get %d-th Param from Challenge\n",fn,k));
			goto _error;
		}
		pName=pSipParam->pName;
		if(sip_getValueAtIndexFromSipParam(pSipParam,&pValue,0,&err)==SipFail)
		{
			NETERROR(MSIP, ("%s Fail to get value from SipParam\n",fn));
			goto _error;
		}
		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s %d-th param name=%s value=%s\n",
					     fn,k,pName,pValue));
		if(strcasecmp(pName,"username")==0)
		{
			pusername=unq(pValue);
		}
		else if(strcasecmp(pName,"realm")==0)
		{
			prealm=unq(pValue);
		}
		else if(strcasecmp(pName,"nonce")==0)
		{
			pnonce=unq(pValue);
		}
		else if(strcasecmp(pName,"uri")==0)
		{
			puri=unq(pValue);
		}
		else if(strcasecmp(pName,"response")==0)
		{
			presponse=unq(pValue);
		}
		else if(strcasecmp(pName,"algorithm")==0)
		{
			palgorithm=unq(pValue);
		}
		k++;
		sip_freeSipParam(pSipParam);
		pSipParam = NULL;
	}
	
	if(pusername==NULL || prealm==NULL || pnonce==NULL || puri==NULL || presponse==NULL)
	{
		NETERROR(MSIP, ("%s not all fields are found in message\n",fn));
		goto _error;
	}

	if(sipauth == 2)
	{
		if (SipExtractFromUri(s, &from, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s ExtractFromHeaders failed\n", fn));
			goto _error;
		}

		if (SipGetCallID(s, &callid, &err) == SipFail)
		{
			NETERROR(MSIP, ("%s Could not extract callid\n", fn));
			goto _error;
		}

		credentials.method = method;
		credentials.authentication_name = pusername;
		credentials.realm = prealm;
		credentials.uri = puri;
		credentials.digest_response = presponse;
		credentials.nonce = pnonce;
		credentials.algorithm = palgorithm ? palgorithm : strdup("md5");

		returnvalue = sendCredentials(from, callid, &credentials);

	}
	else
	{
		/* A1=unq(username-value) ":" unq(realm-value) ":" passwd */
		tmpptr=&stringA1[0];
		strcpy(tmpptr,pusername);
		strcat(tmpptr,":");
		strcat(tmpptr,prealm);
		strcat(tmpptr,":");
		strcat(tmpptr,sipauthpassword);
		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s A1=%s\n",fn,stringA1));
		HashString(tmpptr,tmpptr);
		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Hash(A1)=%s\n",fn,stringA1));

		/* A2=Method ":" digest-uri-value */
		tmpptr=&stringA2[0];
		strcpy(tmpptr,method);
		strcat(tmpptr,":");
		strcat(tmpptr,puri);
		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s A2=%s\n",fn,stringA2));
		HashString(tmpptr,tmpptr);
		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Hash(A2)=%s\n",fn,stringA2));

		/* request-digest=<"> <KD(H(A1), unq(nonce-value) ":" H(A2))> <"> */
		tmpptr=&stringA1[0];
		strcat(tmpptr,":");
		strcat(tmpptr,pnonce);
		strcat(tmpptr,":");
		strcat(tmpptr,stringA2);
		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Final Hash input=%s\n",fn,stringA1));
		HashString(tmpptr,tmpptr);
		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Final Hash output=%s\n",fn,stringA1));

		if(strcmp(tmpptr,presponse)==0)
		{
			returnvalue=0;
		}
	}

_error:
	SipCheckFree(from); SipCheckFree(callid);
	SipCheckFree(pusername); SipCheckFree(prealm); SipCheckFree(pnonce);
	SipCheckFree(puri); SipCheckFree(presponse); SipCheckFree(palgorithm);
	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipGenericCredential(pCredential);
	sip_freeSipGenericChallenge(pChallenge);
	sip_freeSipParam(pSipParam);
	return returnvalue;
}

/* return -1 if operation fails, 0 if succeeds */
int SipInsertAuthenticateHdr(SipMessage *s, char *method)
{
	char fn[] = "SipInsertAuthenticateHdr:";
	SipError err;
	SipHeader *header = NULL;
	en_HeaderType dType;
	SipGenericChallenge *pChallenge=NULL;
	char *nonce, *realm, *algorithm;
	SIP_S8bit *pScheme=NULL;
	SipParam *pSipParam=NULL;
	SIP_S8bit *pName[10],*pValue[10];
	SIP_S8bit quotedsipdomain[32];
	SIP_U32bit count,k;
	header_url *from = NULL;
	char *callid = NULL;
	Challenge challenge;
	int returnvalue=-1;
	char nonceStr[24] = "";
	time_t  rand = 0;

	time(&rand);
	snprintf(nonceStr, sizeof(nonceStr), "\"618%0lx\"", rand);
	nonce = strdup(nonceStr);
	realm = strdup("\"NexTone\"");
	algorithm = strdup("md5");

	count=0;
	pName[count]="realm";
	pValue[count++]=realm;

	#if 0
	/* Client should assume that the protection space consists of all URIs */
	pName[count]="domain";
	strcpy(quotedsipdomain,"\"sip:");
	strcat(quotedsipdomain,sipdomain);
	strcat(quotedsipdomain,"\"");
	pValue[count++]=quotedsipdomain;
	#endif

	pName[count]="nonce";
	pValue[count++]=nonce;
#if 0
	/* Opague is optional.  */
	pName[count]="opaque";
	pValue[count++]="\"\"";
#endif
	/* stale is always set to false when using freeRadius
	 * FreeRadius does not indicate if the username/password is correct
	 */
	pName[count]="stale";
	pValue[count++]="FALSE";

	pName[count]="algorithm";
	pValue[count++]=algorithm;

	if(strcmp(method,"REGISTER"))
	{
		/* non-register msg, insert ProxyAuthenticate */
		dType=SipHdrTypeProxyAuthenticate;
	}
	else
	{
		/* register msg, insert Www-Authenticate */
		dType=SipHdrTypeWwwAuthenticate;
	}

	if(sip_initSipGenericChallenge(&pChallenge,&err)==SipFail)
	{
		NETERROR(MSIP, ("%s could not init pChallenge\n",fn));
		goto _error;
	}
	if ((sip_initSipHeader(&header, dType, &err))==SipFail)
	{
		NETERROR(MSIP, ("%s could not initialize sip header\n",fn));
		goto _error;
	}

	for(k=count;k>0;k--)
	{
		if(sip_initSipParam(&pSipParam,&err)==SipFail)
		{
			NETERROR(MSIP, ("%s could not init SipParam\n",fn));
			goto _error;
		}
		if(sip_setNameInSipParam(pSipParam,strdup(pName[k-1]),&err)==SipFail)
		{
			NETERROR(MSIP, ("%s could not set name in SipParam\n",fn));
			goto _error;
		}
		if(sip_insertValueAtIndexInSipParam(pSipParam,strdup(pValue[k-1]),0,&err)
		   ==SipFail)
		{
			NETERROR(MSIP, ("%s could not insert value in SipParam\n",fn));
			goto _error;
		}
		if(sip_insertAuthorizationParamAtIndexInChallenge(pChallenge,pSipParam,0,&err)
		   ==SipFail)
		{
			NETERROR(MSIP, ("%s could not insert SipParam in Challenge\n",fn));
			goto _error;
		}
		sip_freeSipParam(pSipParam);
		pSipParam = NULL;
	}
	if(sip_setSchemeInChallenge(pChallenge,strdup("Digest"),&err)==SipFail)
	{
		NETERROR(MSIP, ("%s could not set scheme in Challenge\n",fn));
		goto _error;
	}

	switch(dType)
	{
	case SipHdrTypeProxyAuthenticate:
		if(sip_setChallengeInProxyAuthenticateHdr(header,pChallenge,&err)==SipFail)
		{
			NETERROR(MSIP, ("%s could not set Challenge in ProxyAuthHdr\n",fn));
			goto _error;
		}
		break;
	case SipHdrTypeWwwAuthenticate:
		if(sip_setChallengeInWwwAuthenticateHdr(header,pChallenge,&err)==SipFail)
		{
			NETERROR(MSIP, ("%s could not set Challenge in WWWAuthHdr\n",fn));
			goto _error;
		}
		break;
	default:
		goto _error;
	}

	if(sip_insertHeaderAtIndex(s,header,0,&err)==SipFail)
	{
		NETERROR(MSIP, ("%s could not insert header (%d) in message\n",fn,dType));
		goto _error;
	}
	returnvalue=0;

_error:
	sip_freeSipHeader(header);
	SipCheckFree(header);
	sip_freeSipGenericChallenge(pChallenge);
	sip_freeSipParam(pSipParam);

	return returnvalue;
}

/* make Authorization or Proxyauthorization hdr as string based on s and authenticate
 * and store at authorization
 * memory MUST be provided by calling function
 */
int SipMakeAuthorizationString(SipMessage *s, char *authenticate, en_HeaderType dType,
			       char *authorization)
{
	char fn[]="SipMakeAuthorizationString";
	SipError err;
	SipReqLine *pReqLine = NULL;
	SIP_S8bit *pMethod = NULL;
	header_url *requri = NULL;
	SIP_S8bit pusername[256],prealm[256],pnonce[256],puri[256],presponse[256];
	SIP_S8bit stringA1[256],stringA2[256];
	SIP_S8bit *tmpptr=NULL, *ptrstart=NULL, *ptrend=NULL;
	char username[]="\"SIP User\"";
	char userpasswd[]="NeedToConfigurePasswd";

	if( sip_getReqLine(s, &pReqLine, &err) == SipFail) 
	{
		NETERROR(MSIP, ("%s fail to get reqline\n",fn));
		goto _error;
	}
	if( (sip_getMethodFromReqLine(pReqLine, &pMethod, &err)) == SipFail)
	{
		NETERROR(MSIP, ("%s fail to get method from reqline\n",fn));
		goto _error;
	}

	if( SipExtractReqUri(s, &requri, &err) == SipFail)
	{
		NETERROR(MSIP, ("%s SipExtractRequri faled\n",fn));
		goto _error;
	}

	/* header name */
	if(dType==SipHdrTypeProxyauthorization)
	{
		strcpy(authorization, "Proxy-authorization:");
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s preparing Proxy-authorizatin\n", fn));
	}
	else 
	{
		strcpy(authorization, "Authorization:");
		NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s preparing Authorizatin\n", fn));
	}
	/* scheme */
	strcat(authorization, "Digest ");
	/* username */
	strcpy(pusername, username);
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s username = %s\n", fn, pusername));
	strcat(authorization, "username=");
	strcat(authorization, pusername);
	strcat(authorization, ", ");
	/* realm */
	if( (tmpptr = strstr(authenticate, "realm")) == NULL)
	{
		NETERROR(MSIP, ("%s no realm found\n", fn));
		goto _error;
	}
	if( (ptrstart = strstr(tmpptr, "\"")) == NULL)
	{
		NETERROR(MSIP, ("%s no realm value\n", fn));
		goto _error;
	}
	if( (ptrend = strstr(ptrstart+1, "\"")) == NULL)
	{
		NETERROR(MSIP, ("%s error in realm value\n", fn));
		goto _error;
	}
	strncpy(prealm, ptrstart, ptrend-ptrstart+2);
	prealm[ptrend-ptrstart+1] = '\0';
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s realm = %s\n", fn, prealm));
	strcat(authorization, "realm=");
	strcat(authorization, prealm);
	strcat(authorization, ", ");
	/* nonce */
	if( (tmpptr = strstr(authenticate, "nonce")) == NULL)
	{
		NETERROR(MSIP, ("%s no nonce found\n", fn));
		goto _error;
	}
	if( (ptrstart = strstr(tmpptr, "\"")) == NULL)
	{
		NETERROR(MSIP, ("%s no nonce value\n", fn));
		goto _error;
	}
	if( (ptrend = strstr(ptrstart+1, "\"")) == NULL)
	{
		NETERROR(MSIP, ("%s error in nonce value\n", fn));
		goto _error;
	}
	strncpy(pnonce, ptrstart, ptrend-ptrstart+2);
	pnonce[ptrend-ptrstart+1] = '\0';
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s nonce = %s\n", fn, pnonce));
	strcat(authorization, "nonce=");
	strcat(authorization, pnonce);
	strcat(authorization, ", ");
	/* opaque */
	strcat(authorization, "opaque=\"\", ");
	/* uri */
	strcpy(puri, "\"sip:");
	if(requri->name)
	{
		strcat(puri, requri->name);
		strcat(puri, "@");
	}
	strcat(puri, requri->host);
	strcat(puri, "\"");
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s uri = %s\n", fn, puri));
	strcat(authorization, "uri=");
	strcat(authorization, puri);
	strcat(authorization, ", ");

	/* response */
	/* A1=unq(username-value) ":" unq(realm-value) ":" passwd */
	tmpptr=&stringA1[0];
	strncpy(tmpptr,(pusername+1),strlen(pusername)-1);
	tmpptr[strlen(pusername)-2] = '\0';
	strcat(tmpptr,":");
	strncat(tmpptr,(prealm+1),strlen(prealm)-2);
	strcat(tmpptr,":");
	strcat(tmpptr,userpasswd);
	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s A1=%s\n",fn,stringA1));
	HashString(tmpptr,tmpptr);
	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Hash(A1)=%s\n",fn,stringA1));

	/* A2=Method ":" digest-uri-value */
	tmpptr=&stringA2[0];
	strcpy(tmpptr,pMethod);
	strcat(tmpptr,":");
	strncat(tmpptr,(puri+1),strlen(puri)-2);
	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s A2=%s\n",fn,stringA2));
	HashString(tmpptr,tmpptr);
	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Hash(A2)=%s\n",fn,stringA2));

	/* request-digest=<"> <KD(H(A1), unq(nonce-value) ":" H(A2))> <"> */
	tmpptr=&stringA1[0];
	strcat(tmpptr,":");
	strncat(tmpptr,(pnonce+1),strlen(pnonce)-2);
	strcat(tmpptr,":");
	strcat(tmpptr,stringA2);
	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Final Hash input=%s\n",fn,stringA1));
	HashString(tmpptr,tmpptr);
	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Final Hash output=%s\n",fn,stringA1));

	/* set response */
	strcat(authorization, "response=\"");
	strcat(authorization, tmpptr);
	strcat(authorization, "\"");

	NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Hdr is %s\n", fn, authorization));

	sip_freeSipReqLine(pReqLine);
	SipCheckFree(requri);
	return 1;
 _error:
	sip_freeSipReqLine(pReqLine);
	SipCheckFree(requri);
	return -1;
}
