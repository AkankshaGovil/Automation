
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
#include "uh323.h"
#include "uh323cb.h"
#include "db.h"
#include "firewallcontrol.h"
#include <md5.h>
#include "gis.h"
#include "gk.h"

#ifdef H323v30
/* define this for version 3 stack */
unsigned char * cmEmGetQ931Syntax();
#endif


static char 	md5[] =  "iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) md5(5)"; 
static int		oidlen = 128;
static char 	oidbuff[128]  = {0};
static HPST 	hPstPwdCert = NULL;

void uh235Init(void)
{
	static char fn[] = "uh235Init";
	oidlen = oidEncodeOID(oidlen,oidbuff,md5);
	hPstPwdCert = pstConstruct(cmEmGetQ931Syntax(),"PwdCertToken");
	if(hPstPwdCert == NULL)
	{
		NETERROR(MH323,
			("%s pstConstruct PwdCertToken Error\n",fn));
	}
}


int 
rrqAddAuthentication(HRAS hsRas,char senId[],char pass[])
{
	static char 	fn[] = "rrqAddAuthentication";
	int 			senderIdlen,pwdlength;
	char 			senderId[256] = {0},passwd[128] = {0};
	long 			timeStamp;
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int				nodeId;
	struct 	timeval tp;
	char			hash[16] = {0};

	if(strlen(pass) == 0)
	{
		return 0;
	}

	senderIdlen = __stringToBMP(senId,(char*)senderId);
	pwdlength = __stringToBMP(pass,(char*)passwd);

	cmMeiEnter(UH323Globals()->hApp);
	nodeId = cmGetProperty((HPROTOCOL)hsRas);

	if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
		nodeId,
		"request.registrationRequest.cryptoTokens.1.cryptoEPPwdHash.alias.h323-ID",
		senderIdlen, senderId) < 0)
	{
		NETERROR(MH323, ("%s could not build cryptoH323\n",fn));
	}

	gettimeofday(&tp,NULL);
	timeStamp = tp.tv_sec;
	if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
		nodeId,
		"request.registrationRequest.cryptoTokens.1.cryptoEPPwdHash.timeStamp",
		timeStamp,NULL) < 0)
	{
		NETERROR(MH323, ("%s could not set TIMESTAMP\n",fn));
	}

	if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
		nodeId,
		"request.registrationRequest.cryptoTokens.1.cryptoEPPwdHash.token.algorithmOID",
		oidlen,oidbuff) < 0)
	{
		NETERROR(MH323, ("%s could not set token algorithmOID\n",fn));
	}

	if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
		nodeId,
		"request.registrationRequest.cryptoTokens.1.cryptoEPPwdHash.token.paramS.null",
		0,NULL) < 0)
	{
		NETERROR(MH323, ("%s could not set token paramS\n",fn));
	}
	
	mkHash(timeStamp,senderIdlen,senderId,pwdlength,passwd,hash);

	if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
		nodeId,
		"request.registrationRequest.cryptoTokens.1.cryptoEPPwdHash.token.hash",
		128,hash) < 0)
	{
		NETERROR(MH323, ("%s could not set token paramS\n",fn));
	}

	cmMeiExit(UH323Globals()->hApp);
	return 0;
}

int 
grqAddAuthentication(HRAS hsRas)
{
	static char 	fn[] = "grqAddAuthentication";
	int 			senderIdlen,pwdlength;
	char 			senderId[256] = {0},passwd[128] = {0};
	long 			timeStamp;
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int				nodeId;
	struct 	timeval tp;
	char			hash[16] = {0};

	cmMeiEnter(UH323Globals()->hApp);
	nodeId = cmGetProperty((HPROTOCOL)hsRas);
#ifdef  _SHA1 
		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"gatekeeperRequest.authenticationCapability.AuthenticationMechanism.pwdSymEnc",
			0,NULL) < 0)
		{
			NETERROR(MH323, ("GRQ could not add authentication\n"));
		}
