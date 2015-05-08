

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "serverp.h"
#include "srvrlog.h"
#include "systemlog.h"
#include <stdlib.h>

#define MFCP_PRIVATE_INTERFACE /* make mfcp.h include the private structs */
#include "mfcp.h"
#include "mfcpproto.h"
#include "fclocal.h"
#include "nsfglue.h"
#include "srvrlog.h"
#include "systemlog.h"


char  config_file[256] = "config.val";
char pidfile[128];
LsMemStruct lsMemData;
LsMemStruct             *lsMem = &lsMemData;
MemoryMap *map;
int keep=0;


void *callBack(void *rPtr) {


}

int 
main (int argc, char **argv) {
  char strAddr[80];
  struct in_addr ipaddr;
  MFCP_Session *sess;
  MFCP_Request *rPtr;
  int lfd;
  int opt;

  int mode;
  int ncalls;
  int ingressPool = 0;
  int egressPool = 0;
  MFCP_Return ret;

  bzero(&ipaddr,sizeof(struct in_addr));

  NetSyslogOpen(argv[0], NETLOG_TERMINAL);

  lsMem->maxCalls = 2000;

  mode =0; /* async */
  ncalls =1;
  while((opt = getopt(argc,argv,"hksn:a:d:i:e:")) != -1) {
    switch (opt) {
    case 'a':
      strcpy(strAddr,optarg);
      inet_pton(AF_INET,strAddr,&ipaddr);
      break;
    case 'h':
      break;
    case 'k':
      keep=1; // keep the calls up
      break;
    case 'n':
      ncalls = atoi(optarg);
      break;
    case 's':
      mode = 1;
      break;
    case 'i':
      ingressPool = atoi(optarg);
      break;
    case 'e':
      egressPool = atoi(optarg);
      break;
    case 'd':
      NETLOG_SETLEVEL(MFCE,(1>>(atoi(optarg)-1)));
    }
  }

  nsfGlueInit(); /* init the nsf since we're local */
  lfd = mfcp_sess_listen(INADDR_ANY,MFCP_PORT);

  while(1) {
    sess = mfcp_sess_accept(lfd);
    NETDEBUG(MFCE,NETLOG_DEBUG4,("accepted mfcp connection\n"));
    while(1) {
      rPtr = lexor_parseit(sess);
      if (rPtr != NULL) {
	mfcp_req_display(rPtr,"main");
	rPtr->sess = sess;
	fc_dorequest(rPtr);
	//      mfcp_req_callback(rPtr,(void *)callBack,NULL);
	// ret = mfcp_sess_send_res (sess, rPtr);
	//      fc_dorequest(rPtr);
	//      mfcp_req_rsp(sess);      
      } else {
	mfcp_logit("error from parse");
	close(sess->fd);
	mfcp_sess_free(sess);
	break;
      }
    }
  }


}










