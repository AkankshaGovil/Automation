#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include "serverp.h"
#include "srvrlog.h"
#include "license.h"
#include "licenseIf.h"

#define LICENSE_FILE "/usr/local/nextone/bin/iserver.lc"
#define MAX_LINE_SIZE 200
#define DATE_LEN 40
#define VERSION_LEN 10
#define RELEASE_LEN 20
#define NUM_FIELDS 8 /*includes signature */

extern char featureList[MAX_FEATURES][FEATURE_NAME_LEN];

char	*licenseFile = NULL;
	
int lm_getLicCount(char *release, char * version,time_t *pexpiry, char *macStr,int *pMaxCalls, int *pMRCalls)
{
	int 	found = 0;
	char 	str[2][MAX_LINE_SIZE] = {0};
	char 	tokens[NUM_FIELDS][MAX_LINE_SIZE];
	unsigned char 	tmpstr[2*MAX_LINE_SIZE],*q;
	unsigned char sign[MAX_LINE_SIZE];
	char 	*p;
	unsigned char 	macstr[MAC_ADDR_LEN];
	time_t 	now,exp;
	char 	date[DATE_LEN];
	struct 	tm tm_now, tm_exp;
	FILE 	*fp,*fp2;
	int 	i,n,j;
	char 	ch;
	double	diff;
	int 	nlic;
	int		siglen;
	int 	sigstr;
	int 	maclen;
	
	if(licenseFile == NULL)
	{
		licenseFile = LICENSE_FILE;
	}

	if (!(fp = fopen(licenseFile,"r")))
	{
		ERROR(MLMGR, ("Error Opening license file\n"));
		return -1;
	}


	for(i=0,q=tmpstr;i <2 && !feof(fp); ++i)
	{
		fscanf(fp,"%[^'\n']",str[i]);
		/* discard new line*/
		ch = fgetc(fp);
		sprintf(q,"%s\n",str[i]);
		q+=strlen(q);
	}
	
	if(!strstr(str[0],release) && strstr(str[0],version))
	{
		ERROR(MLMGR, ("Error no line containing release name\n"));
		return -1;
	}

	if(!strstr(str[1],"Features"))
	{
		ERROR(MLMGR, ("Error no line containing feature list\n"));
		return -1;
	}

	for(i = 0, p = str[0]; i<NUM_FIELDS-1; ++i, p+=n+1)
	{
		while( isspace(*p))
		{
			p++;
		}
		n = strcspn(p," '\n''\t'");
			
		if (!n)
		{	
			ERROR(MLMGR, ("License File corrupt \n\t"));
			return -1;
		}
		strncpy (tokens[i],p,n);
		tokens[i][n] = '\0'; /* null terminate the string */
		DEBUG(MLMGR,NETLOG_DEBUG4,("Token %d = %s! n = %d\n",
			i,tokens[i],n));
	}
	
	
	fscanf(fp,"%d",&siglen);
	fgetc(fp); /* skip the blank */


	fread(sign,1,siglen,fp);

	if( lm_verify((unsigned char *)tmpstr,strlen(tmpstr),sign,siglen) != 0 )
	{
		ERROR(MLMGR, ("Signature not verified\n"));
		return -1;
	}

	DEBUG(MLMGR,NETLOG_DEBUG4,("Signature verified\n"));

	/*  check for non expiring */	
	if(strcmp("timeless",tokens[2]))
	{	
		/* Not a timeless license. check for expiry date */
		now = time(&now);
		memset(&tm_exp,0,sizeof(tm_exp));
	
		sscanf(tokens[2],"%d%c%d%c%d",
			&tm_exp.tm_mon,&ch,&tm_exp.tm_mday,&ch,&tm_exp.tm_year);
		
		DEBUG(MLMGR,NETLOG_DEBUG4,("Time read = %d %d %d\n",
			tm_exp.tm_mon,tm_exp.tm_mday,tm_exp.tm_year));

		tm_exp.tm_year -= 1900;
		exp = mktime(&tm_exp);
	
		if(exp == (time_t)-1)
		{
		
			ERROR(MLMGR, ("Invalid Expiry time\n"));
			*pexpiry = now;
			return -1;
		}

		*pexpiry = exp;

		diff = difftime(exp,now);
	
		if(diff < 0)
		{
			ERROR(MLMGR,("Error License expired at %s",
				ctime(&exp)));
			return  -1;
		}
	

	} else 	
		{
			*pexpiry = 0; /* never expires */
		}


	/* verify Mac;*/
	if(strncmp(tokens[5],"unbound",strlen("unbound")))
	{
		DEBUG(MLMGR,NETLOG_DEBUG4,
			("Node locked license.Verifying Mac\n"));
		/* Its a nodelocked license */
#ifndef SUNOS
		for(p = tokens[5],i=0; p && *p!='\0';p = strchr(p,':'),++i)
		{
			if(*p == ':') p++;
			sscanf(p,"%x",&n);
			macstr[i] = n; 
		}
		maclen = i;
				
		if(validateHwAddress(maclen, macstr) != 0) 
#else
		if(validateHostid(tokens[5])!=0)
#endif

		{
			ERROR(MLMGR,
				("Error mac address does not match host mac"));
			macStr[0] = '\0';
			return -1;
		}
		strcpy(macStr,tokens[5]);
	}
	else
		{
			strcpy(macStr,"unbound");
		}
	
	nlic = atoi(tokens[3]);	
	*pMaxCalls = atoi(tokens[4]);
        *pMRCalls = atoi(tokens[6]);

	lsMem->features = 0;
	for(i = 0, p = str[1];; ++i, p+=n+1)
	{
		while( isspace(*p))
		{
			p++;
		}
		n = strcspn(p," '\n''\t'");
			
		if (!n)
		{	
			break;
		}
		strncpy (tokens[i],p,n);
		tokens[i][n] = '\0'; /* null terminate the string */
		DEBUG(MLMGR,NETLOG_DEBUG4,("Token %d = %s! n = %d\n",
			i,tokens[i],n));
		for(j = 0; j <MAX_FEATURES;++j)
		{
			if(!strcasecmp(featureList[j],tokens[i]))
			{
				lsMem->features |= 1<<j;
				break;
			}
		}
	}

	if(!fceEnabled())
	{
		*pMRCalls = 0;
	}
	DEBUG(MLMGR,NETLOG_DEBUG4,
		("Verified license file. Max Endpoints = %d. MaxCalls = %d MediaRouted %d\n",
		nlic,*pMaxCalls,*pMRCalls));

	DEBUG(MLMGR,NETLOG_DEBUG4,("features = %x\n",lsMem->features));
	return nlic;

}
