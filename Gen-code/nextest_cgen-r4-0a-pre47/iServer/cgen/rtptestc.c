#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <ti.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <rvcommon.h>

#ifndef USE_VXD
#include <rtp.h>
#include <rtcp.h>
#include <payload.h>
#else
#include <vrtp.h>
#include <vrtcp.h>
#include <vpayload.h>
#endif

#include "rtptest.h"
/*static HWND hDlg;*/

static BOOL   multicast;
static BOOL   loop=1;

static UINT32 theTime;
static int writeToDisk = FALSE;

#ifdef _WIN32
#define GetCurrentTimeFunction GetCurrentTime
#else
#define GetCurrentTimeFunction timerGetTimeInMilliseconds
#endif

#define AUTO_RTCP

typedef struct
{
    HRTPSESSION   hRTP;
    HRTCPSESSION  hRTCP;
    int           file;
    UINT32        bytes;
    UINT32        lastTime;
    int           channels;
} rtpCtrl;


static rtpCtrl rC[100];
void rtpTestInit(void)
{
    rtpInit();
    rtcpInit();
    theTime = GetCurrentTimeFunction();
}


UINT32 rtpTestRate(int session)
{
    UINT32 rate=(rC[session].bytes * 8) / (GetCurrentTimeFunction() - rC[session].lastTime + 1);
    rC[session].lastTime = GetCurrentTimeFunction();
    rC[session].bytes = 0;
    return rate;
}

void eventHandler(HRTPSESSION hRTP, void* context)
{
    static char buff[8000];
    static i = 33333333;
    rtpCtrl*  rC = (rtpCtrl *)context;
    rtpParam p;
    int error;

#ifndef AUTO_RTCP
    error = rtpRead(hRTP, buff, 8000, &p);
#else
    error = rtpReadEx(hRTP, buff, 8000, (GetCurrentTimeFunction() - theTime) * 8, &p);
#endif

    if (error!=ERROR && rC)
    {
        rC->bytes += p.len - p.sByte - ((p.payload == H261) ? 4 : 0);
#ifndef AUTO_RTCP
        rtcpRTPPacketRecv(rC->hRTCP, p.sSrc, (GetCurrentTimeFunction() - theTime) * 8, p.timestamp, p.sequenceNumber);
#endif
        if (loop && rC->channels == 2)
        {
            /*p.timestamp=i;*/
            i+=p.len;
/*            if (writeToDisk)
            {
                write(rC->file, &(p.len), 4);
                write(rC->file, buff, p.len);
            }
 */
            rtpWrite(hRTP,buff, p.len, &p);
#ifndef AUTO_RTCP
            rtcpRTPPacketSent(rC->hRTCP, p.len, p.timestamp);
#endif
        }
    }
}


int rtpTestOpen(int session, char* name, int startPort)
{
    char buf[30];
    rC[session].channels++;
    rtpTestRate(session);
    if (rC[session].hRTP) return rtpGetPort(rC[session].hRTP);
    sprintf(buf,"rtptest%1d",session);
/*    if (writeToDisk)
        rC[session].file=open(buf,O_CREAT|O_TRUNC|O_WRONLY, S_IREAD | S_IWRITE);
*/
    rC[session].hRTP=rtpOpenEx((UINT16)(session*2+startPort),1,0xff,
#ifndef AUTO_RTCP
                    NULL
#else
                    name
#endif
        );

    rtpUseSequenceNumber(rC[session].hRTP);

    if (!rC[session].hRTCP)
#ifndef AUTO_RTCP
        rC[session].hRTCP = rtcpOpen(rtpGetSSRC(rC[session].hRTP),
                                     (UINT16)(session * 2 + startPort + 1),
                                     name);
#else
        rC[session].hRTCP = rtpGetRTCPSession(rC[session].hRTP);
#endif

    rtpSetEventHandler(rC[session].hRTP,eventHandler,NULL);
    return session*2+startPort;
}

void rtpTestSetRtp(int session,UINT32 ip,UINT16 port)
{
    rtpSetRemoteAddress(rC[session].hRTP,ip,port);
    if (multicast)
        rtpSetGroupAddress(rC[session].hRTP,ip);
    rtpSetEventHandler(rC[session].hRTP,eventHandler,&rC[session]);
}

void rtpTestSetRtcp(int session, UINT32 ip,UINT16 port)
{
    rtcpSetRemoteAddress(rC[session].hRTCP,ip,port);
    if (multicast)
        rtcpSetGroupAddress(rC[session].hRTCP,ip);

}

int rtpTestClose(int session)
{
    if (!rC[session].channels) return 0;
    rC[session].channels--;
    if (!rC[session].channels)
    {
/*        if (writeToDisk)
            close(rC[session].file);
*/
        rtpClose(rC[session].hRTP);
        rC[session].hRTP=NULL;
#ifndef AUTO_RTCP
        rtcpClose(rC[session].hRTCP);
#endif
        rC[session].hRTCP=NULL;
    }
    return 0;
}

void rtpTestEnd(void)
{
    rtcpEnd();
    rtpEnd();
}

BOOL CALLCONV enumerator(IN HRTCPSESSION hRTCP,
        IN UINT32  ssrc)
{
    char line[100];
    RTCPINFO info;
    rtcpGetSourceInfo(hRTCP, ssrc, &info);

    sprintf(line,"SSRC=%x LOST=%d CNAME=%s\r",ssrc, info.rrFrom.cumulativeLost, info.cname);

    fprintf(stdout, "%s", line);
    return FALSE;
}

void rtpTestTimer(int param)
{
    {
        static buff[812];
        static int ii=12345678;
        int i;
        /*
        rtpParam p;

          if (param==1 && !(loop && channels==2))
          {
          p.marker=1;
          p.timestamp=i;
          ii+=800;
          rtpPCMUPack(buff,812,&p,NULL);
          rtpWrite(hRTP,buff,812,&p);
          rtcpRTPPacketSent(hRTCP,p.len,p.timestamp);
          }

        **/
        if (param==2)
        {
            static j;
            j++;
            for (i=0;i<100;i++)
                if (rC[i].hRTCP)
            {
                rtcpEnumParticipants(rC[i].hRTCP,enumerator);
            }
        }
    }
}


void rtpTestWriteRTP(BOOL bwrite)
{
	writeToDisk = bwrite;
}
