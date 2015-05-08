

/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                general.c
 *
 * This file provides general functions for the Test Application.
 * These functions are used for initiating the test application.
 *
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#include "TAP_init.h"
#include "TAP_tools.h"
#include "TAP_utils.h"
#include "TAP_general.h"
#include <stdlib.h>


/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                       *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * Hex2Byte
 * purpose : Convert a hex character to a byte value
 * input   : ch - Character to convert
 * output  : none
 * return  : Byte value
 ********************************************************************************************/
BYTE Hex2Byte(char ch)
{
    switch (ch) {
        case '0': return 0x0;
        case '1': return 0x1;
        case '2': return 0x2;
        case '3': return 0x3;
        case '4': return 0x4;
        case '5': return 0x5;
        case '6': return 0x6;
        case '7': return 0x7;
        case '8': return 0x8;
        case '9': return 0x9;
        case 'a': return 0xa;
        case 'b': return 0xb;
        case 'c': return 0xc;
        case 'd': return 0xd;
        case 'e': return 0xe;
        case 'f': return 0xf;
        default:
            return 0xff;
    }
}


/********************************************************************************************
 * Buffer2String
 * purpose : Convert a buffer of bytes into a readable string
 * input   : buffer - Buffer to convert
 *         : result - Resulting string holding the field
 *         : size   - Size of buffer
 * output  : none
 * return  : none
 ********************************************************************************************/
void Buffer2String(void* buffer, char* result, int size)
{
    static const char* hexChars = "0123456789abcdef";
    BYTE* p = (BYTE *)buffer;
    int i;

    for (i = 0; i < size; i++)
    {
        result[i*2+1] = hexChars[p[i] & 0x0f];
        result[i*2] = hexChars[p[i] >> 4];
    }
    result[size*2] = '\0';
}


/********************************************************************************************
 * String2Buffer
 * purpose : Convert a readble string into a buffer struct
 * input   : string - String to convert
 *         : buffer - Resulting buffer struct
 *         : size   - Size of buffer
 * output  : none
 * return  : Number of bytes in buffer
 ********************************************************************************************/
int String2Buffer(char* string, void* buffer, int size)
{
    BYTE* p = (BYTE *)buffer;
    int i;
    int len;
    len = strlen(string) / 2;
    if (len > size) len = size;

    for (i = 0; i < len; i++)
    {
        p[i] =
            Hex2Byte(string[i*2+1]) |
            (Hex2Byte(string[i*2]) << 4);
    }
    return len;
}


/********************************************************************************************
 * Bmp2String
 * purpose : Convert a BMP string into a string, removing all BMP - related chars
 * input   : bmp    - BMP String to convert
 *         : string - Resulting string
 *         : size   - Size of buffer in bytes
 * output  : none
 * return  : none
 ********************************************************************************************/
void Bmp2String(char* bmp, char* string, int size)
{
    int count;

    for (count = 0; count < size/2; count++)
        string[count] = bmp[count*2+1];
    string[size/2] = '\0';
}


/********************************************************************************************
 * String2Bmp
 * purpose : Convert a string into a BMP string, adding all BMP - related chars
 * input   : string - String to convert
 *           bmp    - Resulting BMP string
 *           size   - Size of string in bytes
 * output  : none
 * return  : none
 ********************************************************************************************/
void String2Bmp(char* string, char* bmp, int size)
{
    int count;

    for (count = 0; count < size; count++)
    {
        bmp[count*2] = '\0';
        bmp[count*2+1] = string[count];
    }
}





