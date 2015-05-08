/*
 *	File:
 *
 *		ispd_rpc.x
 *
 *	Description:
 *
 *		This file protocol definition used by rpcgen
 *		to generate the RPC client-server interface
 *		for the ispd, ( iServer Peering	Daemon ).
 *
 */

typedef string String<128>;			/* defines String as
									 * an string of 128
									 * bytes
									 */

/*
 *  Structure for getting peer info.
 *    Used to pass information to peer iserver hosts via
 *    PUSH_ISERVER_INFO() and PULL_ISERVER_INFO().
 */

struct iserver_info
{
	int		server_type;		/* STANDBY or ACTIVE */
	int		iserver_status;		/* UP or DOWN */

	String	primary_interface;	/* interface name of primary interface */
	String	primary_ip;			/* real ip address of primary interface */
	String	primary_router;		/* router address for primary interface */
	String	primary_vip;		/* virtual ip address of primary interface */

	int		primary_interface_status; /* UNKNOWN, UP, DOWN */
	int		primary_link_status;      /* UNKNOWN, UP, DOWN */
	int		primary_vip_status;       /* UNKNOWN, UP, DOWN */
	int		primary_router_echo;      /* UNKNOWN, REACHABLE or UNREACHABLE */

	String	secondary_interface;/* interface name of secondary interface */
	String	secondary_ip;		/* real ip address of secondary interface */
	String	secondary_router;	/* router address for secondary interface */
	String	secondary_vip;		/* virtual ip address of secondary interface */

	int		secondary_interface_status; /* UNKNOWN, UP, DOWN */
	int		secondary_link_status;      /* UNKNOWN, UP, DOWN */
	int		secondary_vip_status;       /* UNKNOWN, UP, DOWN */
	int		secondary_router_echo;      /* UNKNOWN, REACHABLE or UNREACHABLE */

	String	ctl_ip;				  /* real ip address of control interface */
	int		ctl_interface_status; /* UNKNOWN, UP, DOWN */
	int		ctl_link_status;      /* UNKNOWN, UP, DOWN */
};

/*
 *  Structure for selecting vip owner.
 *    Used to pass information to peer iserver hosts via
 *    SELECT_VIP_OWNER() call.
 */

struct vip_info
{
	int		host_key;
	int		vip_id;                 /* type of vip :
                                     *    PRIMARY_VIP
                                     *    SECONDARY_VIP
                                     */
	String	vip;                    /* value of vip on calling host. */
};

/*
 *  Structure for status change.
 *    Used to pass information to local gis and
 *    local dbsync daemon.
 */

struct status_change
{
	int		status;					/*     0 means inactive
									 *     1 means active
									 */

	String	primary_vip;            /* value of primary vip.   */
	String	secondary_vip;          /* value of secondary vip. */
};

/*
 * the ispd program rpc function definitions
 *
 *   Currently 3 version numbers are used for different
 *   purposes.
 *
 *        ISPD_VERS   - used for rpc communication between
 *                      ispd daemons.
 *
 *        GIS_VERS    - used for rpc communication between
 *                      the ispd daemon and the gis daemon
 *                      on the localhost.
 *
 *        DBSYNC_VERS - used for rpc communication between
 *                      the ispd daemon and the dbsync daemon
 *                      on the localhost.
 */

program ISPD_PROG
{
    /*
     * The ISPD_VERS version number is used for
     * communication between the ispd daemons on
     * different hosts
     */
	version ISPD_VERS
	{
		/*
         * The PULL_ISERVER_INFO() call is made by STANDBY
         * and ACTIVE ispd clients to other peer ispds.
         * The call pulls iserver information from the 
         * called ispds.
		 */

        iserver_info      PULL_ISERVER_INFO( void )              = 1;

		/*
         * The PUSH_ISERVER_INFO() call is made by STANDBY
         * and ACTIVE ispd clients to other peer ispds.
         * The call is made when iserver information on the
         * local host changes and the changes need to be
		 * reflected to the host's peer ispds.
		 */

         void             PUSH_ISERVER_INFO( iserver_info )      = 2;

         /*
          *
          */

         int              SELECT_VIP_OWNER( vip_info )           = 3;

	} = 1;

    /*
     * The GIS_VERS version number is used for
     * communication between an ispd daemon and
     * the gis daemon on the same host. The gis
     * runs a GIS_VERS server thread which is
     * started at gis initialization time.
     */

	version GIS_VERS
	{
		/*
         * The GIS_STATUS_CHANGE() call is made whenever
         * the local ispd determines that the status of the
         * local host has changed from ACTIVE to INACTIVE
         * or vice-versa. It tells the gis daemon that
         * the status of the localhost has changed.
		 */

         void             GIS_STATUS_CHANGE( status_change )     = 1;
	} = 2;

    /*
     * The DBSYNC_VERS version number is used for
     * communication between an ispd daemon and
     * the database syncronization daemon on the
     * same host. The database syncronization
     * server runs a DBSYNC_VERS server thread
     * which is started at database sycronization
     * initialization time.
     */

	version DBSYNC_VERS
	{
		/*
         * The DBSYNC_STATUS_CHANGE() call is made whenever
         * the local ispd determines that the status of the
         * local host has changed from ACTIVE to INACTIVE
         * or vice-versa. It tells the database syncronization
         * daemon that the status of the localhost has changed.
		 */
 
         void             DBSYNC_STATUS_CHANGE( status_change )  = 1;
         int              DBSYNC_HEALTH_CHECK( void )        	 = 2;
	} = 3;

} = 0x202fbf01;
