// This file although may not look like a C file but it contains 
// macros which are operated on by the t_enum.h file
// A typedef enum cam be defined in the following manner. 
// The macros below emulate the following C structures 
// 
// CAUTION: do not use spaces unjudiciously in defining following macros.
// MACRO(a,b) is a macro taking two arguments
// MACRO (a,b) is a macro with no arguments

T_ENUM(pkt,
	ENUM_ENT(pkt_auth,1)
	ENUM_ENT(pkt_val,2)
	ENUM_ENT(pkt_resp,3)
	ENUM_ENT(pkt_send,4)
	ENUM_ENT(pkt_err,5)
)

T_ENUM(bool,
	ENUM_ENT(FALSE,0)
	ENUM_ENT(TRUE,1)
)


// The macro DEF_ENUM actually defines the above C typedefs.
// look at example.h for an example
// 
// typedef enum {
//		pkt_auth = 1,
//		pkt_val = 2,
//		pkt_resp = 3,
//		pkt_send = 4,
//		pkt_err = 5,
//		pkt_MAX
// } pkt;
//
// typedef enum {
// 		FALSE = 0,
//		TRUE = 1,
//		bool_MAX
// } bool;
// 
//


// The macro DEF_ENUM_2STR actually defines the following C functions.
// look at common/example.c for an example
//
//  const char *  pkt2str(int typ) {
//		if (typ ==  pkt_auth ) 	{   return "pkt_auth";  }
//		if (typ ==  pkt_val ) 	{ 	return "pkt_val";   }
//		if (typ ==  pkt_resp ) 	{   return "pkt_resp";  }
//		if (typ ==  pkt_send ) 	{   return "pkt_send";  }
//		if (typ ==  pkt_err ) 	{ 	return "pkt_err";   }
//		return "unknown"; 
//  };
//
//	const char *  bool2str(int typ) {
//		if (typ ==  FALSE ) 	{   return "FALSE"; }
//		if (typ ==  TRUE ) 		{   return "TRUE";  }
//		return "unknown";
//	};


//
// The macro DEF_ENUM_2STR_PROTO actually declares the following function prototypes.
// look at example.h for an example
//
//	const char *  pkt2str(int typ);
//	const char *  bool2str(int typ);
//
// The above prototypes were created by the #define DEF_ENUM_2STR_PROTO statement
// defining DEF_ENUM_2STR_PROTO to something else will put that as a prefix to the
// statements. e.g. - 
//
//  		#define DEF_ENUM_2STR_PROTO extern 
//
// defines the following prototypes
//
//	extern const char *  pkt2str(int typ);
//	extern const char *  bool2str(int typ);
//
// The same holds true for DEF_ENUM and DEF_ENUM_2STR macros


