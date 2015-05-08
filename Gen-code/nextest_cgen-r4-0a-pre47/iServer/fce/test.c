/* test program for mfcp and lower */

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

#include "mfcp.h"
#include "ifs.h"



char  config_file[256] = "config.val";
char pidfile[128];
LsMemStruct lsMemData;
LsMemStruct             *lsMem = &lsMemData;
MemoryMap *map;
int keep=0;
extern char fceConfigFwName[];

struct {
  pthread_mutex_t sLock;
  int ncalls;
  int good;
  int bad;
  int hups;
} cStats;

typedef struct Leg {
	uint32_t	bid;	// Bundle Id
	uint32_t	rid;	// Resource Id	
	uint32_t	mpool;  // media Pool
	uint32_t	dip; 	// sdp ip on leg's side
	uint16_t	dport; 	// sdp port on leg's side
} Leg;

typedef struct Call {
	Leg Leg1;
	Leg Leg2;
} Call;

Call cs[100];

void normal(MFCP_Session *sess, Call *c);
void obp(MFCP_Session *sess, Call *c);

void *
delres(MFCP_Request *rPtr) {
  mfcp_req_free(rPtr);
  pthread_mutex_lock(&cStats.sLock);
  cStats.hups++;
  pthread_mutex_unlock(&cStats.sLock);
  return NULL;
}

void 
*retfunc(MFCP_Request *rPtr) {
  int resId, bunId;
  MFCP_Session *sess;
  Call *c;

  NETDEBUG(MFCE,NETLOG_DEBUG4,("in retfunc \n"));

  if (! rPtr) {
    NETERROR(MFCE,("NULL Pointer rPtr in retfunc\n"));
    return NULL;
  }
  pthread_mutex_lock(&cStats.sLock);
  if (mfcp_get_res_status(rPtr) == 200) {
    NETDEBUG(MFCE,NETLOG_DEBUG4,("IPaddr: %x\n",mfcp_get_dest_addr(rPtr)));
    NETDEBUG(MFCE,NETLOG_DEBUG4,("Port: %d\n",mfcp_get_dest_port(rPtr)));
    cStats.good++;
  } else {
    cStats.bad++;

  }
  pthread_mutex_unlock(&cStats.sLock);

  //  int mfcp_get_int(MFCP_Request *rPtr, MFCP_ParameterTypes param);
  //char *mfcp_get_str(MFCP_Request *rPtr, MFCP_ParameterTypes param);
  
  NETDEBUG(MFCE,NETLOG_DEBUG4,("ret string: %s\n",mfcp_get_res_estring(rPtr)));
  NETDEBUG(MFCE,NETLOG_DEBUG4,("status %d\n",mfcp_get_res_status(rPtr)));
  NETDEBUG(MFCE,NETLOG_DEBUG4,("type %d\n",mfcp_get_res_type(rPtr)));

  if (bunId = mfcp_get_res_appdata(rPtr)) {
     NETDEBUG(MFCE,NETLOG_DEBUG4,("Bundle Id %d call back \n", bunId));
  }
  resId = mfcp_get_int(rPtr,MFCP_PARAM_RESOURCE_ID);
//  bunId = mfcp_get_int(rPtr,MFCP_PARAM_BUNDLE_ID);
  sess = mfcp_get_sess(rPtr);

  // callid = (bundleId-1)/2
  c = &cs[(bunId-1)/2];
  if ( bunId % 2)
	  c->Leg1.rid = resId;
  else 
	  c->Leg2.rid = resId;

  NETDEBUG(MFCE,NETLOG_DEBUG4,("resId = %d, bunId = %d\n",resId,bunId));
  if (!keep) {
    if (mfcp_get_res_status(rPtr) == 200) {
      mfcp_req_dlr(sess, 0, resId,  (void *)delres,NULL);
    }
  }
  mfcp_req_free(rPtr);
  return NULL;
}

void *
close_sess(MFCP_Request *rPtr) {
  mfcp_sess_close(mfcp_get_sess(rPtr));
  return NULL;
}

int 
main (int argc, char **argv) {
  char strAddr[80];
  struct in_addr ipaddr;
  MFCP_Session *sess;
  MFCP_Request *rPtr;
  int i,j;
  int opt;
  unsigned long int mark;
  int mode;
  int ncalls;
  int ingressPool = 0;
  int egressPool = 0;
  int otherPool = 0;

  bzero(&cStats, sizeof(cStats));
  bzero(&ipaddr,sizeof(struct in_addr));
  strcpy(fceConfigFwName, "NSF");

  for (i = 0; i < 100; i++) {
	  cs[i].Leg1.bid = 2*i+1;
	  cs[i].Leg2.bid = 2*i+2;
  }

  pthread_mutex_init(&cStats.sLock,NULL);

  NetSyslogOpen(argv[0], NETLOG_TERMINAL);
  //  NETLOG_SETLEVEL(MFCE, NETLOG_DEBUG4);

  // initPoolAllocation("pools_test.xml");

  //return 0;

  lsMem->maxCalls = 2000;

  mode =0; /* async */
  ncalls =1;
  while((opt = getopt(argc,argv,"hksn:a:d:i:e:o:")) != -1) {
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
    case 'o':
      otherPool = atoi(optarg);
      break;
    case 'd':
      NETLOG_SETLEVEL(MFCE,(1>>(atoi(optarg)-1)));
    }
  }

  mfcp_init();
  sleep(4);