#endif
		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.gatekeeperRequest.authenticationCapability.AuthenticationMechanism.pwdHash",
			0,NULL) < 0)
		{
			NETERROR(MH323, ("GRQ could not add authentication\n"));
		}

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.gatekeeperRequest.algorithmOIDs",
			0,NULL) < 0)
		{
			NETERROR(MH323, ("GRQ could not add OID %d %s\n",oidlen,oidbuff));
		}

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.gatekeeperRequest.algorithmOIDs",
			oidlen,oidbuff) < 0)
		{
			NETERROR(MH323, ("GRQ could not add ObjectId %d %s\n",oidlen,oidbuff));
		}

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.gatekeeperRequest.algorithmOIDs.0",
			oidlen,oidbuff) < 0)
		{
			NETERROR(MH323, ("GRQ could not add ObjectId %d %s\n",oidlen,oidbuff));
		}

		cmMeiExit(UH323Globals()->hApp);
		return 0;
}

int arqAddAuthentication(HRAS hsRas,char senId[],char pass[])
{
	static char 	fn[] = "arqAddAuthentication";
	int 			senderIdlen,pwdlength;
	char 			senderId[256] = {0},passwd[128] = {0};
	long 			timeStamp;
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int				nodeId;
	struct 	timeval tp;
	char			hash[16] = {0};

	if(strlen(pass) == 0)
	{
		return 0;
	}

	cmMeiEnter(UH323Globals()->hApp);
	nodeId = cmGetProperty((HPROTOCOL)hsRas);

		senderIdlen = __stringToBMP(senId,(char*)senderId);
		pwdlength = __stringToBMP(pass,(char*)passwd);

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.admissionRequest.cryptoTokens.1.cryptoEPPwdHash.alias.h323-ID",
			senderIdlen, senderId) < 0)
		{
			NETERROR(MH323, ("%s could not build cryptoH323\n",fn));
		}

		gettimeofday(&tp,NULL);
		timeStamp = tp.tv_sec;
		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.admissionRequest.cryptoTokens.1.cryptoEPPwdHash.timeStamp",
			timeStamp,NULL) < 0)
		{
			NETERROR(MH323, ("%s could not set TIMESTAMP\n",fn));
		}

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.admissionRequest.cryptoTokens.1.cryptoEPPwdHash.token.algorithmOID",
			oidlen,oidbuff) < 0)
		{
			NETERROR(MH323, ("%s could not set token algorithmOID\n",fn));
		}

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.admissionRequest.cryptoTokens.1.cryptoEPPwdHash.token.paramS.null",
			0,NULL) < 0)
		{
			NETERROR(MH323, ("%s could not set token paramS\n",fn));
		}
		
		mkHash(timeStamp,senderIdlen,senderId,pwdlength,passwd,hash);

		/* CHECKOUT - SHOULD THE LENGTH Be 128 */
		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.admissionRequest.cryptoTokens.1.cryptoEPPwdHash.token.hash",
			128,hash) < 0)
		{
			NETERROR(MH323, ("%s could not set token paramS\n",fn));
		}

	cmMeiExit(UH323Globals()->hApp);
	return 0;
}

// Not used currently
int irrAddAuthentication(int nodeId,char senId[],char pass[])
{
	static char 	fn[] = "irrAddAuthentication";
	int 			senderIdlen,pwdlength;
	char 			senderId[256] = {0},passwd[128] = {0};
	long 			timeStamp;
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	struct 	timeval tp;
	char			hash[16] = {0};

	if(strlen(pass) == 0)
	{
		return 0;
	}


		senderIdlen = __stringToBMP(senId,(char*)senderId);
		pwdlength = __stringToBMP(pass,(char*)passwd);

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.infoRequestResponse.cryptoTokens.1.cryptoEPPwdHash.alias.h323-ID",
			senderIdlen, senderId) < 0)
		{
			NETERROR(MH323, ("%s could not build cryptoH323\n",fn));
		}

		gettimeofday(&tp,NULL);
		timeStamp = tp.tv_sec;
		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.infoRequestResponse.cryptoTokens.1.cryptoEPPwdHash.timeStamp",
			timeStamp,NULL) < 0)
		{
			NETERROR(MH323, ("%s could not set TIMESTAMP\n",fn));
		}

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.infoRequestResponse.cryptoTokens.1.cryptoEPPwdHash.token.algorithmOID",
			oidlen,oidbuff) < 0)
		{
			NETERROR(MH323, ("%s could not set token algorithmOID\n",fn));
		}

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.infoRequestResponse.cryptoTokens.1.cryptoEPPwdHash.token.paramS.null",
			0,NULL) < 0)
		{
			NETERROR(MH323, ("%s could not set token paramS\n",fn));
		}
		
		mkHash(timeStamp,senderIdlen,senderId,pwdlength,passwd,hash);

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"request.infoRequestResponse.cryptoTokens.1.cryptoEPPwdHash.token.hash",
			128,hash) < 0)
		{
			NETERROR(MH323, ("%s could not set token paramS\n",fn));
		}

	return 0;
}


