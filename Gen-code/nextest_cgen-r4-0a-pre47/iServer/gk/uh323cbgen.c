#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"

#include "cm.h"
#include "uh323.h"
#include "uh323cb.h"

/* PROTOTYPES of GENERAL CALLBACKS */
int     CALLCONV cmEvNewCall(
        IN      HAPP            hApp,
        IN      HCALL           hsCall,
        IN      LPHAPPCALL      lphaCall);


int     CALLCONV cmEvRegEvent(
        IN      HAPP                hApp,
            IN      cmRegState          regState,
        IN      cmRegEvent          regEvent,
        IN      int                 regEventHandle);

SCMEVENT cmEvent={  cmEvNewCall , cmEvRegEvent };
int     CALLCONV cmEvNewCall(
        IN      HAPP            hApp,
        IN      HCALL           hsCall,
        IN      LPHAPPCALL      lphaCall)
{
}

int     CALLCONV cmEvRegEvent(
        IN      HAPP                hApp,
            IN      cmRegState          regState,
        IN      cmRegEvent          regEvent,
        IN      int                 regEventHandle)
{
}

