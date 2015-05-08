#ifndef _IWF_UTILS_H_
#define _IWF_UTILS_H_

#define IWF_STATE_STR 80
char *iwfState2Str(int state,char string[IWF_STATE_STR]);
char * sanitizeSipNumber(char *number);
char *sipEvent2Str(int event,char *str);
int IWF_IsSipEvent (int event);
int h3232SipRespCode (int cause, int error);
int iwfConvertSipCodeToCause (int respCode);

#endif
