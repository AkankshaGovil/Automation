#ifndef _entry_h_
#define _entry_h_

int
DiffNetoid(
        NetoidInfoEntry *netInfo,       /* Database entry found for the
                                         * registration id of the netoid
                                         * referred in the data_pkt
                                         */
        PhoNode         *phonodep,
        unsigned short  *rflags,        /* Tells the difference
                                         * netInfo - phonodep,
                                         * as a set of flags.
                                         */
        unsigned short  *xflags         /* places where netInfo doesnt
                                         * match phonodep
                                         */
);

CacheTableInfo *
FindProxyMasterEntry(PhoNode *phonodep);

int
ProcessProxyRegistration(
        InfoEntry       *masterInfo,
        InfoEntry       *infoEntry,
        PhoNode         *phonodep
);

int
ApplyAdmissionPolicy(
        PhoNode         *phonodep,
	NetoidInfoEntry	*info,
        int             isProxyReg,
        unsigned short  xflags,         /* Places where iedge differs from
                                         * its db entry
                                         */
        unsigned short  pflags,         /* Places where iedge differs from
                                         * its master entry
                                         */
        int             *reason
);

int
ProcessIedgeRegistration(Pkt * data_pkt);

#endif	/* _entry_h_ */
