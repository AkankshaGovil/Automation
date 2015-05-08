//
//	The following lines define the typedef and function 
//	prorotypes
//
//  example_td.h is the file where typedef in defined in terms of
//  the macros T_ENUM and ENUM_ENT. This file has to be included first. 
//
//  DEF_ENUM specifies that the typedef has to be defined here
//
//  DEF_ENUM_2STR_PROTO specifies the prototypes have to be declared here
//  and they should be extern
//
//  t_enum.h include file operates on these macros and defines the appropriate
//  C expressions
 

#define INCLUDE_ENUM_FILE "example_td.h"
#define DEF_ENUM
#define DEF_ENUM_2STR_PROTO extern
#include "t_enum.h"