int addDrqCdr179(int nodeId,  long timeStamp, CallHandle *callHandle)
{
    static char fn[] = "addDrqCdr179";
    int len,cause = 0,discReason = 0;
    struct  timeval tp;
    static char vt[] =  "(2) (16) (840) (1) (113777) (1) (2) (179)";
    int oidlen = 128,datalen = 0,isstring,tokenVal;
    char oidbuff[128]  = {0},databuff[512] = {0},tokenbuff[512] = {0},
		f2[512] = {0x60,0x25,0x00,0x1e,0x30},
		f1[32]= {0x60,0x25,0x00,0x1d,0x20};
    HPVT            hVal = cmGetValTree(UH323Globals()->hApp);
	int i,j,totallen,f1len = 5,f2len = 5,tokenbufflen = 0;
	unsigned long duration = 0,x,y;
	char 	ipstr[17] = {0};
	timedef durTime;


	if(!timedef_iszero(&callHandle->callConnectTime))
	{
		timedef_sub(&callHandle->callEndTime, &callHandle->callConnectTime, &durTime);
		duration = timedef_rndsec(&durTime);
	}
	else  
	{
		if(callHandle->callError || callHandle->callDetails2.callError)
		{
			discReason = 1;
		}
		else {
			discReason = 2;
		}
	}
	for(i=0,x = duration>>8;x;x = x>>8,++i);
	y = 0x80 << 8*i;
	if(duration & y) 
	{
		++i;
		f1[f1len+1] = 0;
	}
	f1[f1len++] = ++i; 
	for(j=i-1,x = duration ;j>=0;--j)
	{
		f1[f1len+j] = x & 0xff; 
		x = x >> 8;
	}
	f1len += i;

	if(callHandle->cause>0)
	{
		cause = callHandle->cause -1;
	}
	else if(callHandle->callDetails2.cause >0)
	{
		cause = callHandle->callDetails2.cause - 1;
	}
		
	sprintf(databuff,"DISCONNECT_REASON=%d,TIME=%lu,DURATION=%lu,DISCONNECT_STRING=%s,ORIGIN=0,OUTBOUND_GTW_IP=%s,DNIS=%s",discReason,timeStamp,duration,getIsdnCCString(cause),FormatIpAddress(callHandle->phonode.ipaddress.l,ipstr),callHandle->dialledNumber);
	datalen = strlen(databuff);
	f2[f2len++] = (datalen >> 8) |0x80;
	f2[f2len++] = datalen & 0xff;
	memcpy(f2+f2len,databuff,datalen);

	totallen = f1len+f2len+datalen;
	
	tokenbuff[tokenbufflen++] = 2;
	memcpy(tokenbuff+tokenbufflen,f1,f1len);
	tokenbufflen += f1len;
	memcpy(tokenbuff+tokenbufflen,f2,f2len);
	tokenbufflen += f2len;
	memcpy(tokenbuff+tokenbufflen,databuff,datalen);
	tokenbufflen += datalen;

    oidlen = oidEncodeOID(oidlen,oidbuff,vt);
	if (pvtBuildByPath(hVal,
		nodeId,
		"request.disengageRequest.tokens.1.tokenOID",
		oidlen, oidbuff) < 0)
	{
		NETERROR(MH323, 
			("%s could not build clearToken tokenOID hsCall = %p\n",
			fn,H323hsCall(callHandle)));
	}
    oidlen = oidEncodeOID(oidlen,oidbuff,vt);

	if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
		nodeId,
		"request.disengageRequest.tokens.1.nonStandard.nonStandardIdentifier",
		oidlen, oidbuff) < 0)
	{
		NETERROR(MH323, 
			("%s could not build nonStandardIdentfier hsCall = %p\n",
			fn,H323hsCall(callHandle)));
	}

	if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
		nodeId,
		"request.disengageRequest.tokens.1.nonStandard.data",
		tokenbufflen, tokenbuff) < 0)
	{
		NETERROR(MH323, 
			("%s could not build nonStandarddata hsCall = %p\n",
			fn,H323hsCall(callHandle)));
	}
	return 0;
}


