#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "example.h"

// The following three lines define the type2str function for all the types
// defined in example_td.h 
//
// In this example, the DEF_ENUM_2STR defines the following functions
// char * pkt2str(int typ) { definition }; 
// char * bool2str(int typ)  { definition };
//
// Note that if the second line is defined to some value then that will be
// prefixed to the definition/declaration
//
// e.g. if #define DEF_ENUM_2STR extern
// then the following functions will be defined 
// extern char * pkt2str(int typ)  { definition };
// extern char * bool2str(int typ)  { definition };

#define INCLUDE_ENUM_FILE "example_td.h"
#define DEF_ENUM_2STR
#include "t_enum.h"

int
main(int argc, char *argv[], char *envp[])
{
	bool	flag;
	pkt		ptype;
	char	cmd[160];

	printf("\njust a test program for testing bool class we created\n");
	
	for (flag=0; flag<bool_MAX; flag++) {
		printf("%d is %s\n", flag, bool2str(flag));	
	}

	printf("\njust a test program for testing pkt class we created\n");
	
	for (ptype=0; ptype<pkt_MAX; ptype++) {
		printf("%d is %s\n", ptype, pkt2str(ptype));	
	}

	printf("\nHit any key to continue....\n");
	scanf("%c", (char *)&flag);

	snprintf(cmd, 160, "/usr/bin/less %s/include/example_td.h", getenv("BASE"));
				
	system(cmd);

	return 0;
}
