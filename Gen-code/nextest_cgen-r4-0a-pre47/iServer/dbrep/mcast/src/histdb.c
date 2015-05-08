#include "unp.h"
#include "rs.h"
#include "hist.h"

int
main(int atgc, char **argv, char **envp)
{
	HDB		*hdbp;

	hdbp = OpenCliHist();

#if 0
	fprintf(stdout, "Acquired locks \n");
	sleep(10);
#endif

	PrintCliHist(hdbp);

	CloseCliHist(hdbp);
#if 0
	fprintf(stdout, "Releasing locks \n");
#endif

	return(0);
}
