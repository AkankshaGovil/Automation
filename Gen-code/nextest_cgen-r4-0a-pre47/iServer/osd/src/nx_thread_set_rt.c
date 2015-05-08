#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#if HAVE_PRIOCNTL
#include <sys/priocntl.h>
#include <sys/rtpriocntl.h>
#include <sys/tspriocntl.h>
#else 
#include <sched.h>
#endif

#if HAVE_PRIOCNTL

static	int32_t prio_inited = 0;

static  void
get_priocntl_info( pcinfo_t *prt_pcinfo )
{

        if ( !prio_inited )
        {
                (void) strcpy( prt_pcinfo->pc_clname, "RT" );
                priocntl(       (int32_t) 0,
                                        (idtype_t) 0,
                                        (id_t) PC_GETCID,
                                        (caddr_t) prt_pcinfo);

                prio_inited = 1;
        }

        return;
}

void
nx_thread_set_rt(int32_t ns_quantum)
{
	lwpid_t     lwpid = _lwp_self();
	pcparms_t   pcparms;
	pcinfo_t	rt_pcinfo;
	rtparms_t   *rtparmsp = (struct rtparms *) pcparms.pc_clparms;
	rtinfo_t    *rtinfop = (struct rtinfo *) rt_pcinfo.pc_clinfo;

	get_priocntl_info(&rt_pcinfo);

	memset( &pcparms, (int32_t) 0, sizeof(pcparms_t) );

	pcparms.pc_cid = rt_pcinfo.pc_cid;
	rtparmsp->rt_pri = rtinfop->rt_maxpri - 1;
	rtparmsp->rt_tqsecs = 0;
	rtparmsp->rt_tqnsecs = ns_quantum;
		
	if ( priocntl(  (int32_t) P_LWPID, (idtype_t) lwpid, (id_t) PC_SETPARMS, (caddr_t) &pcparms ) == -1L )
	{
		return;
	}
     return;
}

#else 

void
nx_thread_set_rt(int32_t ns_quantum)
{
	struct sched_param param;
	pid_t pid = getpid();
	param.sched_priority = sched_get_priority_max(SCHED_RR) - 1;
	if(sched_setscheduler(pid, SCHED_RR, &param)){
		return;
	}
	return;
}
#endif