int drqAddAuthentication(HRAS hsRas,char senId[],char pass[],CallHandle *callHandle)
{
	static char 	fn[] = "drqAddAuthentication";
	int 			senderIdlen,pwdlength;
	char 			senderId[256] = {0},passwd[128] = {0};
	long 			timeStamp;
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	int				nodeId;
	struct 	timeval tp;
	char			hash[16] = {0};

	if(strlen(pass) == 0)
	{
		return 0;
	}

	gettimeofday(&tp,NULL);
	timeStamp = tp.tv_sec;
	cmMeiEnter(UH323Globals()->hApp);
	nodeId = cmGetProperty((HPROTOCOL)hsRas);
	addDrqCdr179(nodeId,timeStamp,callHandle);

	senderIdlen = __stringToBMP(senId,(char*)senderId);
	pwdlength = __stringToBMP(pass,(char*)passwd);

	if (pvtBuildByPath(hVal,
		nodeId,
		"request.disengageRequest.cryptoTokens.1.cryptoEPPwdHash.alias.h323-ID",
		senderIdlen, senderId) < 0)
	{
		NETERROR(MH323, ("%s could not build cryptoH323 hsCall = %p\n",
			fn,H323hsCall(callHandle)));
	}

	if (pvtBuildByPath(hVal,
		nodeId,
		"request.disengageRequest.cryptoTokens.1.cryptoEPPwdHash.timeStamp",
		timeStamp,NULL) < 0)
	{
		NETERROR(MH323, ("%s could not build TIMESTAMP hsCall = %p\n",
			fn,H323hsCall(callHandle)));
	}

	if (pvtBuildByPath(hVal,
		nodeId,
		"request.disengageRequest.cryptoTokens.1.cryptoEPPwdHash.token.algorithmOID",
		oidlen,oidbuff) < 0)
	{
		NETERROR(MH323, ("%s could not build algorithmOID hsCall = %p\n",
			fn,H323hsCall(callHandle)));
	}

	if (pvtBuildByPath(hVal,
		nodeId,
		"request.disengageRequest.cryptoTokens.1.cryptoEPPwdHash.token.paramS.null",
		0,NULL) < 0)
	{
		NETERROR(MH323, ("%s could not build params hsCall = %p\n",
			fn,H323hsCall(callHandle)));
	}
	
	mkHash(timeStamp,senderIdlen,senderId,pwdlength,passwd,hash);

	if (pvtBuildByPath(hVal,
		nodeId,
		"request.disengageRequest.cryptoTokens.1.cryptoEPPwdHash.token.hash",
		128,hash) < 0)
	{
		NETERROR(MH323, ("%s could not build hash hsCall = %p\n",
			fn,H323hsCall(callHandle)));
	}

	cmMeiExit(UH323Globals()->hApp);
	return 0;
}