/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * String2Alias
 * purpose : change strinf into a cmAlias
 * input   : str - the alias string from the listbox of NewCall
 *         : Alias - the target struct
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int String2Alias(char* str,cmAlias* Alias)
{
    char *pStr, *partyNumDigits, *hlp;
    cmTransportAddress transportAddress;

    /* Check what kind of alias we have */
    if (strncmp("TEL", str, 3) == 0)
        Alias->type = cmAliasTypeE164;
    else if (strncmp("NAME", str, 4) == 0)
        Alias->type = cmAliasTypeH323ID;
    else if (strncmp("URL", str, 3) == 0)
        Alias->type = cmAliasTypeURLID;
    else if (strncmp("TNAME", str, 5) == 0)
        Alias->type = cmAliasTypeTransportAddress;
    else if (strncmp("EMAIL", str, 5) == 0)
        Alias->type = cmAliasTypeEMailID;
    else if (strncmp("PN", str, 2) == 0)
        Alias->type = cmAliasTypePartyNumber;
    else
        return RVERROR;

    /* Find the position of the alias */
    pStr = strchr(str, ':');
    if (pStr == NULL) return RVERROR;

    /* Skip ":" */
    pStr++;

    switch (Alias->type)
    {
    case cmAliasTypeH323ID:
        {
            int i;
            Alias->length= strlen(pStr)*2;
            Alias->string = (char*)AppAlloc(Alias->length);
            for (i = 0; i < (int)strlen(pStr); i++)
            {
                Alias->string[i*2] = '\0';
                Alias->string[i*2+1] = pStr[i];
            }
            break;
        }
    case cmAliasTypeE164:
            {
            Alias->length = strlen(pStr);
            Alias->string = (char *)AppAlloc(Alias->length+1);
            strcpy(Alias->string , pStr);
            break;
        }
    case cmAliasTypeURLID:
            {
            Alias->length = strlen(pStr);
            Alias->string = (char *)AppAlloc(Alias->length+1);
            strcpy(Alias->string , pStr);
            break;
        }
    case cmAliasTypeEMailID:
        {
            Alias->length = strlen(pStr);
            Alias->string = (char *)AppAlloc(Alias->length+1);
            strcpy(Alias->string , pStr);
            break;
        }
    case cmAliasTypeTransportAddress:
        {
            Alias->string = NULL;
            String2TransportAddress(pStr, &transportAddress);
            memcpy(&(Alias->transport),&transportAddress,sizeof(cmTransportAddress));
            break;
        }
    case cmAliasTypePartyNumber:
        {
            partyNumDigits = strchr(pStr, '$');
            if (partyNumDigits == NULL)
            {
                /* didn't find $ in the string, not a propper Party Number: "PN:<num>$<type>" */
                return RVERROR;
            }
            hlp = partyNumDigits;
            ++partyNumDigits;
            *hlp = '\0';
            Alias->length = strlen(pStr);
            Alias->string = (char *)AppAlloc(Alias->length+1);
            strcpy(Alias->string, pStr);
            Alias->pnType = String2PartyNumber(partyNumDigits);
            *hlp = '$';
            break;
        }
    default:
        break;
    }

    return 0;
}



/********************************************************************************************
 * FreeAliasList
 * purpose : Free an array created by BuildAliasList
 * input   : aliasArray - Alias array to free
 * output  : none
 * return  : none
 ********************************************************************************************/
void FreeAliasList(cmAlias** aliasArray)
{
    cmAlias** alias;

    if (aliasArray == NULL) return;

    /* Free each of the alias structures */
    alias = aliasArray;
    while (*alias != NULL)
    {
        switch ((*alias)->type)
        {
        case cmAliasTypeH323ID:
        case cmAliasTypePartyNumber:
        case cmAliasTypeE164:
        case cmAliasTypeURLID:
        case cmAliasTypeEMailID:
            {
                AppFree((*alias)->string);
                break;
            }
        default:
            break;
        }
        AppFree (*alias);
        alias++;
    }

    /* Free the array of alias pointers */
    AppFree(aliasArray);
}


/********************************************************************************************
 * BuildAliasList
 * purpose : make array of cmAlias from array of char*
 * input   : tclList   - TCL list value
 * output  : none
 * return  : cmAlias - the target array
 ********************************************************************************************/
cmAlias ** BuildAliasList(cmAlias * * AliasArray, int AliasListSize, char * * AliasList)
{
    BOOL newAlias;
    int i, j;

    /* Create the aliases */
    j = 0;
    newAlias = TRUE;
    for (i = 0; i < AliasListSize; i++)
    {
        if (newAlias)
        {
            /* We're creating a place for a new alias */
            AliasArray[j] = (cmAlias *)AppAlloc(sizeof(cmAlias));
        }
        memset(AliasArray[j],0,sizeof(cmAlias));

        /* Convert a single string into an alias */
        if (String2Alias(AliasList[j], AliasArray[i]) >= 0)
        {
            newAlias = TRUE;
            j++;
        }
        else
            newAlias = FALSE;
    }

    if (newAlias == FALSE)
    {
        /* Remove the last allocation if we had any problems */
        AppFree(AliasArray[j]);
        AliasArray[j] = NULL;
    }

    return AliasArray;
}



