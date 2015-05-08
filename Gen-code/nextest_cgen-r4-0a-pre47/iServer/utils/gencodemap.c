#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "codemap.h"

#define VALID_HUNTSTR(_hs) (!strcasecmp(_hs, "yes") || !strcasecmp(_hs, "no"))

/* input config values from ASCII file */
typedef struct {
	int16_t prevorigcode;		/* previous value of orig code to track order */
	int16_t origcode;			/* current value of orig code */
    char huntstr[8];			/* hunt string */
    int16_t isdncode;			/* new ISDN code */
    int16_t sipcode;			/* new SIP code */
} InputCodeMap;

static InputCodeMap	inmap;		/* input codemap read from ASCII file */
static CodeMap outmap;			/* output codemap written to binary file */
static char txtfilename[32];	/* name of asii file */
static char datfilename[32];	/* name of binary file */
static FILE *txtfile;			/* file descriptor for ASCII file */
static FILE *datfile;			/* file descriptor for binary file */
static int lineno;				/* line number of ASCII file */
static int32_t checksum;		/* checksum of data in binary file */

static int processCodeCfg();		/* process configuration */
static int readConfig();			/* read config line from ASCII file */
static int writeConfig();			/* write config line to binary file */
static int updateChecksum(int i);	/* update checksum */
static int findIsdnCodeCfg();		/* locate start of ISDN config */
static int checkIsdnCodeCfg();		/* validate ISDN config */
static int findSipCodeCfg();		/* locate start of SIP config */
static int checkSipCodeCfg();		/* validate SIP config */
static int appendChecksum();		/* append checksum to end of binary file */

int main(int argc, char **argv)
{
    /* get configuration template (001, 002, etc.) */
    if (argc < 2)
	{
    	fprintf(stderr, "Error: Insufficient arguments\n");
    	fprintf(stderr, "Usage: gencodemap <template eg. 001, 002, etc.>\n");
		return -1;
	}

    /* construct filenames */
    snprintf(txtfilename, sizeof(txtfilename), "codemap_%s.txt", argv[1]);
    snprintf(datfilename, sizeof(datfilename), "codemap_%s.dat", argv[1]);

    /* open ASCII file for reading */
	if (!(txtfile = fopen(txtfilename, "r")))
  	{ 
    	fprintf(stderr, "Could not open %s: %s\n", txtfilename, strerror(errno));
    	return -1;
  	} 

	/* open binary file for writing */
	if (!(datfile = fopen(datfilename, "w")))
  	{ 
    	fprintf(stderr, "Could not open %s: %s\n", datfilename, strerror(errno));
		fclose(txtfile);
    	return -1;
  	} 

	/* process code config from ASCII file and write to binary file */
	if (processCodeCfg() < 0)
	{
    	fprintf(stderr, "Could not process code configuration from file %s\n", 
			txtfilename);
		fclose(txtfile);
		fclose(datfile);
		return -1;
	}

	/* close files */
	fclose(txtfile);
	fclose(datfile);

	return 0;
}

/* Read configuration from ASCII file and write to binary file */
static int
processCodeCfg()
{
	int i;

	/* find beginning og ISDN code config in ASCII file */
	if (findIsdnCodeCfg() < 0)
		return -1;

	/* read ISDN code config, validate it and write it to the binary file */
    for(i = 0, inmap.prevorigcode = -1; i < 128; i++, inmap.prevorigcode++)
    {
        if (readConfig() < 0)
			return -1;

        if (checkIsdnCodeCfg() < 0)
			return -1;

        if (writeConfig() < 0)
			return -1;

		updateChecksum(i * CODEMAP_SIZE);
    }

	/* find beginning of SIP code config in ASCII file */
	if (findSipCodeCfg() < 0)
		return -1;

	/* read SIP code config, validate it and write it to the binary file */
    for(i = 400, inmap.prevorigcode = 399; i < 700; i++, inmap.prevorigcode++)
    {
        if (readConfig() < 0)
			return -1;

        if (checkSipCodeCfg() < 0)
			return -1;

        if (writeConfig() < 0)
			return -1;

		updateChecksum(CODEMAP_SIPINDEX(i) * CODEMAP_SIZE);
    }

	/* append checksum to binary file */
	if (appendChecksum() < 0)
		return -1;

	return 0;
}

/* Locate start of ISDN code config columns in ASCII file */
static int 
findIsdnCodeCfg()
{
	char buf[BUFSIZ];

	while(fgets(buf, BUFSIZ, txtfile))
	{
		lineno++;
		if (strstr(buf, "OrigIsdnCauseCode HuntOnIsdnCauseCode NewIsdnCauseCode NewSipResponseCode"))
		{
			fgets(buf, BUFSIZ, txtfile);	/* skip one line */
			lineno++;
			return 0;
		}
	}

   	fprintf(stderr, "Could not find ISDN code configuration in file %s\n", 
		txtfilename);

	return -1;
}