int maketree(long timeStamp,int length,char senderId[],int pwdlength,char passwd[],HPVT hVal,int nodeId)
{
	static char fn[] = "maketree";
	int tokenNodeId;
	int buflen = 256;
	char tokenOid[] = "0 0", tokenOidBuff[128];
	int tokenOidLen = 128;

	tokenOidLen = oidEncodeOID(tokenOidLen,tokenOidBuff,tokenOid);
	if (pvtBuildByPath(hVal,
		nodeId,
		"tokenOID",
		tokenOidLen, tokenOidBuff) < 0)
	{
		NETERROR(MH323, ("%s could not build tokenOid\n", fn));
	}

	if (pvtBuildByPath(hVal,
		nodeId,
	"timeStamp",
		timeStamp,NULL) < 0)
	{
		NETERROR(MH323, ("%s RRQ could not set TimeStamp\n",fn));
	}
	

	if (pvtBuildByPath(hVal,
		nodeId,
		"generalID",
		length, senderId) < 0)
	{
		NETERROR(MH323, ("%s could not build ClearToken GeneralID\n",fn));
	}

	if (pvtBuildByPath(hVal,
		nodeId,
		"password",
		pwdlength, passwd) < 0)
	{
		NETERROR(MH323, ("%s could not build ClearToken password\n",fn));
	}


	return nodeId;
	/* delete the root that you have added!!! */
}

void
getHash(char buffer[], int encodedlen, char hash[])
{
	static char fn[] = "getHash";
	MD5(buffer,encodedlen,hash);
	/* print Hash */
	{
		int j;
		unsigned char *p;
		char *q;
		char tmpstr[64] = {0};

			p = hash;
			q = tmpstr;


			for(j = 0; j<16; ++j,++p,q+=2)
			{
				sprintf (q,"%02x",*p);
			}
		NETDEBUG(MH323,NETLOG_DEBUG4,("%s hash = %s \n",fn,tmpstr));
	}
}

void
printAsn1(char buffer[], int encodedlen)
{

		int j;
		unsigned char *p;
		char *q;
		char tmpstr[256] = {0};

		p = buffer;
		q = tmpstr;


		for(j = 0; j<encodedlen; ++j,++p,q+=2)
		{
			sprintf (q,"%2x",*p);
		}
		NETDEBUG(MH323,NETLOG_DEBUG4,("printAsn1 len = %d Asn1 =%s \n",encodedlen,tmpstr));
}


void mkHash(long timeStamp,int genIDlen,char generalID[],int pwdlength,char passwd[], unsigned char hash[])
{
	static char fn[] = "mkHash";
	static char oidstr[] = {"(0) (0)"};
	HPVT 		hVal = cmGetValTree(UH323Globals()->hApp);
	int 		nodeId,tokenNodeId;
	char 		oidbuff[256] = {0};
	char 		encodedMsg[256] = {0};
	int 		buflen = 256,encodedlen;
	int			oidlen;

	

	oidlen = strlen(oidstr);
	oidlen = oidEncodeOID(oidlen,oidbuff,oidstr);
	if(oidlen <=0)
	{
		NETERROR(MH323,
			("%s oidEncodeOID failed. oidlen = %d\n",fn,oidlen));
	}
	cmMeiEnter(UH323Globals()->hApp);

		nodeId = pvtAddRoot(hVal,
				hPstPwdCert,
				0,
				NULL);

		if (nodeId < 0)
		{
			NETERROR(MH323,
			("%s pvtAddRoot  returns error\n",fn));
		}

		tokenNodeId = maketree(timeStamp,genIDlen,generalID,pwdlength,passwd,hVal,nodeId);

		if(cmEmEncode ( hVal,
			tokenNodeId, encodedMsg, buflen, &encodedlen) <0)
		{
			NETERROR(MH323, ("%s Error Encoding ClearToken\n",fn));
		}
		
#if 0
		pvtPrintStd(hVal,tokenNodeId,62);
		printAsn1(encodedMsg,encodedlen);
#endif
		getHash(encodedMsg,encodedlen,hash);
		pvtDelete(hVal,nodeId);
		cmMeiExit(UH323Globals()->hApp);
	
}

// Add a Null Character in the end
int __stringToBMP(const char* str,char* bmpStr)
{
	char str1[256] = {0};
    int len=strlen(str)+1;
    int i;
	strcpy(str1,str);

    for (i=0;i<len;i++) {
        bmpStr[i*2]=0;
        bmpStr[i*2+1]=str[i];
    }
    return i*2;
}
			