/********************************************************************************************
 * String2TransportAddress
 * purpose : make RV323_PRTCL_TransportAddress from string
 * input   : StringIp - the source string
 * output  : address - the address in cmTransportAddress format
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int String2TransportAddress(char * StringIp,cmTransportAddress* address)
{
    int ip1, ip2, ip3, ip4, port;
    BYTE ipBuf[4];

    if (sscanf(StringIp, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port) != 5)
        return RVERROR;

    ipBuf[0] = ip1;
    ipBuf[1] = ip2;
    ipBuf[2] = ip3;
    ipBuf[3] = ip4;

    address->ip = *((UINT32 *)ipBuf);
    address->distribution = cmDistributionUnicast;
    address->type = cmTransportTypeIP;
    address->port = port;

    return 0;
}

/********************************************************************************************
 * TransportAddress2String
 * purpose : make string from Transport Address
 * input   : StringIp - the source string
 * output  : address - the address in cmTransportAddress format
 * return  : none
 ********************************************************************************************/
void TransportAddress2String(cmTransportAddress* address,char * StringIp)
{
    sprintf(StringIp,
            "%d.%d.%d.%d:%d",
            ((BYTE*)&address->ip)[0],
            ((BYTE*)&address->ip)[1],
            ((BYTE*)&address->ip)[2],
            ((BYTE*)&address->ip)[3],
            address->port);
}


/********************************************************************************************
 * Alias2String
 * purpose : make array of strings from array of cmAlias **
 * input   : Alias - the source alias
 *         : String - the result string (should be allocated)
 * output  : String - the result string
 * return  : char* - the result string
 ********************************************************************************************/
char * Alias2String( cmAlias * Alias, char * String)
{
    if (Alias != NULL)
    {
        switch (Alias->type)
        {
        case cmAliasTypeE164:
            {
                Alias->string[Alias->length] = '\0';
                sprintf(String,"TEL:%s",Alias->string);
                break;
            }
        case cmAliasTypeH323ID:
            {
                int i;
                Alias->string[Alias->length] = '\0';
                memset(String, 0, Alias->length+20);
                strcpy(String, "NAME:");
                for (i = 0; i < Alias->length/2; i++)
                    String[i+5] = Alias->string[i*2+1];
                break;
            }
        case cmAliasTypeURLID:
            {
                Alias->string[Alias->length] = '\0';
                sprintf(String,"URL:%s",Alias->string);
                break;
            }
        case cmAliasTypeEMailID:
            {
                Alias->string[Alias->length] = '\0';
                sprintf(String,"EMAIL:%s",Alias->string);
                break;
            }
        case cmAliasTypeTransportAddress:
            {
                sprintf(String,"TNAME:");
                TransportAddress2String(&(Alias->transport), &String[6]);
                break;
            }
        case cmAliasTypePartyNumber:
            {
                Alias->string[Alias->length] = '\0';
                sprintf(String, "PN:%s$%s", Alias->string, PartyNumber2String(Alias->pnType));
                break;
            }
        default:
            String[0] = '\0';
            break;
        }
    }
    return String;
}



/******************************************************************************************
 * TclExecute
 * purpose : execute a command in tcl
 * input   : cmd - the command that is going to be executed
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ******************************************************************************************/
int TclExecute(const char* cmd, ...)
{
    va_list v;
    char ptr[2048];

    va_start(v, cmd);
    vsprintf(ptr, cmd, v);
    va_end(v);

    if (Tcl_GlobalEval(interp, ptr) != TCL_OK)
    {
        if ((strlen(ptr) + strlen(Tcl_GetStringResult(interp))) < sizeof(ptr))
            sprintf(ptr + strlen(ptr), ": %s", Tcl_GetStringResult(interp));
        PutError("ERROR: GlobalEval", ptr);
    }

    return TCL_OK;
}


/******************************************************************************************
 * TclString
 * purpose : make a given string a printable TCL string
 * input   : line    - Line to convert
 * output  : none
 * return  : Converted string
 ******************************************************************************************/
char* TclString(const char* line)
{
    static char buf[2048];
    char* destPtr;
    char* srcPtr;
    srcPtr = (char *)line;
    destPtr = buf;

    while (*srcPtr)
    {
        switch (*srcPtr)
        {
            case '[':
            case ']':
            case '{':
            case '}':
            case '\\':
                *destPtr++ = '\\';
            default:
                break;
        }

        *destPtr = *srcPtr;
        destPtr++;
        srcPtr++;
    }

    *destPtr = '\0';
    return buf;
}


/******************************************************************************************
 * TclGetVariable
 * purpose : get variable from tcl
 * input   : varName - the name of the variable that is going to be imported
 * input   : none
 * output  : none
 * return  : The variable's string value
 ******************************************************************************************/