/* Validate ISDN configuration read from ASCII file */
static int 
checkIsdnCodeCfg()
{
	if (inmap.origcode != (inmap.prevorigcode + 1))
   	{
          fprintf(stderr,
              "OrigIsdnCauseCode on line %d in file %s is out of order\n",
              lineno, txtfilename);
   	}
	else if (!VALID_ISDNCODE(inmap.origcode))
   	{
    	fprintf(stderr,
        	"Invalid value for OrigIsdnCauseCode on line %d in file %s\n",
            lineno, txtfilename);
        fprintf(stderr, "Valid values are 0 through 127\n");
    }
	else if (!VALID_ISDNCODE(inmap.isdncode))
   	{
       	fprintf(stderr,
           	"Invalid value for NewIsdnCauseCode on line %d in file %s\n",
           	lineno, txtfilename);
        fprintf(stderr, "Valid values are 0 through 127\n");
   	}
	else if (!VALID_SIPCODE(inmap.sipcode))
   	{
       	fprintf(stderr,
           	"Invalid value for NewSipResponseCode on line %d in file %s\n",
           	lineno, txtfilename);
        fprintf(stderr, "Valid values are 400 through 699\n");
   	}
	else if (!VALID_HUNTSTR(inmap.huntstr))
    {
        fprintf(stderr,
            "Invalid string for HuntOnIsdnCauseCode on line %d in file %s\n",
            lineno, txtfilename);
        fprintf(stderr, "Valid strings are yes/YES or no/NO\n");
    }
	else
	{
		return 0;
	}

	return -1;
}

/* Read a config line into inmap */
static int
readConfig()
{
	lineno++;

	if (fscanf(txtfile, "%hd%s%hd%hd\n",
        &(inmap.origcode), inmap.huntstr,
        &(inmap.isdncode), &(inmap.sipcode)) != 4)
    {
        fprintf(stderr, "Error while reading line %d from file %s\n",
        	lineno, txtfilename);
        return -1;
   	}


	return 0;
}

/* Write a config line to the binary file */
static int
writeConfig()
{
	/* update outmap */
	outmap.hunt = !strcasecmp(inmap.huntstr, "yes") ? 1 : 0;
   	outmap.isdncode = (char) (inmap.isdncode);
   	outmap.sipcode = htons(inmap.sipcode);

	/* write config to binary file */
	if (fwrite(&outmap, CODEMAP_SIZE, 1, datfile) != 1)
	{
    	fprintf(stderr, "Error while writing configuration to file %s\n", 
			datfilename);
       	return -1;
	}

	return 0;
}

/* Update the checksum to be appended to the binary file */
/* XOR each byte with its position in the binary file */
static int
updateChecksum(int pos)
{
	int i;

	for (i = 0; i < CODEMAP_SIZE; i++, pos++)
		checksum += (((char *) &outmap)[i] ^ pos);

	return 0;
}

/* Locate start of SIP code config columns in ASCII file */
static int 
findSipCodeCfg()
{
	char buf[BUFSIZ];

	while(fgets(buf, BUFSIZ, txtfile))
	{
		lineno++;
		if (strstr(buf, "OrigSipResponseCode HuntOnSipResponseCode NewIsdnCauseCode NewSipResponseCode"))
		{
			fgets(buf, BUFSIZ, txtfile);	/* skip one line */
			lineno++;
			return 0;
		}
	}

   	fprintf(stderr, "Could not find SIP code configuration in file %s\n", 
		txtfilename);

	return -1;
}

/* Validate SIP configuration read from ASCII file */
static int 
checkSipCodeCfg()
{
	if (inmap.origcode != (inmap.prevorigcode + 1))
   	{
          fprintf(stderr,
              "OrigSipResponseCode on line %d in file %s is out of order\n",
              lineno, txtfilename);
   	}
	else if (!VALID_SIPCODE(inmap.origcode))
   	{
    	fprintf(stderr,
        	"Invalid value for OrigSipResponseCode on line %d in file %s\n",
            lineno, txtfilename);
        fprintf(stderr, "Valid values are 400 through 699\n");
    }
	else if (!(VALID_ISDNCODE(inmap.isdncode) || inmap.isdncode == -1))
   	{
       	fprintf(stderr,
           	"Invalid value for NewIsdnCauseCode on line %d in file %s\n",
           	lineno, txtfilename);
        fprintf(stderr, "Valid values are 0 through 127 or -1\n");
   	}
	else if (!VALID_SIPCODE(inmap.sipcode))
   	{
       	fprintf(stderr,
           	"Invalid value for NewSipResponseCode on line %d in file %s\n",
           	lineno, txtfilename);
        fprintf(stderr, "Valid values are 400 through 699\n");
   	}
	else if (!VALID_HUNTSTR(inmap.huntstr))
    {
        fprintf(stderr,
            "Invalid string for HuntOnSipResponseCode on line %d in file %s\n",
            lineno, txtfilename);
        fprintf(stderr, "Valid strings are yes/YES or no/NO\n");
    }
	else
	{
		return 0;
	}

	return -1;
}

/* Append a checksum to the end of the binary file */
static int
appendChecksum()
{
	checksum = htonl(checksum);		/* convert to network byte order */

	if (fwrite(&checksum, sizeof(int), 1, datfile) != 1)
	{
    	fprintf(stderr, "Error while writing checksum to file %s\n", 
			datfilename);
       	return -1;
	}

	return 0;
}