#ifdef _DMALLOC_
  mark = dmalloc_mark();
#endif

  sess = mfcp_sess_connect(ntohl(ipaddr.s_addr), 10, NULL);

  for (i = 1; i <= ncalls; i++) {
    pthread_mutex_lock(&cStats.sLock);
    cStats.ncalls++;
    pthread_mutex_unlock(&cStats.sLock);
    if (mode == 0) {
		Call *c = cs;

// normal call
	  c->Leg1.mpool = ingressPool; c->Leg1.dip = 0x028899aa; c->Leg1.dport=0x1000;
	  c->Leg2.mpool = egressPool; c->Leg2.dip = 0x03aabbcc; c->Leg2.dport=0x2000;
	  normal(sess, c);
	  c++;

// obp call
	  c->Leg1.mpool = ingressPool; c->Leg1.dip = 0x028899aa; c->Leg1.dport=0x1000;
	  c->Leg2.mpool = egressPool; c->Leg2.dip = 0xdeadbeef; c->Leg2.dport=0x9000;
	  c++;
	  c->Leg1.mpool = egressPool; c->Leg1.dip = 0xdeadbeef; c->Leg1.dport=0x9000;
	  c->Leg2.mpool = otherPool; c->Leg2.dip = 0x03aabbcc; c->Leg2.dport=0x2000;
	  c--;
	  obp(sess, c); 
	  c+=2;

    } else {
/*
      rPtr = mfcp_sreq_crr (sess, i, 0, 0, 0x01020304, i*10 + j + 8000, ingressPool,egressPool,"rtp");
      retfunc(rPtr);
      rPtr = mfcp_sreq_crr (sess, i, 0,0 , 0x05060708, i*10 + j + 8000, ingressPool,egressPool,"rtp");
      retfunc(rPtr);
*/
    }


    if (!(i % 500)) {
      sleep(1); /// slow it down a little
      printf("Calls made: %d\nResources created: %d\nResources deleted: %d\nResources create failed: %d\n",cStats.ncalls,cStats.good,cStats.hups,cStats.bad);
      
    }
  }
  
  if (keep) {
    printf("Enter return to continue:");
    getchar();

  }

    mfcp_sess_close(sess);
  for(i=0; i< 30; i++) {
    sleep(1);
    printf("Calls made: %d\nResources created: %d\nResources deleted: %d\nResources create failed: %d\n",cStats.ncalls,cStats.good,cStats.hups,cStats.bad);
  }
  mfcp_req_dls(sess, (void *)close_sess, NULL);

#ifdef _DMALLOC_
  dmalloc_log_changed(mark,1,0,1);
#endif

  printf("Calls made: %d\nResources created: %d\nResources deleted: %d\nResources create failed: %d\n",cStats.ncalls,cStats.good,cStats.hups,cStats.bad);
  return 0;
}

void
normal(MFCP_Session *sess, Call *c)
{
      mfcp_req_crr (sess, c->Leg1.bid, c->Leg2.rid, 0, 0, c->Leg1.dip, c->Leg1.dport,
			 		 c->Leg2.mpool, c->Leg1.mpool, "rtp", 0, 0, 0, (void *) retfunc, c->Leg1.bid);
	  usleep(10000);
      mfcp_req_crr (sess, c->Leg2.bid, c->Leg1.rid, 0, 0, c->Leg2.dip, c->Leg2.dport,
			 		 c->Leg1.mpool, c->Leg2.mpool, "rtp", 0, 0, 0, (void *) retfunc, c->Leg2.bid);
	  usleep(10000);
}

void
obp(MFCP_Session *sess, Call *c)
{
      mfcp_req_crr (sess, c->Leg1.bid, c->Leg2.rid, 0, 0, c->Leg1.dip, c->Leg1.dport,
			 		 c->Leg2.mpool, c->Leg1.mpool, "rtp", 0, 0, 0, (void *) retfunc, c->Leg1.bid);
	  usleep(10000);
      mfcp_req_chr (sess, c->Leg1.bid, c->Leg1.rid, (c+1)->Leg2.rid, 0, 0, c->Leg1.dip,
	 				 c->Leg1.dport, (c+1)->Leg2.mpool, c->Leg1.mpool, "rtp", 0, 0, 0,
					 (void *) retfunc, c->Leg1.bid);
	  usleep(10000);
      mfcp_req_crr (sess, (c+1)->Leg2.bid, (c+1)->Leg1.rid, 0, 0, (c+1)->Leg2.dip, (c+1)->Leg2.dport,
			 		 (c+1)->Leg1.mpool, (c+1)->Leg2.mpool, "rtp", 0, 0, 0, (void *) retfunc, (c+1)->Leg2.bid);
	  usleep(10000);
      mfcp_req_chr (sess, (c+1)->Leg2.bid, (c+1)->Leg2.rid, c->Leg1.bid, 0, 0, c->Leg1.dip,
			 		 c->Leg1.dport, c->Leg1.mpool, (c+1)->Leg2.mpool, "rtp", 0, 0, 0,
					 (void *) retfunc, (c+1)->Leg2.bid);
	  usleep(10000);
}
