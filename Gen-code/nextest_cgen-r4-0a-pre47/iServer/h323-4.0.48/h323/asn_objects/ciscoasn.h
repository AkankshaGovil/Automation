/* This file was generated by RADVISION ASN.1 Compiler */
/* This is a compilation of : h225nonstdpdu2.asn */

#ifndef _h225nonstdpdu2_ASN_H
#define _h225nonstdpdu2_ASN_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned char* ciscoasnGetSyntax(void);

#ifdef __cplusplus
#define __ENUM enum
#define __NAME1 _cisco
#define __NAME2 
#else
#define __ENUM typedef enum
#define __NAME1 
#define __NAME2 _cisco
#endif


__ENUM __NAME1 {
  _cisco_____0123456789 = 0,
  _cisco_ACFnonStandardInfo = 15,
  _cisco_ARJnonStandardInfo = 35,
  _cisco_ARQnonStandardInfo = 55,
  _cisco_AliasAddr = 75,
  _cisco_AudioTranscodeType = 86,
  _cisco_CallEventCode = 106,
  _cisco_CallEventInfo = 121,
  _cisco_CallId = 136,
  _cisco_CallMgrParam = 144,
  _cisco_CallReleaseSource = 158,
  _cisco_CallSignallingParam = 177,
  _cisco_ChannelEvent = 198,
  _cisco_ChannelType = 212,
  _cisco_ClearTokenNonStandardInfo = 225,
  _cisco_CommonParam = 252,
  _cisco_DDRInfo = 265,
  _cisco_DRQRasnonStdUsageInfo = 274,
  _cisco_DRQnonStandardInfo = 297,
  _cisco_GRQnonStandardInfo = 317,
  _cisco_GatewayNonstdInfo = 337,
  _cisco_GatewayResourceInfo = 356,
  _cisco_GatewayType = 377,
  _cisco_H323_UU_NonStdInfo = 390,
  _cisco_IEI = 410,
  _cisco_IRRnonStandardInfo = 415,
  _cisco_IRRperCallnonStandardInfo = 435,
  _cisco_IpAddress = 462,
  _cisco_LCFnonStandardInfo = 473,
  _cisco_LRJnonStandardInfo = 493,
  _cisco_LRQnonStandardInfo = 513,
  _cisco_NonStdCallTerminationCause = 533,
  _cisco_NonStdReleaseCompleteReason = 561,
  _cisco_OldLRQnonStandardInfo = 590,
  _cisco_ProgIndIEInfo = 613,
  _cisco_ProgIndParam = 628,
  _cisco_PropID = 642,
  _cisco_PropItem = 650,
  _cisco_PropList = 660,
  _cisco_PropListPdu = 670,
  _cisco_PropValue = 683,
  _cisco_ProtoParam = 694,
  _cisco_QosInfo = 706,
  _cisco_QosType = 715,
  _cisco_QsigNonStdInfo = 724,
  _cisco_RSVPParam = 740,
  _cisco_RSVPParamInfo = 751,
  _cisco_RasnonStdUsageInformation = 766,
  _cisco_RedirectIEinfo = 793,
  _cisco_ReleaseConferenceIdentifier = 809,
  _cisco_ReleaseGloballyUniqueID = 838,
  _cisco_ReleaseH221NonStandard = 863,
  _cisco_ReleaseNonStandardIdentifier = 887,
  _cisco_ReleaseNonStandardParameter = 917,
  _cisco_ResultCode = 946,
  _cisco_RsvpClass = 958,
  _cisco_TieTrkIEInfo = 969,
  _cisco_TieTrkParam = 983,
  _cisco_TransportType = 996,
  _cisco_VideoTranscodeType = 1011,
  _cisco_absolute = 1031,
  _cisco_activeCalls = 1041,
  _cisco_adaptiveBusy = 1054,
  _cisco_arjterminationCause = 1068,
  _cisco_arqterminationCause = 1089,
  _cisco_audio_lport = 1110,
  _cisco_audio_rport = 1123,
  _cisco_audioTranscode = 1136,
  _cisco_badFormatAddress = 1152,
  _cisco_bandwidth = 1170,
  _cisco_bstr = 1181,
  _cisco_bytesReceived = 1187,
  _cisco_bytesSent = 1202,
  _cisco_callConnected = 1213,
  _cisco_callDisconnected = 1228,
  _cisco_callEvent = 1246,
  _cisco_callEvent_1 = 1257,
  _cisco_callMgrParam = 1270,
  _cisco_callReleaseSource = 1284,
  _cisco_callSignallingParam = 1303,
  _cisco_calledPartyInPstn = 1324,
  _cisco_calledPartyInVoip = 1343,
  _cisco_calledPartyNotRegistered = 1362,
  _cisco_callerNotRegistered = 1388,
  _cisco_callingOctet3a = 1409,
  _cisco_callingPartyInPstn = 1425,
  _cisco_callingPartyInVoip = 1445,
  _cisco_chanBandwidth = 1465,
  _cisco_chanEvent = 1480,
  _cisco_chanID = 1491,
  _cisco_chanType = 1499,
  _cisco_channelClosed = 1509,
  _cisco_channelCreated = 1524,
  _cisco_channelModified = 1540,
  _cisco_chapPassword = 1557,
  _cisco_classReference = 1571,
  _cisco_clearToken = 1587,
  _cisco_commonParam = 1599,
  _cisco_connectedNumber = 1612,
  _cisco_connectedNumber_141 = 1629,
  _cisco_consoleCommand = 1650,
  _cisco_constrainingField = 1666,
  _cisco_containerOfObjectSets = 1685,
  _cisco_containerOfObjects = 1708,
  _cisco_containerOfTypes = 1728,
  _cisco_containerOfValues = 1746,
  _cisco_controlledLoad = 1765,
  _cisco_data = 1781,
  _cisco_ddr = 1787,
  _cisco_ddrDialString = 1792,
  _cisco_ddrDialString_101 = 1807,
  _cisco_delta = 1826,
  _cisco_destinationRejection = 1833,
  _cisco_displayInformationElement = 1855,
  _cisco_drqRasnonStdUsageData = 1882,
  _cisco_dstProxyAlias = 1905,
  _cisco_dstProxyDDRInfo = 1920,
  _cisco_dstProxySignalAddress = 1937,
  _cisco_dstTerminalAlias = 1960,
  _cisco_e164 = 1978,
  _cisco_e164_47 = 1984,
  _cisco_email_id = 1993,
  _cisco_email_id_49 = 2003,
  _cisco_enterpriseID = 2016,
  _cisco_event = 2030,
  _cisco_externalCallControlAgent = 2037,
  _cisco_externalGKTMPServer = 2063,
  _cisco_externalNmsApp = 2084,
  _cisco_externalRadiusServer = 2100,
  _cisco_facilityCallDeflection = 2122,
  _cisco_failure = 2146,
  _cisco_fieldOfClassReference = 2155,
  _cisco_fieldOfEnumerated = 2178,
  _cisco_fieldOfObjectReference = 2197,
  _cisco_fieldReference = 2221,
  _cisco_flags = 2237,
  _cisco_flags_124_125 = 2244,
  _cisco_forwardOriginalGTD = 2259,
  _cisco_g711 = 2279,
  _cisco_g722 = 2285,
  _cisco_g7231 = 2291,
  _cisco_g728 = 2298,
  _cisco_g729 = 2304,
  _cisco_gatekeeper = 2310,
  _cisco_gatekeeperResources = 2322,
  _cisco_gatewayResources = 2343,
  _cisco_gatewaySrcInfo = 2361,
  _cisco_gateways = 2377,
  _cisco_gateways_114 = 2387,
  _cisco_genericDataReason = 2401,
  _cisco_gkID = 2420,
  _cisco_gkID_113 = 2426,
  _cisco_gtd = 2436,
  _cisco_gtdData = 2441,
  _cisco_gtdData_117 = 2450,
  _cisco_gtd_116 = 2463,
  _cisco_gtd_55 = 2472,
  _cisco_guaranteed = 2480,
  _cisco_guid = 2492,
  _cisco_guid_50 = 2498,
  _cisco_gupAddress = 2507,
  _cisco_gwAlias = 2519,
  _cisco_gwAlias_119 = 2528,
  _cisco_gwResource = 2541,
  _cisco_gwType = 2553,
  _cisco_h221NonStandard = 2561,
  _cisco_h261 = 2578,
  _cisco_h262 = 2584,
  _cisco_h263 = 2590,
  _cisco_h320_gateway = 2596,
  _cisco_h323_ID = 2610,
  _cisco_h323_ID_48 = 2619,
  _cisco_i1 = 2631,
  _cisco_i1_128 = 2635,
  _cisco_i2 = 2643,
  _cisco_i4 = 2647,
  _cisco_iecInfo = 2651,
  _cisco_iecInfo_155 = 2660,
  _cisco_iecInfo_155_156 = 2673,
  _cisco_iei = 2690,
  _cisco_imports = 2695,
  _cisco_importsArray = 2704,
  _cisco_inConf = 2718,
  _cisco_inUseBChannels = 2726,
  _cisco_inUseDSPs = 2742,
  _cisco_interclusterVersion = 2753,
  _cisco_interclusterVersion_135 = 2774,
  _cisco_interfaceDescription = 2799,
  _cisco_interfaceDescription_109 = 2821,
  _cisco_interfaceSpecificBillingId = 2847,
  _cisco_internalCallControlApp = 2875,
  _cisco_internalReleaseInPotsLeg = 2899,
  _cisco_internalReleaseInVoipAAA = 2925,
  _cisco_internalReleaseInVoipLeg = 2951,
  _cisco_inuseBandwidth = 2977,
  _cisco_invalidRevision = 2993,
  _cisco_ip = 3010,
  _cisco_ip_99 = 3014,
  _cisco_lpstr = 3021,
  _cisco_lrjterminationCause = 3028,
  _cisco_lrqterminationCause = 3049,
  _cisco_manufacturerCode = 3070,
  _cisco_maxBChannels = 3088,
  _cisco_maxDSPs = 3102,
  _cisco_media_ip_addr = 3111,
  _cisco_module = 3126,
  _cisco_multicast = 3134,
  _cisco_neededFeatureNotSupported = 3145,
  _cisco_newConnectionNeeded = 3172,
  _cisco_noBandwidth = 3193,
  _cisco_noPermission = 3206,
  _cisco_nonStandardIdentifier = 3220,
  _cisco_nonStandardReason = 3243,
  _cisco_nonstd_callIdentifier = 3262,
  _cisco_nonstd_none = 3285,
  _cisco_nonstd_unknown = 3298,
  _cisco_numberBChannels = 3314,
  _cisco_object = 3331,
  _cisco_objectArray = 3339,
  _cisco_objectReference = 3352,
  _cisco_objectSetArray = 3369,
  _cisco_objectSetElement = 3385,
  _cisco_objectSetReference = 3403,
  _cisco_object_96 = 3423,
  _cisco_port = 3434,
  _cisco_progIndIE = 3440,
  _cisco_progIndIE_140 = 3451,
  _cisco_progIndIEinfo = 3466,
  _cisco_progIndParam = 3481,
  _cisco_propid = 3495,
  _cisco_proplist = 3503,
  _cisco_protoParam = 3513,
  _cisco_proxy = 3525,
  _cisco_qoSBestEffort = 3532,
  _cisco_qoSControlledLoad = 3547,
  _cisco_qoSGuaranteedDelay = 3566,
  _cisco_qoSGuaranteedDelay_152 = 3586,
  _cisco_qos_video = 3610,
  _cisco_qosIE = 3621,
  _cisco_qsigNonStdInfo = 3628,
  _cisco_rasMessageSpecificData = 3644,
  _cisco_rasMessageSpecificData_154 = 3668,
  _cisco_rawMesg = 3696,
  _cisco_reRouteCount = 3705,
  _cisco_redirectIE = 3719,
  _cisco_redirectIEinfo = 3731,
  _cisco_releaseCompleteCauseIE = 3747,
  _cisco_releaseCompleteCauseIE_97 = 3771,
  _cisco_releaseCompleteReason = 3798,
  _cisco_remote_qos_audio_bw = 3821,
  _cisco_remote_qos_video = 3842,
  _cisco_remote_qos_video_bw = 3860,
  _cisco_replaceWithConferenceInvite = 3881,
  _cisco_resources = 3910,
  _cisco_result = 3921,
  _cisco_rsvpClass = 3929,
  _cisco_rsvpInfo = 3940,
  _cisco_rsvpParam = 3950,
  _cisco_securityDenied = 3961,
  _cisco_sigAddress = 3977,
  _cisco_sourceAlias = 3989,
  _cisco_sourceExtAlias = 4002,
  _cisco_srcInfo = 4018,
  _cisco_srcTerminalAlias = 4027,
  _cisco_startTime = 4045,
  _cisco_startTime_153 = 4056,
  _cisco_success = 4071,
  _cisco_t35CountryCode = 4080,
  _cisco_t35Extension = 4096,
  _cisco_termAlias = 4110,
  _cisco_tieTrkIE = 4121,
  _cisco_tieTrkIEinfo = 4131,
  _cisco_tieTrkParam = 4145,
  _cisco_timestamp = 4158,
  _cisco_transport = 4169,
  _cisco_ttl = 4180,
  _cisco_ttl_104 = 4185,
  _cisco_tunnelledSignallingRejected = 4194,
  _cisco_typeArray = 4223,
  _cisco_typeFromConstraint = 4234,
  _cisco_typeReference = 4254,
  _cisco_undefinedReason = 4269,
  _cisco_unicast = 4286,
  _cisco_unreachableDestination = 4295,
  _cisco_unreachableGatekeeper = 4319,
  _cisco_unreserved = 4342,
  _cisco_value = 4354,
  _cisco_valueNode = 4361,
  _cisco_valueTree = 4372,
  _cisco_values = 4383,
  _cisco_version = 4391,
  _cisco_video_lport = 4400,
  _cisco_video_lport_146 = 4413,
  _cisco_video_rport = 4430,
  _cisco_videoTranscode = 4443,
  _cisco_voip = 4459,
  _cisco_withSyntaxElement = 4465,
  _cisco_yesno = 4484,
  _cisco_yesno_133 = 4491} __NAME2;


#undef __NAME1
#undef __NAME2
#undef __ENUM

#define _cisco(a) _cisco_##a,
#define __cisco(a) _cisco_##a

#ifdef __cplusplus
}
#endif

#endif

