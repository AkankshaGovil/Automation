// Define a typedef and functions to print it. You can also define the prorotype. */
// DEF_ENUM defines the typedef enum specified in the INCLUDE_ENUM_FILE
// DEF_ENUM_2STR defines the function for converting the above mentioned typedef value
// into string.
// DEF_ENUM_2STR_PROTO declares the above function's prototype

/* Define the class */

#if defined(DEF_ENUM)

#define ENUM_ENT(mac,val) mac = val, 
#define T_ENUM(name,fields) DEF_ENUM typedef enum { \
		fields \
		name ## _MAX \
		} name;	

#include INCLUDE_ENUM_FILE

#undef T_ENUM
#undef ENUM_ENT

#endif
#undef DEF_ENUM


/* Define how to print the class */

#if defined(DEF_ENUM_2STR)

#define ENUM_ENT(mac, val) \
	if (typ == mac) {\
		return #mac;\
	}
#define T_ENUM(name,fields) DEF_ENUM_2STR const char * name ## 2str(int typ) { \
		fields \
		return "unknown"; \
		};

#include INCLUDE_ENUM_FILE

#undef T_ENUM
#undef ENUM_ENT

#endif
#undef DEF_ENUM_2STR


/* Define prototype of how to print the class */
#if defined(DEF_ENUM_2STR_PROTO)

#define ENUM_ENT(mac, val) 
#define T_ENUM(name,fields) DEF_ENUM_2STR_PROTO const char * name ## 2str(int typ); 

#include INCLUDE_ENUM_FILE

#undef T_ENUM
#undef ENUM_ENT

#endif
#undef DEF_ENUM_PRINTPROTO


/* Undefine it so that the same macro can be used in the file again */
#undef INCLUDE_ENUM_FILE
