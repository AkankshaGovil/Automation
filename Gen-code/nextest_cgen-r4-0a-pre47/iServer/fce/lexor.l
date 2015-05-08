/***************************************************************************
                          lexor.l  -  description
                             -------------------
    begin                : Mon May 19 2003
    copyright            : (C) 2003 by bruce mattie
    email                : bmattie@nextone.com
 ***************************************************************************/



%{
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#define MFCP_PRIVATE_INTERFACE /* make mfcp.h include the private structs */
#include "mfcp.h"
#include "mfcpproto.h"
#include "fclocal.h"
#include "nsfglue.h"
#include "srvrlog.h"
#include "systemlog.h"
#include "nxosd.h"


#define LEX_SERVER 1
#define LEX_CLIENT 2

int mfcp_fd = -1;
int mfcpType = LEX_CLIENT;
MFCP_Request *rPtr = NULL;
MFCP_ParameterTypes pType;
MFCP_Session *sess;

#define YY_INPUT(buf,result,max_size)                               \
{                                                                   \
  ssize_t nb_bytes;                                                 \
  nb_bytes = recv(mfcp_fd, (void*)buf, max_size, 0);                \
  if ((nb_bytes == 0) || ((nb_bytes == -1) && (errno != EINTR))) {	\
    result = YY_NULL;                                               \
    NETDEBUG(MFCE,NETLOG_DEBUG4,("MFCP EOF"));                      \
  } else if ((nb_bytes == -1) && (errno == EINTR)) {			\
    NETDEBUG(MFCE,NETLOG_DEBUG4,("recv interrupted\n"));		\
    result = 0;                                              		\
    buf[0]='\0';                                             		\
  } else {                                                          \
    NETDEBUG(MFCE,NETLOG_DEBUG4,("Socket read nb_bytes:%d, max_size:%d, buf=%p\n",nb_bytes,max_size,buf));  \
    result = nb_bytes;                                              \
    buf[nb_bytes]='\0';                                             \
  }                                                                 \
}


%}

%option noyywrap

DIGIT    [0-9]
PARAMNAME [a-zA-Z_]+
INTEGER [\-\+0-9]

%x HEADER VERSION ID STATUS PARAMETER COMMAND EQUAL STRING VALUE DATASTR RSPID RSPTEXT
%%

<INITIAL,HEADER>"MFCP/"    {
        mfcpType = LEX_CLIENT;
        BEGIN(VERSION);
        }


<VERSION>{DIGIT}+"."{DIGIT}+ {
        BEGIN(COMMAND);
        }

<COMMAND>[ \t]* /* eat WS */

<COMMAND>RSP {
      mfcpType = LEX_CLIENT;
      BEGIN(RSPID);
      }

