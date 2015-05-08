/* Linux requires only sys/ioctl.h to be included for all the ioctl request constants and function prototype . On solaris , however, the request consants are defined in separate files (sys/filio.h sys/sockio.h etc) which should also  be included
 */
#include "osdconfig.h"
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#if HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif

#if HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
