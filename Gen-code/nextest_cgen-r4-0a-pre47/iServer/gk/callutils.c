#include <stdio.h>
#include <sys/time.h> 
#include <stdlib.h>
#include "uh323proto.h"
#include "calldefs.h"

static int seed;

void initMsdSeed(void)
{
		struct timeval tp;
		gettimeofday(&tp,NULL) ;
		seed = tp.tv_sec; 
}

int genMsdNumber(void)
{
    static int seed = 0;
	int	msd;
	long long l;
	l = rand_r(&seed);
	l = ((l<< 32)/RAND_MAX);
	msd = l%16777215;
	return msd;
}



char isdnCauseStr[MAX_ISDN_CAUSE][40] = {
"(0)",
"UnAssigned Number(1)",
"No Route Transit(2)",
"No Route to Destination (3)",
"(4)",
"(5)",
"(6)",
"(7)",
"(8)",
"(9)",
"(10)",
"(11)",
"(12)",
"(13)",
"(14)",
"(15)",
"Normal Call Clearing(16)",
"User Busy(17)",
"User Not Responding(18)",
"User Not Answering(19)",
"Subscriber Absent(20)",
"Call Rejected(21)",
"Number Changed(22)",
"(23)",
"(24)",
"(25)",
"Non Selected User(26)",
"Destination Out Of Order(27)",
"Invalid Number Format(28)",
"(29)",
"(30)",
"Normal Unspecified(31)",
"(32)",
"(33)",
"No Circuit Available(34)",
"(35)",
"(36)",
"(37)",
"Network Out Of Order(38)",
"(39)",
"(40)",
"Temporary Failure(41)",
"Switching Equipment Congestion(42)",
"(43)",
"(44)",
"(45)",
"(46)",
"No Resource(47)",
"(48)",
"(49)",
"(50)",
"(51)",
"(52)",
"(53)",
"(54)",
"Incoming Class Barred(55)",
"(56)",
"Bearer Capability Not Authorized (57)",
"BearerCap Presently Not Available(58)",
"(59)",
"(60)",
"(61)",
"(62)",
"Service or Option Not Available(63)",
"(64)",
"(65)",
"(66)",
"(67)",
"(68)",
"(69)",
"(70)",
"(71)",
"(72)",
"(73)",
"(74)",
"(75)",
"(76)",
"(77)",
"(78)",
"Service or Option Not Implemented(79)",
"(80)",
"(81)",
"(82)",
"(83)",
"(84)",
"(85)",
"(86)",
"(87)",
"(88)",
"(89)",
"(90)",
"Invalid Transit(91)",
"(92)",
"(93)",
"(94)",
"(95)",
"(96)",
"(97)",
"(98)",
"(99)",
"(100)",
"(101)",
"Recovery on Timer Expiry(102)",
"(103)",
"(104)",
"(105)",
"(106)",
"(107)",
"(108)",
"(109)",
"(110)",
"(111)",
"(112)",
"(113)",
"(114)",
"(115)",
"(116)",
"(117)",
"(118)",
"(119)",
"(120)",
"(121)",
"(122)",
"(123)",
"(124)",
"(125)",
"(126)",
"Interworking(127)"
};

char * getIsdnCCString(int cause)
{
    if(cause >=0 && cause <Cause_eMax)
        return isdnCauseStr[cause];

    return "Invalid Cause Code";
}

// To be called only if  config does not say "pass"
int
getDestCalledPartyNumType(char q931ie)
{
	int pnType;

	switch(q931ie)
	{
	case Q931CDPN_Default:
	case Q931CDPN_Unknown:
		pnType = cmPartyNumberPublicUnknown;
		break;
	case Q931CDPN_International:   
		pnType = cmPartyNumberPublicInternationalNumber;
		break;
	case Q931CDPN_National:
		pnType = cmPartyNumberPublicNationalNumber;
		break;
	case Q931CDPN_Specific:
		pnType = cmPartyNumberPublicNetworkSpecificNumber;
		break;
	case Q931CDPN_Subscriber:
		pnType = cmPartyNumberPublicSubscriberNumber;
		break;
	case Q931CDPN_Abbreviated:
		pnType = cmPartyNumberPublicAbbreviatedNumber;
		break;
	default:
		pnType = cmPartyNumberPublicUnknown;
		break;
	}

	return pnType;
}

int
getDestCallingPartyNumType(char cgpn)
{
	int pnType;

	switch(cgpn)
	{
	case Q931CGPN_Unknown:
		pnType = cmPartyNumberPublicUnknown;
		break;
	case Q931CGPN_International:   
		pnType = cmPartyNumberPublicInternationalNumber;
		break;
	case Q931CGPN_National:
		pnType = cmPartyNumberPublicNationalNumber;
		break;
	case Q931CGPN_Specific:
		pnType = cmPartyNumberPublicNetworkSpecificNumber;
		break;
	case Q931CGPN_Subscriber:
		pnType = cmPartyNumberPublicSubscriberNumber;
		break;
	case Q931CGPN_Abbreviated:
		pnType = cmPartyNumberPublicAbbreviatedNumber;
		break;
	default:
		pnType = cmPartyNumberPublicUnknown;
		break;
	}

	return pnType;
}