<COMMAND>CRS {
      rPtr = mfcp_req_new (MFCP_REQ_CRS);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>MDS {
      rPtr = mfcp_req_new (MFCP_REQ_MDS);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>CRB {
      rPtr = mfcp_req_new (MFCP_REQ_CRB);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>CRR {
      rPtr = mfcp_req_new (MFCP_REQ_CRR);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>MDR {
      rPtr = mfcp_req_new (MFCP_REQ_MDR);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>DLR {
      rPtr = mfcp_req_new (MFCP_REQ_DLR);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>DLB {
      rPtr = mfcp_req_new (MFCP_REQ_DLB);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>DLS {
      rPtr = mfcp_req_new (MFCP_REQ_DLS);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>AUS {
      rPtr = mfcp_req_new (MFCP_REQ_AUS);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>HLP {
      rPtr = mfcp_req_new (MFCP_REQ_HLP);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>PNG {
      rPtr = mfcp_req_new (MFCP_REQ_PNG);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>NTF {
      rPtr = mfcp_req_new (MFCP_REQ_NTF);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
<COMMAND>RQT {
      rPtr = mfcp_req_new (MFCP_REQ_RQT);
      mfcpType = LEX_SERVER;
      BEGIN(ID);
      }
     
<ID>[ \t]* /* eat WS */
  
<ID>{DIGIT}+ {
      rPtr->seqId = atoi(yytext);
      }

<ID>[ \t]*\n {
      BEGIN(PARAMETER);
    }
      
<RSPID>[ \t]+ /* eat WS */

<RSPID>{DIGIT}+ {
        int id;
        id = atoi(yytext);
        NETDEBUG(MFCE,NETLOG_DEBUG4,("ID from firewall %s\n",yytext)); 
        if (id > 0) {
          if ((rPtr = mfcp_req_check_expiry_lookup (sess, id, 1)) == NULL) {
            NETDEBUG(MFCE,NETLOG_DEBUG2,("No Matching ID from firewall %d\n",id)); 
          }
        }
        BEGIN(STATUS);
      }

<STATUS>[ \t]+ /* eat WS */      

<STATUS>{DIGIT}+ {
      if (rPtr) 
      	rPtr->respStatus = atoi(yytext);
      NETDEBUG(MFCE,NETLOG_DEBUG4,("Status return from firewall %s\n",yytext)); 
      BEGIN(RSPTEXT);
      }

<RSPTEXT>[ \t]*\" /* eat " */

<RSPTEXT>[^\"]* {
  if (rPtr)
     nx_strlcpy(rPtr->respString, yytext, 128);
  NETDEBUG(MFCE,NETLOG_DEBUG4,("Status text from firewall %s\n",yytext)); 
  }

<RSPTEXT>\"[ \t]*\n {
    BEGIN(PARAMETER);
}
 
<PARAMETER>"data"[ \t]*"="[ \t]*DQUOTE {
  BEGIN(DATASTR);
  }

<PARAMETER>{PARAMNAME} {
        if ((pType = mfcp_param_gettype(yytext)) == MFCP_PARAM_NONE) {
          NETERROR(MFCE,("Unknown parameter (%s)\n", yytext));
        }
      BEGIN(EQUAL);
      }

<PARAMETER>\n {
      BEGIN(HEADER);
      return(1);
      }

<DATASTR>[^\"]* {

  }

<DATASTR>\" {
  BEGIN(VALUE);  /* just to get the trailing LF */
  }

<EQUAL>"=" {
      BEGIN(VALUE);
      }

<VALUE>\" {
      BEGIN(STRING);
      }

<VALUE>{INTEGER}+ {
      NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp integer:%s\n",yytext)); 
      if (rPtr) {
      	if (mfcpType == LEX_SERVER) {
      	  mfcp_req_add_param (rPtr, pType, mfcp_value_new_i(atoi(yytext)));
        } else {
          mfcp_res_add_param (rPtr, pType, mfcp_value_new_i(atoi(yytext)));
        }
      }
    }

<VALUE>, {
      }

<VALUE>[ \t]*\n {
      BEGIN(PARAMETER);
      }

<STRING>[^\"]* {
      NETDEBUG(MFCE,NETLOG_DEBUG4,("mfcp string:%s\n",yytext)); 
      if (rPtr) {
        if (mfcpType == LEX_SERVER) {
          mfcp_req_add_param (rPtr, pType, mfcp_value_new_s (yytext));
        } else {
          mfcp_res_add_param (rPtr, pType, mfcp_value_new_s (yytext));
        }
      }
    }

<STRING>\" {
      BEGIN(VALUE);
      }

<*>.   NETDEBUG(MFCE,NETLOG_DEBUG2,("Unknown character in parser(%s)\n",yytext));

<<EOF>> {
        if (rPtr != NULL) {
          mfcp_req_free(rPtr);
        }
        pthread_mutex_lock (&sess->sessLock);  
        sess->state = MFCP_EOF;
        pthread_mutex_unlock (&sess->sessLock);  
        yyterminate();
        }

%%

MFCP_Request
* lexor_parseit(MFCP_Session *tsess) {

  if (tsess != NULL  && tsess->sessType == MFCP_SESS_TCP && (tsess->state >= MFCP_TCP_CONNECTED)) {
    sess = tsess;
    mfcp_fd = sess->fd;
    rPtr=NULL;
    mfcplex();
  }

  return rPtr;

}

