#ifndef _CODEMAP_H
#define _CODEMAP_H

#define CODEMAP_SHORT			"001"
#define CODEMAP_SHORTFILE		"codemap_001.dat"
#define CODEMAP_LONG			"002"
#define CODEMAP_LONGFILE		"codemap_002.dat"

#define VALID_ISDNCODE(_cc)		((_cc >= 0 && _cc < 128) ? 1 : 0)
#define VALID_SIPCODE(_rc) 		((_rc >= 400 && _rc < 700) ? 1 : 0)

#define CODEMAP_SIPINDEX(_rc) 	(_rc - 400 + 128)
#define CODEMAP_ITEMS 			428

typedef struct
{
	char hunt;			/* whether to hunt (1) or not (0) */
    char isdncode;		/* mapping to new ISDN cause code */
    int16_t sipcode;	/* mapping to new SIP response code */
} CodeMap;

#define CODEMAP_SIZE	sizeof(CodeMap)

extern CodeMap codemap[CODEMAP_ITEMS];

extern int CodeMapConfig();
extern int CodeMapReconfig();

#endif
