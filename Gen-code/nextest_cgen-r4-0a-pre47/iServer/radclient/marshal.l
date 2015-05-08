%option noyywrap

%{
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "radacct.h"

AccountingInfo *p;

pthread_mutex_t radacctLexMutex = PTHREAD_MUTEX_INITIALIZER;

%}

%s aA aSI aST aSTe cST cCT cET cO cSP cT cdSI cgSI cNP cI cIo dC
%s fTD gwRC gkXC gwFXC gwI iCI nI nIP nP nPT oA pS rA rMI sT sub tG

%%

"<aA>" { BEGIN aA; }
<aA>.*/"</aA>" { p->acctAuthentic = strdup(yytext); BEGIN INITIAL; }
"</aA>" { BEGIN INITIAL; }

"<aSI>" { BEGIN aSI; }
<aSI>.*/"</aSI>" { p->acctSessionId = strdup(yytext); BEGIN INITIAL; }
"</aSI>" { BEGIN INITIAL; }

"<aST>" { BEGIN aST; }
<aST>.*/"</aST>" { p->acctSessionTime = strdup(yytext); BEGIN INITIAL; }
"</aST>" { BEGIN INITIAL; }

"<aSTe>" { BEGIN aSTe; }
<aSTe>.*/"</aSTe>" { p->acctStatusType = strdup(yytext); BEGIN INITIAL; }
"</aSTe>" { BEGIN INITIAL; }

"<cST>" { BEGIN cST; }
<cST>.*/"</cST>" { p->callStartTime = strdup(yytext); BEGIN INITIAL; }
"</cST>" { BEGIN INITIAL; }

"<cCT>" { BEGIN cCT; }
<cCT>.*/"</cCT>" { p->callConnectTime = strdup(yytext); BEGIN INITIAL; }
"</cCT>" { BEGIN INITIAL; }

"<cET>" { BEGIN cET; }
<cET>.*/"</cET>" { p->callEndTime = strdup(yytext); BEGIN INITIAL; }
"</cET>" { BEGIN INITIAL; }

"<cO>" { BEGIN cO; }
<cO>.*/"</cO>" { p->callOrigin = strdup(yytext); BEGIN INITIAL; }
"</cO>" { BEGIN INITIAL; }

"<cSP>" { BEGIN cSP; }
<cSP>.*/"</cSP>" { p->callSessionProtocol = strdup(yytext); BEGIN INITIAL; }
"</cSP>" { BEGIN INITIAL; }

"<cT>" { BEGIN cT; }
<cT>.*/"</cT>" { p->callType = strdup(yytext); BEGIN INITIAL; }
"</cT>" { BEGIN INITIAL; }

"<cdSI>" { BEGIN cdSI; }
<cdSI>.*/"</cdSI>" { p->calledStationId = strdup(yytext); BEGIN INITIAL; }
"</cdSI>" { BEGIN INITIAL; }

"<cgSI>" { BEGIN cgSI; }
<cgSI>.*/"</cgSI>" { p->callingStationId = strdup(yytext); BEGIN INITIAL; }
"</cgSI>" { BEGIN INITIAL; }

"<cNP>" { BEGIN cNP; }
<cNP>.*/"</cNP>" { p->ciscoNasPort = strdup(yytext); BEGIN INITIAL; }
"</cNP>" { BEGIN INITIAL; }

"<cI>" { BEGIN cI; }
<cI>.*/"</cI>" { p->confId = strdup(yytext); BEGIN INITIAL; }
"</cI>" { BEGIN INITIAL; }

"<cIo>" { BEGIN cIo; }
<cIo>.*/"</cIo>" { p->connectInfo = strdup(yytext); BEGIN INITIAL; }
"</cIo>" { BEGIN INITIAL; }

"<dC>" { BEGIN dC; }
<dC>.*/"</dC>" { p->cause = strdup(yytext); BEGIN INITIAL; }
"</dC>" { BEGIN INITIAL; }

"<fTD>" { BEGIN fTD; }
<fTD>.*/"</fTD>" { p->faxTxDuration = strdup(yytext); BEGIN INITIAL; }
"</fTD>" { BEGIN INITIAL; }

"<gwRC>" { BEGIN gwRC; }
<gwRC>.*/"</gwRC>" { p->gwRxdCdn = strdup(yytext); BEGIN INITIAL; }
"</gwRC>" { BEGIN INITIAL; }

"<gkXC>" { BEGIN gkXC; }
<gkXC>.*/"</gkXC>" { p->gkXlatedCdn = strdup(yytext); BEGIN INITIAL; }
"</gkXC>" { BEGIN INITIAL; }

"<gwFXC>" { BEGIN gwFXC; }
<gwFXC>.*/"</gwFXC>" { p->gwFinalXlatedCdn = strdup(yytext); BEGIN INITIAL; }
"</gwFXC>" { BEGIN INITIAL; }

"<gwI>" { BEGIN gwI; }
<gwI>.*/"</gwI>" { p->gwId = strdup(yytext); BEGIN INITIAL; }
"</gwI>" { BEGIN INITIAL; }

