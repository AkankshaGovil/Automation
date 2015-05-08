#ifndef _EXPORT_H_
#define _EXPORT_H_

#include "ipc.h"
#include "serverdb.h"
#include "key.h"

int ExportInfoEntry (FILE *exportf, InfoEntry *infoEntry, ClientAttribs *clAttribs);
int ExportVpnEntry (FILE *exportf, VpnEntry *vpnEntry);
int ExportVpnGEntry (FILE *exportf, VpnGroupEntry *vpnGroupEntry);
int ExportCPEntry (FILE *stream, CallPlanEntry *entry);
int ExportCREntry (FILE *stream, VpnRouteEntry *entry);
int ExportRealmEntry (FILE *stream, RealmEntry *entry);
int ExportIgrpEntry (FILE *stream, IgrpInfo *entry);
int ExportCPBEntry (FILE *stream, CallPlanBindEntry *entry);
int ExportTriggerEntry (FILE *stream, TriggerEntry *entry);
int ExportVnetEntry (FILE *stream, VnetEntry *entry);

#endif /* _EXPORT_H_ */
