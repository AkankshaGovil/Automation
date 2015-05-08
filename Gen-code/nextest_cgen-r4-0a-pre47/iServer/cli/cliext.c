#include <stdlib.h>

#include <signal.h>
#include "cli.h"
#include "shm.h"
#include "shmapp.h"
#include "serverp.h"
#include "licenseIf.h"

void
CliSetupExtFns(void)
{
	_lsAlarmFn = lsAlarmFn;
	_CliRouteLogFn = CliRouteLogFn;
}
