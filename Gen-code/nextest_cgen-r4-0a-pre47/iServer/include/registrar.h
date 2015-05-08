
#ifndef _registrar_h_
#define _registrar_h_


#define REGEX_STRING_LEN	48
#define REGEX_COMPILED_STRING_LEN  64


typedef struct _RuleTableStruct
{
	IPaddr	ipaddr;
	char	regexstring[REGEX_STRING_LEN];
	char	regexcomp[REGEX_COMPILED_STRING_LEN];
} RuleTableStruct;


#endif /* _registrar_h_ */