/* This is called from ACF */
int getAcfTokenNodeId(HRAS hsRas)
{
	static char 	fn[] = "getAcfTokenNodeId";
	int     		nodeId,tokenNodeId,newNodeId;
	HPVT    		hVal = cmGetValTree(UH323Globals()->hApp);
	HPVT 			peerhVal = cmGetValTree(UH323Globals()->peerhApp);


	cmMeiEnter(UH323Globals()->hApp);
	cmMeiEnter(UH323Globals()->peerhApp);
	if((nodeId = cmGetProperty((HPROTOCOL)hsRas)) <0)
	{
		NETERROR(MARQ,("%s : cmGetProperty failed\n", fn ));
		goto _error;
	}

	if ((tokenNodeId = pvtGetNodeIdByPath(hVal, nodeId,
		"response.admissionConfirm.tokens")) < 0)
	{
		NETDEBUG(MARQ,NETLOG_DEBUG4,("%s : No tokenNodeId in ACF\n", fn ));
		goto _error;
	}

	newNodeId = pvtAddRoot(peerhVal,
			hPstSetup,
			0,
			NULL);

	if (newNodeId < 0)
	{
		NETERROR(MH323,
		("%s pvtAddRoot  returns error\n",fn));
		goto _error;
	}

	if(pvtAddTree(peerhVal,newNodeId,hVal,tokenNodeId) < 0)
	{
		NETERROR(MH323, ("%s pvtAddTree failed to copy token\n",fn));
		goto _freeNode;
	}

	if ((tokenNodeId = pvtGetNodeIdByPath(peerhVal,
		newNodeId,
		".tokens")) < 0)
	{
		NETERROR(MH323, ("%s could not get tokenNodeId\n",fn));
		goto _freeNode;
	}

	cmMeiExit(UH323Globals()->peerhApp);
	cmMeiExit(UH323Globals()->hApp);
	return tokenNodeId;
_freeNode:
	pvtDelete(peerhVal,newNodeId);	
_error:
	cmMeiExit(UH323Globals()->peerhApp);
	cmMeiExit(UH323Globals()->hApp);
	return -1;
}

/* This is called from GkAdmitCallFromSetup */
int getSetupTokenNodeId(HCALL hsCall)
{
	static char 	fn[] = "getSetupTokenNodeId";
	int     		nodeId,tokenNodeId,newNodeId;
	HPVT    		hVal = cmGetValTree(UH323Globals()->hApp);
	HPVT 			peerhVal = cmGetValTree(UH323Globals()->peerhApp);


	cmMeiEnter(UH323Globals()->hApp);
	cmMeiEnter(UH323Globals()->peerhApp);
	if((nodeId = cmGetProperty((HPROTOCOL)hsCall)) <0)
	{
		NETERROR(MARQ,("%s : cmGetProperty failed\n", fn ));
		goto _error;
	}

	if ((tokenNodeId = pvtGetNodeIdByPath(hVal, nodeId,
		"setup.message.setup.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.setup.tokens")) < 0)
	{
		NETDEBUG(MARQ,NETLOG_DEBUG4,("%s : No tokenNodeId in Setup\n", fn ));
		goto _error;
	}

	newNodeId = pvtAddRoot(peerhVal,
			cmGetSynTreeByRootName(UH323Globals()->hApp, "ras"),
			0,
			NULL);

	if (newNodeId < 0)
	{
		NETERROR(MH323,
		("%s pvtAddRoot  returns error\n",fn));
		goto _error;
	}

	if(pvtAddTree(peerhVal,newNodeId,hVal,tokenNodeId) < 0)
	{
		NETERROR(MH323, ("%s pvtAddTree failed to copy token\n",fn));
		goto _freeNode;
	}

	if ((tokenNodeId = pvtGetNodeIdByPath(peerhVal,
		newNodeId,
		".tokens")) < 0)
	{
		NETERROR(MH323, ("%s could not get tokenNodeId\n",fn));
		goto _freeNode;
	}

	cmMeiExit(UH323Globals()->peerhApp);
	cmMeiExit(UH323Globals()->hApp);
	return tokenNodeId;
_freeNode:
	pvtDelete(peerhVal,newNodeId);	
_error:
	cmMeiExit(UH323Globals()->peerhApp);
	cmMeiExit(UH323Globals()->hApp);
	return -1;
}