char* TclGetVariable(const char* varName)
{
    char arrayName[25];
    char indexName[128];
    char* token;
    char* result;

    /* See if it's an array variable */
    token = strchr(varName, '(');

    if (token == NULL)
        result = Tcl_GetVar(interp, (char *)varName, TCL_GLOBAL_ONLY);
    else
    {
        /* Array - let's first split it into array name and index name */
        int arrayLen = token - varName;
        int indexLen = strlen(token+1)-1;

        if ((arrayLen >= (int)sizeof(arrayName)) || (indexLen >= (int)sizeof(indexName)))
        {
            PutError(varName, "Length of array name or index out of bounds in GetVar");
            return (char *)"-unknown-";
        }

        memcpy(arrayName, varName, arrayLen);
        arrayName[arrayLen] = '\0';
        memcpy(indexName, token+1, indexLen);
        indexName[indexLen] = '\0';
        result = Tcl_GetVar2(interp, arrayName, indexName, TCL_GLOBAL_ONLY);
    }

    if (result == NULL)
    {
        PutError(varName, Tcl_GetStringResult(interp));
        return (char *)"-unknown-";
    }

    return result;
}


/******************************************************************************************
 * TclSetVariable
 * purpose : set variable in tcl
 * input   : varName - the name of the variable that is going to be set
 * input   : value - the value that is entered to the tcl variable
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ******************************************************************************************/
int TclSetVariable(const char* varName, const char* value)
{
    char arrayName[25];
    char indexName[128];
    char* token;
    char* result;

    /* See if it's an array variable */
    token = strchr(varName, '(');

    if (token == NULL)
        result = Tcl_SetVar(interp, (char *)varName, (char *)value, TCL_GLOBAL_ONLY);
    else
    {
        /* Array - let's first split it into array name and index name */
        int arrayLen = token - varName;
        int indexLen = strlen(token+1)-1;

        if ((arrayLen >= (int)sizeof(arrayName)) || (indexLen >= (int)sizeof(indexName)))
        {
            PutError(varName, "Length of array name or index out of bounds in SetVar");
            return TCL_OK;
        }

        memcpy(arrayName, varName, arrayLen);
        arrayName[arrayLen] = '\0';
        memcpy(indexName, token+1, indexLen);
        indexName[indexLen] = '\0';
        result = Tcl_SetVar2(interp, (char *)arrayName, (char *)indexName, (char *)value, TCL_GLOBAL_ONLY);
    }

    if (result == NULL)
        PutError(varName, Tcl_GetStringResult(interp));

    return TCL_OK;
}


/********************************************************************************************
 * AppAlloc
 * purpose : Allocation function used by the application. This is used instead of malloc()
 * input   : size   - Number of bytes to allocate
 * output  : none
 * return  : Pointer to the allocated space
 ********************************************************************************************/
void* AppAlloc(int size)
{
    void* p;

    p = malloc(size + sizeof(void*));

    if (p != NULL)
    {
        int* pBlock = (int *)p;
        *pBlock = size;

        NumAllocations.numOfUsed++;
        SizeAllocations.numOfUsed += size;

        if (NumAllocations.numOfUsed > NumAllocations.maxUsage)
            NumAllocations.maxUsage = NumAllocations.numOfUsed;
        if (SizeAllocations.numOfUsed > SizeAllocations.maxUsage)
            SizeAllocations.maxUsage = SizeAllocations.numOfUsed;
    }

    return ((void *)((char *)p + sizeof(void*)));
}


/********************************************************************************************
 * AppFree
 * purpose : Free allocation function used by the application. This is used instead of free()
 * input   : ptr  - Pointer to AppFree
 * output  : none
 * return  : none
 ********************************************************************************************/
void AppFree(void* ptr)
{
    int* myAlloc;
    int size;

    myAlloc = (int *)((char *)ptr - sizeof(void*));
    size = *myAlloc;

    free(myAlloc);

    NumAllocations.numOfUsed--;
    SizeAllocations.numOfUsed -= size;

    if (NumAllocations.numOfUsed > NumAllocations.maxUsage)
        NumAllocations.maxUsage = NumAllocations.numOfUsed;
    if (SizeAllocations.numOfUsed > SizeAllocations.maxUsage)
        SizeAllocations.maxUsage = SizeAllocations.numOfUsed;
}


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
char*  IpToString (IN  UINT32      ipAddr,
                   OUT char        *buf)
{
    BYTE* ip = (unsigned char*)&ipAddr;

    sprintf(buf,"%d.%d.%d.%d", (int)ip[0],(int)ip[1],(int)ip[2],(int)ip[3]);
    return buf;

}




#ifdef __cplusplus
}
#endif

