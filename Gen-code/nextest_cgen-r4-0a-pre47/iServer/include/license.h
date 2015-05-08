#ifndef _LICENSE_H_
#define _LICENSE_H_

#define FEATURE "iServer"
#define MAC_ADDR_LEN 12

int 	license_init(void);
int 	license_allocate(int n);
void 	license_release(int n);
void 	license_display(void);
void 	nlm_setconfigport(int n);
int 	nlm_getconfigport(void);
/* expects to be called from inside confCache Locks */
int 	nlm_getvport(void);
int	nlm_getUsedvportNolock (void);
void	writeXmlLicense (void);

#endif  