/* This is called from HandleLCF */
int getLcfTokenNodeId(HRAS hsRas)
{
	static char 	fn[] = "getLcfTokenNodeId";
	int     		nodeId,tokenNodeId,newNodeId;
	HPVT    		hVal = cmGetValTree(UH323Globals()->hApp);
	HPVT 			peerhVal = cmGetValTree(UH323Globals()->peerhApp);

	cmMeiEnter(UH323Globals()->hApp);
	cmMeiEnter(UH323Globals()->peerhApp);
	if((nodeId = cmGetProperty((HPROTOCOL)hsRas)) <0)
	{
		NETERROR(MARQ,("%s : cmGetProperty failed\n", fn ));
		goto _error;
	}

	if ((tokenNodeId = pvtGetNodeIdByPath(hVal, nodeId,
		"response.locationConfirm.tokens")) < 0)
	{
		NETDEBUG(MLRQ,NETLOG_DEBUG4,("%s : No tokenNodeId in LCF\n", fn ));
		goto _error;
	}

	newNodeId = pvtAddRoot(peerhVal,
			hPstSetup,
			0,
			NULL);

	if (newNodeId < 0)
	{
		NETERROR(MH323,
		("%s pvtAddRoot  returns error\n",fn));
		goto _error;
	}

	if(pvtAddTree(peerhVal,newNodeId,hVal,tokenNodeId) < 0)
	{
		NETERROR(MH323, ("%s pvtAddTree failed to copy token\n",fn));
		goto _freeNode;
	}

	if ((tokenNodeId = pvtGetNodeIdByPath(peerhVal,
		newNodeId,
		".tokens")) < 0)
	{
		NETERROR(MH323, ("%s could not get tokenNodeId\n",fn));
		goto _freeNode;
	}

	cmMeiExit(UH323Globals()->hApp);
	cmMeiExit(UH323Globals()->peerhApp);
	return tokenNodeId;
_freeNode:
	pvtDelete(peerhVal,newNodeId);	
_error:
	cmMeiExit(UH323Globals()->peerhApp);
	cmMeiExit(UH323Globals()->hApp);
	return -1;
}

int acfAddAuthentication(int nodeId, CallHandle *callHandle)
{
	static char 	fn[] = "acfAddAuthentication";
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	static char vt[] =  "(2) (16) (840) (1) (113777) (1) (2) (179)";
    int oidlen = 128,datalen = 0;
    char oidbuff[128]  = {0},databuff[512];
	char	callingpn[] = "2404536200";

    oidlen = oidEncodeOID(oidlen,oidbuff,vt);

	cmMeiEnter(UH323Globals()->hApp);

		if (pvtBuildByPath(hVal,
			nodeId,
			"response.admissionConfirm.tokens.1.tokenOID",
			oidlen, oidbuff) < 0)
		{
			NETERROR(MH323, ("%s could not build cryptoH323\n",fn));
		}

		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"response.admissionConfirm.tokens.1.nonStandard.nonStandardIdentifier",
			oidlen, oidbuff) < 0)
		{
			NETERROR(MH323, ("%s could not build nonStandardIdentifier\n",fn));
		}

		getVTData179(callHandle->callingPartyNumber,&datalen,databuff);
		if (pvtBuildByPath(cmGetValTree(UH323Globals()->hApp),
			nodeId,
			"response.admissionConfirm.tokens.1.nonStandard.data",
			datalen, databuff) < 0)
		{
			NETERROR(MH323, ("%s could not build nonStandardIdentifier\n",fn));
		}

		
        pvtPrintStd(hVal,nodeId,51);
        pvtPrintStd(hVal,nodeId,62);
        pvtPrintStd(hVal,nodeId,72);
        pvtPrintStd(hVal,nodeId,83);

	cmMeiExit(UH323Globals()->hApp);
	return 0;
}



