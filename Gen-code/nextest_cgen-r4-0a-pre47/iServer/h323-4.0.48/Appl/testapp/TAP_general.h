#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/********************************************************************************************
 *                                general.h
 *
 *
 * This file provides general functions for the Test Application.
 * These functions are used for initiating the test application.
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/



#ifndef _GENERAL_H
#define _GENERAL_H

#include <tcl.h>
#include <tk.h>
#include "cm.h"


/********************************************************************************************
 * String2Alias
 * purpose : change strinf into RV_Common_AliasAddress
 * input   : str - the alias string from the listbox of NewCall
 *         : Alias - the target struct
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int String2Alias(char* str,cmAlias* Alias);


/********************************************************************************************
 * BuildAliasList
 * purpose : make array of RV_Common_AliasAddress from array of char*
 * input   : tclList   - TCL list value
 * output  : none
 * return  : RV_Common_AliasAddress - the target array
 ********************************************************************************************/
cmAlias ** BuildAliasList(cmAlias * * AliasArray, int AliasListSize, char * * AliasList);


/********************************************************************************************
 * FreeAliasList
 * purpose : Free an array created by BuildAliasList
 * input   : aliasArray - Alias array to free
 * output  : none
 * return  : none
 ********************************************************************************************/
void FreeAliasList(cmAlias** aliasArray);

/********************************************************************************************
 * Alias2String
 * purpose : make array of strings from array of RV_Common_AliasAddress **
 * input   : Alias - the source alias
 *         : String - the result string (should be allocated)
 * output  : String - the result string
 * return  : char* - the result string
 ********************************************************************************************/
char * Alias2String( cmAlias * Alias, char * String);

int Quit(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/******************************************************************************************
 * TclExecute
 * purpose : execute a command in tcl
 * input   : cmd - the command that is going to be executed
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ******************************************************************************************/
int TclExecute(const char* cmd, ...);


/******************************************************************************************
 * TclString
 * purpose : make a given string a printable TCL string
 * input   : line    - Line to convert
 * output  : none
 * return  : Converted string
 ******************************************************************************************/
char* TclString(const char* line);


/******************************************************************************************
 * TclGetVariable
 * purpose : get variable from tcl
 * input   : varName - the name of the variable that is going to be imported
 * input   : none
 * output  : none
 * return  : The variable's string value
 ******************************************************************************************/
char* TclGetVariable(const char* varName);


/******************************************************************************************
 * TclSetVariable
 * purpose : set variable in tcl
 * input   : varName - the name of the variable that is going to be set
 * input   : value - the value that is entered to the tcl variable
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ******************************************************************************************/
int TclSetVariable(const char* varName, const char* value);


/********************************************************************************************
 * AppAlloc
 * purpose : Allocation function used by the application. This is used instead of malloc()
 * input   : size   - Number of bytes to allocate
 * output  : none
 * return  : Pointer to the allocated space
 ********************************************************************************************/
void* AppAlloc(int size);


/********************************************************************************************
 * AppFree
 * purpose : Free allocation function used by the application. This is used instead of free()
 * input   : ptr  - Pointer to AppFree
 * output  : none
 * return  : none
 ********************************************************************************************/
void AppFree(void* ptr);


/********************************************************************************************
 * String2TransportAddress
 * purpose : make Transport Address from string
 * input   : StringIp - the source string
 * output  : address - the address in RV_Common_TransportAddress format
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int String2TransportAddress(char * StringIp,cmTransportAddress* address);


/********************************************************************************************
 * TransportAddress2String
 * purpose : make string from Transport Address
 * input   : StringIp - the source string
 * output  : address - the address in RV_Common_TransportAddress format
 * return  : none
 ********************************************************************************************/
void TransportAddress2String(cmTransportAddress* address,char * StringIp);



/********************************************************************************************
 * String2Buffer
 * purpose : Convert a readble string into a buffer struct
 * input   : string - String to convert
 *         : buffer - Resulting buffer struct
 *         : size   - Size of buffer
 * output  : none
 * return  : Number of bytes in buffer
 ********************************************************************************************/
int String2Buffer(char* string, void* buffer, int size);


/********************************************************************************************
 * Buffer2String
 * purpose : Convert a buffer of bytes into a readable string
 * input   : buffer - Buffer to convert
 *         : result - Resulting string holding the field
 *         : size   - Size of buffer
 * output  : none
 * return  : none
 ********************************************************************************************/
void Buffer2String(void* buffer, char* result, int size);


/********************************************************************************************
 * Bmp2String
 * purpose : Convert a BMP string into a string, removing all BMP - related chars
 * input   : bmp    - BMP String to convert
 *         : string - Resulting string
 *         : size   - Size of buffer in bytes
 * output  : none
 * return  : none
 ********************************************************************************************/
void Bmp2String(char* bmp, char* string, int size);


/********************************************************************************************
 * String2Bmp
 * purpose : Convert a string into a BMP string, adding all BMP - related chars
 * input   : string - String to convert
 *           bmp    - Resulting BMP string
 *           size   - Size of string in bytes
 * output  : none
 * return  : none
 ********************************************************************************************/
void String2Bmp(char* string, char* bmp, int size);


/************************************************************************************************************************
* LI_IpToString
* purpose : This function converts an IP address in network format from long integer to dot notation format.
*           The converted address is written to the specified buffer location as a null-terminated string.
* input   : ipAddr : The IP address in long integer network format.
* output    buf    : A pointer to the buffer location where the IP address in dot notation network format should be
*                    written.  The buffer location should have at least 18 bytes allocated.
* return  : This function returns the IP address in dot notation network format.  The address is written to the
*           specified buffer location as a null-terminated string.  If an error occurs, the function returns a NULL
*           value.
*************************************************************************************************************************/
char*  IpToString (IN  UINT32       ipAddr,
                   OUT char         *buf);


#endif


#ifdef __cplusplus
}
#endif