"<iCI>" { BEGIN iCI; }
<iCI>.*/"</iCI>" { p->incomingConfId = strdup(yytext); BEGIN INITIAL; }
"</iCI>" { BEGIN INITIAL; }

"<nI>" { BEGIN nI; }
<nI>.*/"</nI>" { p->nasIdentifer = strdup(yytext); BEGIN INITIAL; }
"</nI>" { BEGIN INITIAL; }

"<nIP>" { BEGIN nIP; }
<nIP>.*/"</nIP>" { p->nasIpaddr = strdup(yytext); BEGIN INITIAL; }
"</nIP>" { BEGIN INITIAL; }

"<nP>" { BEGIN nP; }
<nP>.*/"</nP>" { p->nasPort = strdup(yytext); BEGIN INITIAL; }
"</nP>" { BEGIN INITIAL; }

"<nPT>" { BEGIN nPT; }
<nPT>.*/"</nPT>" { p->nasPortType = strdup(yytext); BEGIN INITIAL; }
"</nPT>" { BEGIN INITIAL; }

"<oA>" { BEGIN oA; }
<oA>.*/"</oA>" { p->outgoingArea = strdup(yytext); BEGIN INITIAL; }
"</oA>" { BEGIN INITIAL; }

"<pS>" { BEGIN pS; }
<pS>.*/"</pS>" { p->proxyState = strdup(yytext); BEGIN INITIAL; }
"</pS>" { BEGIN INITIAL; }

"<rA>" { BEGIN rA; }
<rA>.*/"</rA>" { p->remoteAddr = strdup(yytext); BEGIN INITIAL; }
"</rA>" { BEGIN INITIAL; }

"<rMI>" { BEGIN rMI; }
<rMI>.*/"</rMI>" { p->remoteMediaIpaddr = strdup(yytext); BEGIN INITIAL; }
"</rMI>" { BEGIN INITIAL; }

"<sT>" { BEGIN sT; }
<sT>.*/"</sT>" { p->serviceType = strdup(yytext); BEGIN INITIAL; }
"</sT>" { BEGIN INITIAL; }

"<sub>" { BEGIN sub; }
<sub>.*/"</sub>" { p->subscriber = strdup(yytext); BEGIN INITIAL; }
"</sub>" { BEGIN INITIAL; }

"<tG>" { BEGIN tG; }
<tG>.*/"</tG>" { p->tg = strdup(yytext); BEGIN INITIAL; }
"</tG>" { BEGIN INITIAL; }

. { BEGIN INITIAL; }

%%

void unMarshalAccountingInfo(char *buf, AccountingInfo *data)
{
	pthread_mutex_lock(&radacctLexMutex);

	yy_scan_string(buf);

	p = data;

	yylex();

	yy_delete_buffer(YY_CURRENT_BUFFER);

	pthread_mutex_unlock(&radacctLexMutex);
}


void marshalAccountingInfo(AccountingInfo *data, char *buf, int size)
{
	memset(buf, 0, size);

	snprintf(buf, size,
			"<aA>%s</aA> " \
			"<aSI>%s</aSI> " \
			"<aST>%s</aST> " \
			"<aSTe>%s</aSTe> " \
			"<cST>%s</cST> " \
			"<cCT>%s</cCT> " \
			"<cET>%s</cET> " \
			"<cO>%s</cO> " \
			"<cSP>%s</cSP> " \
			"<cT>%s</cT> " \
			"<cdSI>%s</cdSI> " \
			"<cgSI>%s</cgSI> " \
			"<cNP>%s</cNP> " \
			"<cI>%s</cI> " \
			"<cIo>%s</cIo> " \
			"<dC>%s</dC> " \
			"<fTD>%s</fTD> " \
			"<gwRC>%s</gwRC> " \
			"<gkXC>%s</gkXC> " \
			"<gwFXC>%s</gwFXC> " \
			"<gwI>%s</gwI> " \
			"<iCI>%s</iCI> " \
			"<nI>%s</nI> " \
			"<nIP>%s</nIP> " \
			"<nP>%s</nP> " \
			"<nPT>%s</nPT> " \
			"<oA>%s</oA> " \
			"<pS>%s</pS> " \
			"<rA>%s</rA> " \
			"<rMI>%s</rMI> " \
			"<sT>%s</sT> " \
			"<sub>%s</sub> " \
			"<tG>%s</tG>",
			data->acctAuthentic,
			data->acctSessionId,
			data->acctSessionTime,
			data->acctStatusType,
			data->callStartTime,
			data->callConnectTime,
			data->callEndTime,
			data->callOrigin,
			data->callSessionProtocol,
			data->callType,
			data->calledStationId,
			data->callingStationId,
			data->ciscoNasPort,
			data->confId,
			data->connectInfo,
			data->cause,
			data->faxTxDuration,
			data->gwRxdCdn,
			data->gkXlatedCdn,
			data->gwFinalXlatedCdn,
			data->gwId,
			data->incomingConfId,
			data->nasIdentifer,
			data->nasIpaddr,
			data->nasPort,
			data->nasPortType,
			data->outgoingArea,
			data->proxyState,
			data->remoteAddr,
			data->remoteMediaIpaddr,
			data->serviceType,
			data->subscriber,
			data->tg);
}