#ifdef H323v30
unsigned char * stSyntax();

unsigned char * cmEmGetQ931Syntax(void)
{

	HPST		hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"q931");

	return stSyntax(hSyn);
}
#endif



void getVTData179(char *callingpn,int *datalen,char databuff[])
{
	int len;
	int i = 0,j;
	static char f1[20] = {0x60,0x25,0x00,0x16,0x30,0x0,0,0,0,0,0,0,0};
	int f1len = 5;
	//static char f2[] = {0x6025,0x0017,0x2000,0x20,0x01,0x01,0x00};
	static char f2[] = {
				0x60,0x25,0x00,0x17,0x20,0x01,0x01,
				0x60,0x25,0x00,0x18,0x20,0x01,0x00,
				0x60,0x25,0x00,0x19,0x20,0x01,0xff,
				0x60,0x25,0x00,0x1b,0x20,0x02,0x03,0x48,
				0x60,0x25,0x00,0x1a,0x20,0x01,0xa0,
				0x60,0x25,0x00,0x1f,0x20,0x01,0x04,
				};
						
	int f2len = sizeof(f2);

	static char f8[20] = {
				0x60,0x25,0x00,0x20,0x30,
				};
	int f8len = 5;
	char str8[] = "42949672.00";
	static char f9[] = {
				0x60,0x25,0x00,0x1c,0x20,0x01,0x00
				};
	int f9len = sizeof(f9);
	i = 0;
	databuff[i++] = 9;


	sprintf(f1+1+f1len,"%s",callingpn);
	f1[f1len] = strlen(callingpn);;
	f1len+=1+strlen(callingpn);

	memcpy(databuff+i,f1,f1len);
	i+=f1len;


	memcpy(databuff+i,f2,f2len);
	i+=f2len;

	sprintf(f8+1+f8len,"%s",str8);
	f8[f8len] = strlen(str8);;
	f8len+=1+strlen(str8);

	memcpy(databuff+i,f8,f8len);
	i+=f8len;


	memcpy(databuff+i,f9,f9len);
	i+=f9len;
	*datalen = i;	
}

#if 0
int getVTData232(char *callingpn,int *datalen,char databuff[])
{
	int len;
	int i = 0,j;
	static char f1[20] = {0x60,0x0c,0x00,0x8c,0x30,0x0,0,0,0,0,0,0,0};
	int f1len = 5;
	static char f2[] = {
				//calledpn
				0x60,0x0c,0x00,0x7c,0x30,0x01,0x01,
				0x60,0x0c,0x00,0x58,0x53,
				0x00,0x0c,0x00,0x59,0x30,0x00
				};
						
	int f2len = sizeof(f2);

	static char f8[20] = {
				0x60,0x25,0x00,0x20,0x30,
				};
	int f8len = 5;
	char str8[] = "42949672.00";
	static char f9[] = {
				0x60,0x25,0x00,0x1c,0x20,0x01,0x00
				};
	int f9len = sizeof(f9);
	i = 0;
	databuff[i++] = 9;


	sprintf(f1+1+f1len,"%s",callingpn);
	f1[f1len] = strlen(callingpn);;
	f1len+=1+strlen(callingpn);

	memcpy(databuff+i,f1,f1len);
	i+=f1len;


	memcpy(databuff+i,f2,f2len);
	i+=f2len;

	sprintf(f8+1+f8len,"%s",str8);
	f8[f8len] = strlen(str8);;
	f8len+=1+strlen(str8);

	memcpy(databuff+i,f8,f8len);
	i+=f8len;


	memcpy(databuff+i,f9,f9len);
	i+=f9len;
	*datalen = i;	
}
#endif

void
printData( unsigned char *data,int datalen)
{
	char str[512];
	int i;
	unsigned char ch;
	for(i=0;i<datalen;++i)
	{
		printf("%02x ",data [i]);
	}
}


