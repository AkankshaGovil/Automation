#ifdef __cplusplus
extern "C" {
#endif

#ifndef __SNMPUTIL_H
#define __SNMPUTIL_H

const char * parameterName(h341ParameterName param);
void dPrintParameter(h341ParameterName param);
void dPrintOid(snmpObjectT *oid,char * buffer);
void dPrintIndex(UINT8 *index,int size,char *buffer);
int str2oid(char * str,int * id);
void dPrintMib(mibNodeT * node,char * oid,int offset,int num);



#endif
#ifdef __cplusplus
}
#endif
